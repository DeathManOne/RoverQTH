#include <cmath>
#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "database/ota/parks.h"
#include "database/ota/summits.h"
#include "services/ota.h"
#include "services/storage.h"
#include "services/update.h"
#include "utilities/distance.h"
#include "utilities/ota.h"
#include "utilities/text.h"

namespace ota       = services::ota;
namespace otaDB     = database::ota;
namespace parksDB   = database::ota::parks;
namespace summitsDB = database::ota::summits;
namespace storage   = services::storage;
namespace update    = services::update;
namespace distance  = utilities::distance;
namespace uOta      = utilities::ota;
namespace text      = utilities::text;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr size_t TYPE_COUNT           = 2U;
    constexpr double REFRESH_DISTANCE_KM  = 1.0;
    constexpr uint32_t TASK_STACK_SIZE    = 8192U;

    enum class TaskKind : uint8_t {
        NONE,
        NEAREST,
        BY_CODE,
        BY_PREFIX,
        NEARBY
    };

    struct RuntimeState {
        ota::Status status             = ota::Status::UNAVAILABLE;
        ota::NearbyStatus nearbyStatus = ota::NearbyStatus::UNAVAILABLE;
        char requestedCode[otaDB::CODE_SIZE] {};
        ota::Reference reference {};
        otaDB::SearchResults nearbyResults {};
        double distanceKm        = 0.0;
        double bearingDeg        = 0.0;
        double searchedLatitude  = 0.0;
        double searchedLongitude = 0.0;
        bool hasSearchedPosition = false;
    };

    struct TaskRequest {
        ota::Type type = ota::Type::SUMMITS;
        TaskKind kind   = TaskKind::NONE;
        char value[otaDB::CODE_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        double radiusKm  = 0.0;
    };

    RuntimeState _states[TYPE_COUNT] {};
    TaskRequest _request {};
    bool _taskRunning    = false;
    bool _cancelRequested = false;

    bool _validType(ota::Type type);
    size_t _typeIndex(ota::Type type);
    RuntimeState& _state(ota::Type type);
    bool _validPosition(double latitude, double longitude);
    ota::Reference _toReference(const summitsDB::Summit &summit);
    ota::Reference _toReference(const parksDB::Park &park);
    bool _cancellationRequested(void* userData);
    void _searchTask(void* parameter);

    bool _validType(const ota::Type type) {
        return
            type == ota::Type::SUMMITS ||
            type == ota::Type::PARKS;
    }

    size_t _typeIndex(const ota::Type type) {
        return type == ota::Type::PARKS ? 1U : 0U;
    }

    RuntimeState& _state(const ota::Type type) {
        return _states[_typeIndex(type)];
    }

    bool _validPosition(
        const double latitude,
        const double longitude
    ) {
        return
            std::isfinite(latitude) &&
            std::isfinite(longitude) &&
            latitude >= -90.0 &&
            latitude <= 90.0 &&
            longitude >= -180.0 &&
            longitude <= 180.0;
    }

    ota::Reference _toReference(
        const summitsDB::Summit &summit
    ) {
        ota::Reference reference {};

        text::copy(
            reference.code,
            sizeof(reference.code),
            summit.code
        );

        text::copy(
            reference.area,
            sizeof(reference.area),
            summit.area
        );

        reference.latitude  = summit.latitude;
        reference.longitude = summit.longitude;
        reference.altitude  = summit.altitude;
        reference.points    = summit.points;
        reference.bonus     = summit.bonus;

        return reference;
    }

    ota::Reference _toReference(
        const parksDB::Park &park
    ) {
        ota::Reference reference {};

        text::copy(
            reference.code,
            sizeof(reference.code),
            park.code
        );

        text::copy(
            reference.area,
            sizeof(reference.area),
            park.area
        );

        reference.latitude  = park.latitude;
        reference.longitude = park.longitude;

        return reference;
    }

    bool _cancellationRequested(void*) {
        portENTER_CRITICAL(&_lock);

        const bool requested =
            _taskRunning &&
            (
                _request.kind == TaskKind::BY_PREFIX ||
                _request.kind == TaskKind::NEARBY
            ) &&
            _cancelRequested;

        portEXIT_CRITICAL(&_lock);
        return requested;
    }

    void _searchTask(void*) {
        TaskRequest request {};

        portENTER_CRITICAL(&_lock);
        request = _request;
        portEXIT_CRITICAL(&_lock);

        ota::Reference reference {};
        otaDB::SearchResults results {};
        otaDB::SearchStatus searchStatus = otaDB::SearchStatus::ERROR;
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
        bool found        = false;

        switch (request.kind) {
            case TaskKind::NEAREST:
                if (request.type == ota::Type::PARKS) {
                    parksDB::Park park {};

                    found = parksDB::findNearest(
                        request.latitude,
                        request.longitude,
                        park,
                        distanceKm,
                        bearingDeg
                    );

                    if (found) {
                        reference = _toReference(park);
                    }
                } else {
                    summitsDB::Summit summit {};

                    found = summitsDB::findNearest(
                        request.latitude,
                        request.longitude,
                        summit,
                        distanceKm,
                        bearingDeg
                    );

                    if (found) {
                        reference = _toReference(summit);
                    }
                }
                break;

            case TaskKind::BY_CODE:
                if (request.type == ota::Type::PARKS) {
                    parksDB::Park park {};
                    found = parksDB::findByCode(request.value, park);

                    if (found) {
                        reference = _toReference(park);
                    }
                } else {
                    summitsDB::Summit summit {};
                    found = summitsDB::findByCode(request.value, summit);

                    if (found) {
                        reference = _toReference(summit);
                    }
                }

                if (found) {
                    distanceKm = distance::betweenKilometers(
                        request.latitude,
                        request.longitude,
                        reference.latitude,
                        reference.longitude
                    );

                    bearingDeg = distance::bearingDegrees(
                        request.latitude,
                        request.longitude,
                        reference.latitude,
                        reference.longitude
                    );

                    found =
                        std::isfinite(distanceKm) &&
                        std::isfinite(bearingDeg);
                }
                break;

            case TaskKind::BY_PREFIX:
                searchStatus = request.type == ota::Type::PARKS
                    ? parksDB::findByPrefix(
                        request.latitude,
                        request.longitude,
                        request.value,
                        results,
                        _cancellationRequested,
                        nullptr
                    )
                    : summitsDB::findByPrefix(
                        request.latitude,
                        request.longitude,
                        request.value,
                        results,
                        _cancellationRequested,
                        nullptr
                    );
                break;

            case TaskKind::NEARBY:
                searchStatus = request.type == ota::Type::PARKS
                    ? parksDB::findNearby(
                        request.latitude,
                        request.longitude,
                        request.radiusKm,
                        results,
                        _cancellationRequested,
                        nullptr
                    )
                    : summitsDB::findNearby(
                        request.latitude,
                        request.longitude,
                        request.radiusKm,
                        results,
                        _cancellationRequested,
                        nullptr
                    );
                break;

            case TaskKind::NONE:
            default:
                break;
        }

        const bool listSearch =
            request.kind == TaskKind::BY_PREFIX ||
            request.kind == TaskKind::NEARBY;

        if (listSearch) {
            if (searchStatus == otaDB::SearchStatus::ERROR) {
                storage::appendErrorRecord(
                    request.type == ota::Type::PARKS
                        ? request.kind == TaskKind::BY_PREFIX
                            ? "POTA_PREFIX_SEARCH_FAILED"
                            : "POTA_NEARBY_SEARCH_FAILED"
                        : request.kind == TaskKind::BY_PREFIX
                            ? "SOTA_PREFIX_SEARCH_FAILED"
                            : "SOTA_NEARBY_SEARCH_FAILED"
                );
            } else if (searchStatus == otaDB::SearchStatus::CANCELLED) {
                storage::appendLogRecord(
                    request.type == ota::Type::PARKS
                        ? request.kind == TaskKind::BY_PREFIX
                            ? "POTA_PREFIX_SEARCH_CANCELLED"
                            : "POTA_NEARBY_SEARCH_CANCELLED"
                        : request.kind == TaskKind::BY_PREFIX
                            ? "SOTA_PREFIX_SEARCH_CANCELLED"
                            : "SOTA_NEARBY_SEARCH_CANCELLED"
                );
            }
        } else if (!found) {
            storage::appendErrorRecord(
                request.type == ota::Type::PARKS
                    ? request.kind == TaskKind::BY_CODE
                        ? "POTA_CODE_SEARCH_FAILED"
                        : "POTA_SEARCH_FAILED"
                    : request.kind == TaskKind::BY_CODE
                        ? "SOTA_CODE_SEARCH_FAILED"
                        : "SOTA_SEARCH_FAILED"
            );
        }

        portENTER_CRITICAL(&_lock);

        RuntimeState &state = _state(request.type);

        if (listSearch) {
            switch (searchStatus) {
                case otaDB::SearchStatus::SUCCESS:
                    state.nearbyResults = results;
                    state.nearbyStatus  = results.count > 0U
                        ? ota::NearbyStatus::READY
                        : ota::NearbyStatus::EMPTY;
                    break;

                case otaDB::SearchStatus::CANCELLED:
                    otaDB::clear(state.nearbyResults);
                    state.nearbyStatus = ota::NearbyStatus::IDLE;
                    break;

                case otaDB::SearchStatus::ERROR:
                    otaDB::clear(state.nearbyResults);
                    state.nearbyStatus = ota::NearbyStatus::ERROR;
                    break;
            }
        } else if (found) {
            state.reference  = reference;
            state.distanceKm = distanceKm;
            state.bearingDeg = bearingDeg;
            state.status     = ota::Status::READY;

            if (request.kind == TaskKind::NEAREST) {
                state.searchedLatitude  = request.latitude;
                state.searchedLongitude = request.longitude;
                state.hasSearchedPosition = true;
            } else {
                state.hasSearchedPosition = false;
            }
        } else {
            state.reference  = ota::Reference {};
            state.distanceKm = 0.0;
            state.bearingDeg = 0.0;
            state.hasSearchedPosition = false;
            state.status = ota::Status::ERROR;
        }

        _request        = TaskRequest {};
        _taskRunning    = false;
        _cancelRequested = false;

        portEXIT_CRITICAL(&_lock);
        vTaskDelete(nullptr);
    }
}

void ota::begin() {
    summitsDB::Info summitsInfo {};
    parksDB::Info parksInfo {};

    const bool summitsAvailable =
        summitsDB::info(summitsInfo);

    const bool parksAvailable =
        parksDB::info(parksInfo);

    portENTER_CRITICAL(&_lock);

    _states[0] = RuntimeState {};
    _states[1] = RuntimeState {};

    _states[0].status = summitsAvailable
        ? Status::IDLE
        : Status::UNAVAILABLE;

    _states[0].nearbyStatus = summitsAvailable
        ? NearbyStatus::IDLE
        : NearbyStatus::UNAVAILABLE;

    _states[1].status = parksAvailable
        ? Status::IDLE
        : Status::UNAVAILABLE;

    _states[1].nearbyStatus = parksAvailable
        ? NearbyStatus::IDLE
        : NearbyStatus::UNAVAILABLE;

    _request         = TaskRequest {};
    _taskRunning     = false;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendLogRecord(
        summitsAvailable
            ? "SOTA_SERVICE_READY status=available"
            : "SOTA_SERVICE_READY status=unavailable"
    );

    storage::appendLogRecord(
        parksAvailable
            ? "POTA_SERVICE_READY status=available"
            : "POTA_SERVICE_READY status=unavailable"
    );
}

void ota::invalidate(const Type type) {
    if (!_validType(type)) {
        return;
    }

    bool available = false;

    if (type == Type::PARKS) {
        parksDB::Info info {};
        available = parksDB::info(info);
    } else {
        summitsDB::Info info {};
        available = summitsDB::info(info);
    }

    portENTER_CRITICAL(&_lock);

    RuntimeState &state = _state(type);
    state = RuntimeState {};
    state.status = available
        ? Status::IDLE
        : Status::UNAVAILABLE;

    state.nearbyStatus = available
        ? NearbyStatus::IDLE
        : NearbyStatus::UNAVAILABLE;

    portEXIT_CRITICAL(&_lock);
}

bool ota::requestNearest(
    const Type type,
    const double latitude,
    const double longitude
) {
    if (
        !_validType(type) ||
        !_validPosition(latitude, longitude) ||
        update::isBusy()
    ) {
        return false;
    }

    Status status;
    double searchedLatitude;
    double searchedLongitude;
    bool hasSearchedPosition;

    portENTER_CRITICAL(&_lock);

    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    const RuntimeState &current = _state(type);
    status              = current.status;
    searchedLatitude    = current.searchedLatitude;
    searchedLongitude   = current.searchedLongitude;
    hasSearchedPosition = current.hasSearchedPosition;

    portEXIT_CRITICAL(&_lock);

    if (status == Status::UNAVAILABLE) {
        return false;
    }

    if (
        status == Status::READY &&
        hasSearchedPosition
    ) {
        const double movedKm = distance::betweenKilometers(
            searchedLatitude,
            searchedLongitude,
            latitude,
            longitude
        );

        if (
            std::isfinite(movedKm) &&
            movedKm < REFRESH_DISTANCE_KM
        ) {
            return true;
        }
    }

    portENTER_CRITICAL(&_lock);

    RuntimeState &state = _state(type);

    if (
        _taskRunning ||
        state.status == Status::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    state.requestedCode[0] = '\0';
    state.reference  = Reference {};
    state.distanceKm = 0.0;
    state.bearingDeg = 0.0;
    state.hasSearchedPosition = false;
    state.status = Status::SEARCHING;

    _request = TaskRequest {};
    _request.type      = type;
    _request.kind      = TaskKind::NEAREST;
    _request.latitude  = latitude;
    _request.longitude = longitude;

    _taskRunning     = true;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        type == Type::PARKS
            ? "POTA nearest"
            : "SOTA nearest",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) {
        return true;
    }

    portENTER_CRITICAL(&_lock);

    state.reference  = Reference {};
    state.distanceKm = 0.0;
    state.bearingDeg = 0.0;
    state.hasSearchedPosition = false;
    state.status = Status::ERROR;

    _request         = TaskRequest {};
    _taskRunning     = false;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord(
        type == Type::PARKS
            ? "POTA_TASK_CREATE_FAILED"
            : "SOTA_TASK_CREATE_FAILED"
    );

    return false;
}

bool ota::requestByCode(
    const Type type,
    const char* const code,
    const double latitude,
    const double longitude
) {
    if (
        !_validType(type) ||
        code == nullptr ||
        code[0] == '\0' ||
        std::memchr(code, '\0', otaDB::CODE_SIZE) == nullptr ||
        !_validPosition(latitude, longitude) ||
        update::isBusy()
    ) {
        return false;
    }

    Status status;
    Reference cachedReference {};
    char requestedCode[otaDB::CODE_SIZE] {};

    portENTER_CRITICAL(&_lock);

    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    const RuntimeState &current = _state(type);
    status          = current.status;
    cachedReference = current.reference;

    text::copy(
        requestedCode,
        sizeof(requestedCode),
        current.requestedCode
    );

    portEXIT_CRITICAL(&_lock);

    if (status == Status::UNAVAILABLE) {
        return false;
    }

    if (text::equals(requestedCode, code)) {
        if (status == Status::ERROR) {
            return false;
        }

        if (
            status == Status::READY &&
            cachedReference.code[0] != '\0'
        ) {
            const double distanceKm = distance::betweenKilometers(
                latitude,
                longitude,
                cachedReference.latitude,
                cachedReference.longitude
            );

            const double bearingDeg = distance::bearingDegrees(
                latitude,
                longitude,
                cachedReference.latitude,
                cachedReference.longitude
            );

            if (
                !std::isfinite(distanceKm) ||
                !std::isfinite(bearingDeg)
            ) {
                return false;
            }

            portENTER_CRITICAL(&_lock);

            RuntimeState &state = _state(type);

            if (
                _taskRunning ||
                state.status != Status::READY ||
                !text::equals(state.requestedCode, code)
            ) {
                portEXIT_CRITICAL(&_lock);
                return false;
            }

            state.distanceKm = distanceKm;
            state.bearingDeg = bearingDeg;

            portEXIT_CRITICAL(&_lock);
            return true;
        }
    }

    portENTER_CRITICAL(&_lock);

    RuntimeState &state = _state(type);

    if (
        _taskRunning ||
        state.status == Status::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    text::copy(
        state.requestedCode,
        sizeof(state.requestedCode),
        code
    );

    state.reference  = Reference {};
    state.distanceKm = 0.0;
    state.bearingDeg = 0.0;
    state.hasSearchedPosition = false;
    state.status = Status::SEARCHING;

    _request = TaskRequest {};
    _request.type      = type;
    _request.kind      = TaskKind::BY_CODE;
    _request.latitude  = latitude;
    _request.longitude = longitude;

    text::copy(
        _request.value,
        sizeof(_request.value),
        code
    );

    _taskRunning     = true;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        type == Type::PARKS
            ? "POTA code"
            : "SOTA code",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) {
        return true;
    }

    portENTER_CRITICAL(&_lock);

    state.reference  = Reference {};
    state.distanceKm = 0.0;
    state.bearingDeg = 0.0;
    state.hasSearchedPosition = false;
    state.status = Status::ERROR;

    _request         = TaskRequest {};
    _taskRunning     = false;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord(
        type == Type::PARKS
            ? "POTA_CODE_TASK_CREATE_FAILED"
            : "SOTA_CODE_TASK_CREATE_FAILED"
    );

    return false;
}

bool ota::requestByPrefix(
    const Type type,
    const char* const prefix,
    const double latitude,
    const double longitude
) {
    char normalizedPrefix[otaDB::CODE_SIZE] {};

    if (
        !_validType(type) ||
        !_validPosition(latitude, longitude) ||
        !uOta::normalizeCodePrefix(
            prefix,
            normalizedPrefix,
            sizeof(normalizedPrefix)
        ) ||
        normalizedPrefix[0] == '\0' ||
        update::isBusy()
    ) {
        return false;
    }

    portENTER_CRITICAL(&_lock);

    RuntimeState &state = _state(type);

    if (
        _taskRunning ||
        state.nearbyStatus == NearbyStatus::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    state.nearbyStatus = NearbyStatus::SEARCHING;
    otaDB::clear(state.nearbyResults);

    _request = TaskRequest {};
    _request.type      = type;
    _request.kind      = TaskKind::BY_PREFIX;
    _request.latitude  = latitude;
    _request.longitude = longitude;

    text::copy(
        _request.value,
        sizeof(_request.value),
        normalizedPrefix
    );

    _taskRunning     = true;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        type == Type::PARKS
            ? "POTA prefix"
            : "SOTA prefix",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) {
        return true;
    }

    portENTER_CRITICAL(&_lock);

    state.nearbyStatus = NearbyStatus::ERROR;
    otaDB::clear(state.nearbyResults);

    _request         = TaskRequest {};
    _taskRunning     = false;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord(
        type == Type::PARKS
            ? "POTA_PREFIX_TASK_CREATE_FAILED"
            : "SOTA_PREFIX_TASK_CREATE_FAILED"
    );

    return false;
}

bool ota::requestNearby(
    const Type type,
    const double latitude,
    const double longitude,
    const double radiusKm
) {
    if (
        !_validType(type) ||
        !_validPosition(latitude, longitude) ||
        !std::isfinite(radiusKm) ||
        radiusKm < 0.0 ||
        update::isBusy()
    ) {
        return false;
    }

    portENTER_CRITICAL(&_lock);

    RuntimeState &state = _state(type);

    if (
        _taskRunning ||
        state.nearbyStatus == NearbyStatus::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    state.nearbyStatus = NearbyStatus::SEARCHING;
    otaDB::clear(state.nearbyResults);

    _request = TaskRequest {};
    _request.type      = type;
    _request.kind      = TaskKind::NEARBY;
    _request.latitude  = latitude;
    _request.longitude = longitude;
    _request.radiusKm  = radiusKm;

    _taskRunning     = true;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        type == Type::PARKS
            ? "POTA nearby"
            : "SOTA nearby",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) {
        return true;
    }

    portENTER_CRITICAL(&_lock);

    state.nearbyStatus = NearbyStatus::ERROR;
    otaDB::clear(state.nearbyResults);

    _request         = TaskRequest {};
    _taskRunning     = false;
    _cancelRequested = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord(
        type == Type::PARKS
            ? "POTA_NEARBY_TASK_CREATE_FAILED"
            : "SOTA_NEARBY_TASK_CREATE_FAILED"
    );

    return false;
}

bool ota::cancel() {
    portENTER_CRITICAL(&_lock);

    if (
        !_taskRunning ||
        (
            _request.kind != TaskKind::NEARBY &&
            _request.kind != TaskKind::BY_PREFIX
        ) ||
        !_validType(_request.type) ||
        _state(_request.type).nearbyStatus != NearbyStatus::SEARCHING ||
        _cancelRequested
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _cancelRequested = true;

    portEXIT_CRITICAL(&_lock);
    return true;
}

ota::Snapshot ota::snapshot(const Type type) {
    Snapshot value {};

    if (!_validType(type)) {
        return value;
    }

    portENTER_CRITICAL(&_lock);

    const RuntimeState &state = _state(type);

    value.status     = state.status;
    value.type       = type;
    value.reference  = state.reference;
    value.distanceKm = state.distanceKm;
    value.bearingDeg = state.bearingDeg;

    portEXIT_CRITICAL(&_lock);

    return value;
}

ota::NearbySnapshot ota::nearbySnapshot(const Type type) {
    NearbySnapshot value {};

    if (!_validType(type)) {
        return value;
    }

    portENTER_CRITICAL(&_lock);

    const RuntimeState &state = _state(type);

    value.status = state.nearbyStatus;
    value.count  = state.nearbyResults.count;

    portEXIT_CRITICAL(&_lock);

    return value;
}

bool ota::nearbyResult(
    const Type type,
    const size_t index,
    otaDB::SearchResult &result
) {
    result = otaDB::SearchResult {};

    if (!_validType(type)) {
        return false;
    }

    portENTER_CRITICAL(&_lock);

    const RuntimeState &state = _state(type);

    if (
        state.nearbyStatus != NearbyStatus::READY ||
        index >= state.nearbyResults.count
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    result = state.nearbyResults.items[index];

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool ota::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);

    return busy;
}
