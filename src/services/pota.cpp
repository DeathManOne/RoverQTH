/*
 * src/services/pota.cpp
 *
 * Copyright (c) 2026 DeathManOne
 * https://github.com/DeathManOne
 * 
 * This file is part of the RoverQTH project.
 *
 * RoverQTH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RoverQTH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RoverQTH.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "database/pota.h"
#include "services/pota.h"
#include "services/storage.h"
#include "services/update.h"
#include "utilities/distance.h"
#include "utilities/text.h"

namespace potaDB   = database::pota;
namespace pota     = services::pota;
namespace storage  = services::storage;
namespace update   = services::update;
namespace distance = utilities::distance;
namespace text     = utilities::text;
namespace ota      = utilities::ota;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr double REFRESH_DISTANCE_KM = 1.0;
    constexpr uint32_t TASK_STACK_SIZE   = 8192U;

    enum class TaskKind : uint8_t {
        NONE,
        NEAREST,
        BY_CODE,
        NEARBY
    };

    pota::Status _status             = pota::Status::UNAVAILABLE;
    pota::NearbyStatus _nearbyStatus = pota::NearbyStatus::UNAVAILABLE;

    char _requestedCode[utilities::pota::CODE_SIZE] {};
    utilities::pota::Park _park       {};
    ota::SearchResults _nearbyResults {};
    double _requestedRadiusKm    = 0.0;
    double _distanceKm           = 0.0;
    double _bearingDeg           = 0.0;
    double _requestedLatitude    = 0.0;
    double _requestedLongitude   = 0.0;
    double _searchedLatitude     = 0.0;
    double _searchedLongitude    = 0.0;
    bool _hasSearchedPosition    = false;
    bool _taskRunning            = false;
    TaskKind _taskKind           = TaskKind::NONE;
    bool _cancelNearbyRequested  = false;

    bool _validPosition(const double latitude, const double longitude);
    bool _nearbyCancellationRequested(void*);
    void _searchTask(void*);
    void _codeSearchTask(void*);
    void _nearbySearchTask(void*);

    bool _validPosition(const double latitude, const double longitude) {
        return std::isfinite(latitude) && std::isfinite(longitude) &&
            latitude >= -90.0          && latitude <= 90.0 &&
            longitude >= -180.0        && longitude <= 180.0;
    }

    bool _nearbyCancellationRequested(void*) {
        portENTER_CRITICAL(&_lock);
        const bool requested =
            _taskRunning &&
            _taskKind == TaskKind::NEARBY &&
            _cancelNearbyRequested;
        portEXIT_CRITICAL(&_lock);

        return requested;
    }

    void _searchTask(void*) {
        double latitude;
        double longitude;

        portENTER_CRITICAL(&_lock);
        latitude  = _requestedLatitude;
        longitude = _requestedLongitude;
        portEXIT_CRITICAL(&_lock);

        utilities::pota::Park park {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
        const bool found = potaDB::findNearest(
            latitude,
            longitude,
            park,
            distanceKm,
            bearingDeg
        );

        if (!found) { storage::appendErrorRecord("POTA_SEARCH_FAILED"); }
        portENTER_CRITICAL(&_lock);

        if (found) {
            _park                = park;
            _distanceKm          = distanceKm;
            _bearingDeg          = bearingDeg;
            _searchedLatitude    = latitude;
            _searchedLongitude   = longitude;
            _hasSearchedPosition = true;
            _status              = pota::Status::READY;
        } else {
            _park       = utilities::pota::Park {};
            _distanceKm = 0.0;
            _bearingDeg = 0.0;
            _status     = pota::Status::ERROR;
        }

        _taskRunning           = false;
        _taskKind              = TaskKind::NONE;
        _cancelNearbyRequested = false;
        portEXIT_CRITICAL(&_lock);

        vTaskDelete(nullptr);
    }

    void _codeSearchTask(void*) {
        char code[utilities::pota::CODE_SIZE] {};
        double latitude;
        double longitude;

        portENTER_CRITICAL(&_lock);
        text::copy(code, sizeof(code), _requestedCode);
        latitude  = _requestedLatitude;
        longitude = _requestedLongitude;
        portEXIT_CRITICAL(&_lock);

        utilities::pota::Park park {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;

        bool found = potaDB::findByCode(code, park);

        if (found) {
            distanceKm = distance::betweenKilometers(
                latitude,
                longitude,
                park.latitude,
                park.longitude
            );

            bearingDeg = distance::bearingDegrees(
                latitude,
                longitude,
                park.latitude,
                park.longitude
            );

            found =
                std::isfinite(distanceKm) &&
                std::isfinite(bearingDeg);
        }

        if (!found) {
            storage::appendErrorRecord("POTA_CODE_SEARCH_FAILED");
        }

        portENTER_CRITICAL(&_lock);

        if (found) {
            _park       = park;
            _distanceKm = distanceKm;
            _bearingDeg = bearingDeg;
            _status     = pota::Status::READY;
        } else {
            _park       = utilities::pota::Park {};
            _distanceKm = 0.0;
            _bearingDeg = 0.0;
            _status     = pota::Status::ERROR;
        }

        _hasSearchedPosition   = false;
        _taskRunning           = false;
        _taskKind              = TaskKind::NONE;
        _cancelNearbyRequested = false;

        portEXIT_CRITICAL(&_lock);
        vTaskDelete(nullptr);
    }

    void _nearbySearchTask(void*) {
        double latitude;
        double longitude;
        double radiusKm;

        portENTER_CRITICAL(&_lock);
        latitude  = _requestedLatitude;
        longitude = _requestedLongitude;
        radiusKm  = _requestedRadiusKm;
        portEXIT_CRITICAL(&_lock);

        ota::SearchResults results {};
        const ota::SearchStatus searchStatus = potaDB::findNearby(
            latitude,
            longitude,
            radiusKm,
            results,
            _nearbyCancellationRequested,
            nullptr
        );

        if (searchStatus == ota::SearchStatus::ERROR) {
            storage::appendErrorRecord("POTA_NEARBY_SEARCH_FAILED");
        } else if (searchStatus == ota::SearchStatus::CANCELLED) {
            storage::appendLogRecord("POTA_NEARBY_SEARCH_CANCELLED");
        }

        portENTER_CRITICAL(&_lock);

        switch (searchStatus) {
            case ota::SearchStatus::SUCCESS:
                _nearbyResults = results;
                _nearbyStatus  = results.count > 0U
                    ? pota::NearbyStatus::READY
                    : pota::NearbyStatus::EMPTY;
                break;

            case ota::SearchStatus::CANCELLED:
                ota::clear(_nearbyResults);
                _nearbyStatus = pota::NearbyStatus::IDLE;
                break;

            case ota::SearchStatus::ERROR:
                ota::clear(_nearbyResults);
                _nearbyStatus = pota::NearbyStatus::ERROR;
                break;
        }

        _taskRunning           = false;
        _taskKind              = TaskKind::NONE;
        _cancelNearbyRequested = false;
        portEXIT_CRITICAL(&_lock);
        vTaskDelete(nullptr);
    }
}

void pota::begin() {
    potaDB::Info info {};
    const bool available = potaDB::info(info);

    portENTER_CRITICAL(&_lock);
    _status                 = available ? Status::IDLE : Status::UNAVAILABLE;
    _park                   = utilities::pota::Park {};
    _requestedCode[0]       = '\0';
    _distanceKm             = 0.0;
    _bearingDeg             = 0.0;
    _hasSearchedPosition    = false;
    _nearbyStatus           = available ? NearbyStatus::IDLE : NearbyStatus::UNAVAILABLE;
    ota::clear(_nearbyResults);
    _requestedRadiusKm      = 0.0;
    _taskRunning            = false;
    _taskKind               = TaskKind::NONE;
    _cancelNearbyRequested  = false;
    portEXIT_CRITICAL(&_lock);

    storage::appendLogRecord(
        available
            ? "POTA_SERVICE_READY status=available"
            : "POTA_SERVICE_READY status=unavailable"
    );
}

void pota::invalidate() {
    portENTER_CRITICAL(&_lock);
    _status                 = Status::IDLE;
    _park                   = utilities::pota::Park {};
    _requestedCode[0]       = '\0';
    _distanceKm             = 0.0;
    _bearingDeg             = 0.0;
    _hasSearchedPosition    = false;
    _nearbyStatus           = NearbyStatus::IDLE;
    ota::clear(_nearbyResults);
    _requestedRadiusKm      = 0.0;
    _taskRunning            = false;
    _taskKind               = TaskKind::NONE;
    _cancelNearbyRequested  = false;
    portEXIT_CRITICAL(&_lock);
}

bool pota::requestNearest(const double latitude, const double longitude) {
    if (!_validPosition(latitude, longitude) || update::isBusy()) { return false; }

    bool hasSearchedPosition;
    Status status;
    double searchedLatitude;
    double searchedLongitude;

    portENTER_CRITICAL(&_lock);
    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }
    hasSearchedPosition = _hasSearchedPosition;
    status              = _status;
    searchedLatitude    = _searchedLatitude;
    searchedLongitude   = _searchedLongitude;
    portEXIT_CRITICAL(&_lock);

    if (status == Status::UNAVAILABLE) { return false; }
    if (hasSearchedPosition && status == Status::READY) {
        const double movedKm = distance::betweenKilometers(
            searchedLatitude, searchedLongitude,
            latitude,         longitude
        );
        if (std::isfinite(movedKm) && movedKm < REFRESH_DISTANCE_KM) { return true; }
    }

    portENTER_CRITICAL(&_lock);
    if (_taskRunning || update::isBusy()) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }
    _requestedLatitude     = latitude;
    _requestedLongitude    = longitude;
    _status                = Status::SEARCHING;
    _park                  = utilities::pota::Park {};
    _distanceKm            = 0.0;
    _bearingDeg            = 0.0;
    _hasSearchedPosition   = false;
    _taskRunning           = true;
    _taskKind              = TaskKind::NEAREST;
    _cancelNearbyRequested = false;
    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        "POTA nearest",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );
    if (created == pdPASS) { return true; }

    portENTER_CRITICAL(&_lock);
    _taskRunning           = false;
    _taskKind              = TaskKind::NONE;
    _cancelNearbyRequested = false;
    _status                = Status::ERROR;
    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord("POTA_SEARCH_TASK_CREATE_FAILED");
    return false;
}

bool pota::requestByCode(const char* const code, const double latitude, const double longitude) {
    if (
        code == nullptr ||
        code[0] == '\0' ||
        std::memchr(code, '\0', utilities::pota::CODE_SIZE) == nullptr ||
        !_validPosition(latitude, longitude) ||
        update::isBusy()
    ) {
        return false;
    }

    Status status;
    utilities::pota::Park cachedPark {};
    char requestedCode[utilities::pota::CODE_SIZE] {};

    portENTER_CRITICAL(&_lock);

    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    status     = _status;
    cachedPark = _park;
    text::copy(requestedCode, sizeof(requestedCode), _requestedCode);

    portEXIT_CRITICAL(&_lock);

    if (status == Status::UNAVAILABLE) { return false; }

    if (text::equals(requestedCode, code)) {
        if (status == Status::ERROR) { return false; }

        if (
            status == Status::READY &&
            cachedPark.code[0] != '\0'
        ) {
            const double distanceKm = distance::betweenKilometers(
                latitude,
                longitude,
                cachedPark.latitude,
                cachedPark.longitude
            );

            const double bearingDeg = distance::bearingDegrees(
                latitude,
                longitude,
                cachedPark.latitude,
                cachedPark.longitude
            );

            if (
                !std::isfinite(distanceKm) ||
                !std::isfinite(bearingDeg)
            ) {
                return false;
            }

            portENTER_CRITICAL(&_lock);

            if (
                _taskRunning ||
                _status != Status::READY ||
                !text::equals(_requestedCode, code)
            ) {
                portEXIT_CRITICAL(&_lock);
                return false;
            }

            _distanceKm = distanceKm;
            _bearingDeg = bearingDeg;

            portEXIT_CRITICAL(&_lock);
            return true;
        }
    }

    portENTER_CRITICAL(&_lock);

    if (
        _taskRunning ||
        _status == Status::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    text::copy(_requestedCode, sizeof(_requestedCode), code);
    _requestedLatitude      = latitude;
    _requestedLongitude     = longitude;
    _status                 = Status::SEARCHING;
    _park                   = utilities::pota::Park {};
    _distanceKm             = 0.0;
    _bearingDeg             = 0.0;
    _hasSearchedPosition    = false;
    _taskRunning            = true;
    _taskKind               = TaskKind::BY_CODE;
    _cancelNearbyRequested  = false;

    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _codeSearchTask,
        "POTA code",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) { return true; }

    portENTER_CRITICAL(&_lock);

    _status                 = Status::ERROR;
    _park                   = utilities::pota::Park {};
    _distanceKm             = 0.0;
    _bearingDeg             = 0.0;
    _taskRunning            = false;
    _taskKind               = TaskKind::NONE;
    _cancelNearbyRequested  = false;

    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord("POTA_CODE_TASK_CREATE_FAILED");
    return false;
}

bool pota::requestNearby(const double latitude, const double longitude, const double radiusKm) {
    if (!_validPosition(latitude, longitude) || !std::isfinite(radiusKm) ||
        radiusKm < 0.0                       || update::isBusy()
    ) { return false; }

    portENTER_CRITICAL(&_lock);
    if (_taskRunning ||
        _nearbyStatus == NearbyStatus::UNAVAILABLE ||
        update::isBusy()
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _requestedLatitude     = latitude;
    _requestedLongitude    = longitude;
    _requestedRadiusKm     = radiusKm;
    _nearbyStatus          = NearbyStatus::SEARCHING;
    ota::clear(_nearbyResults);
    _taskRunning            = true;
    _taskKind               = TaskKind::NEARBY;
    _cancelNearbyRequested  = false;
    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _nearbySearchTask,
        "POTA nearby",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );

    if (created == pdPASS) { return true; }

    portENTER_CRITICAL(&_lock);
    _taskRunning           = false;
    _taskKind              = TaskKind::NONE;
    _cancelNearbyRequested = false;
    _nearbyStatus          = NearbyStatus::ERROR;
    ota::clear(_nearbyResults);
    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord(
        "POTA_NEARBY_TASK_CREATE_FAILED"
    );
    return false;
}

bool pota::cancelNearby() {
    portENTER_CRITICAL(&_lock);

    if (!_taskRunning ||
        _taskKind != TaskKind::NEARBY ||
        _nearbyStatus != NearbyStatus::SEARCHING ||
        _cancelNearbyRequested
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _cancelNearbyRequested = true;
    portEXIT_CRITICAL(&_lock);
    return true;
}

pota::NearbySnapshot pota::nearbySnapshot() {
    NearbySnapshot value;

    portENTER_CRITICAL(&_lock);
    value.status = _nearbyStatus;
    value.count  = _nearbyResults.count;
    portEXIT_CRITICAL(&_lock);

    return value;
}

bool pota::nearbyResult(const size_t index, ota::SearchResult &result) {
    result = ota::SearchResult {};

    portENTER_CRITICAL(&_lock);

    if (_nearbyStatus != NearbyStatus::READY ||
        index >= _nearbyResults.count
    ) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    result = _nearbyResults.items[index];
    portEXIT_CRITICAL(&_lock);
    return true;
}

pota::Snapshot pota::snapshot() {
    Snapshot value;

    portENTER_CRITICAL(&_lock);
    value.status     = _status;
    value.park       = _park;
    value.distanceKm = _distanceKm;
    value.bearingDeg = _bearingDeg;
    portEXIT_CRITICAL(&_lock);

    return value;
}

bool pota::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);

    return busy;
}
