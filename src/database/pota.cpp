/*
 * src/database/pota.cpp
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
#include <cstdio>
#include <cstring>

#include <esp_heap_caps.h>

#include "database/pota.h"
#include "services/storage.h"
#include "utilities/distance.h"
#include "utilities/text.h"

namespace pota     = database::pota;
namespace storage  = services::storage;
namespace distance = utilities::distance;
namespace ota      = utilities::ota;
namespace text     = utilities::text;
namespace uPota    = utilities::pota;

namespace {
    constexpr const char* DATABASE_PATH  = "/RoverQTH/database/pota.bin";
    constexpr const char* CANDIDATE_PATH = "/RoverQTH/tmp/pota.bin";
    constexpr const char* BACKUP_PATH    = "/RoverQTH/tmp/pota.bak";
    constexpr const char* CSV_HEADER     = "\"reference\",\"name\",\"active\",\"entityId\",\"locationDesc\",\"latitude\",\"longitude\",\"grid\"";

    constexpr uint8_t MAGIC[4] = {'R', 'Q', 'P', 'T'};
    constexpr uint16_t FORMAT_VERSION = 1U;
    constexpr uint32_t MINIMUM_RECORDS = 10000U;
    constexpr size_t RECORD_BUFFER_COUNT = 4U;

    struct __attribute__((packed)) Header {
        uint8_t magic[4];
        uint16_t formatVersion;
        uint16_t headerSize;
        uint16_t recordSize;
        uint16_t reserved;
        uint32_t recordCount;
        uint64_t sourceSize;
        char etag[pota::ETAG_SIZE];
    };

    struct __attribute__((packed)) Record {
        char code[uPota::CODE_SIZE];
        char area[uPota::AREA_SIZE];
        float latitude;
        float longitude;
    };

    struct HeaderReadContext {
        Header* header;
        size_t received;
    };

    struct ValidateCsvContext {
        size_t line;
        uint32_t records;
        bool valid;
    };

    struct WriteCsvContext {
        size_t line;
        uint32_t records;
        uint32_t totalRecords;
        Record buffer[RECORD_BUFFER_COUNT];
        size_t buffered;
        pota::ProgressCallback callback;
        void* userData;
        bool valid;
        bool writeFailed;
    };

    struct SearchContext {
        size_t skipped;
        uint32_t records;
        uint32_t expectedRecords;
        Record record;
        size_t received;
        double latitude;
        double longitude;
        uPota::Park nearest;
        double distanceKm;
        bool found;
        bool valid;
        ota::SearchResults* nearbyResults;
        const char* normalizedPrefix;
        double radiusKm;
        ota::CancelCallback cancelCallback;
        void* cancelUserData;
        bool cancelled;
    };

    struct CodeSearchContext {
        size_t skipped;
        Record record;
        size_t received;
        const char* code;
        uPota::Park park;
        bool found;
        bool valid;
    };

    void _logCandidateFailure(const char* code, uint32_t records, uint32_t totalRecords, size_t buffered);
    bool _validHeader(const Header& header);
    bool _readHeaderChunk(const uint8_t* const data, const size_t length, void* const userData);
    bool _readHeader(const char* const path, Header& header);
    bool _validateCsvLine(const char* const line, void* const userData);
    bool _flushRecords(WriteCsvContext& context);
    bool _writeCsvLine(const char* const line, void* const userData);
    uPota::Park _toPark(const Record& record);
    bool _searchCodeChunk(const uint8_t* data, size_t length, void* userData);
    bool _searchChunk(const uint8_t* data, size_t length, void* const userData);
    ota::SearchStatus _findResults(double latitude, double longitude, double radiusKm,
        const char* normalizedPrefix,
        ota::SearchResults &results,
        ota::CancelCallback cancelCallback,
        void* cancelUserData
    );

    void _logCandidateFailure(const char* const code, const uint32_t records, const uint32_t totalRecords, const size_t buffered) {
        uint8_t progress = 60U;

        if (totalRecords > 0U) {
            const uint64_t calculated =
                60ULL +
                (static_cast<uint64_t>(records) * 35ULL) /
                totalRecords;

            progress = calculated > 95ULL
                ? 95U
                : static_cast<uint8_t>(calculated);
        }

        const size_t candidateSize =
            storage::fileSize(CANDIDATE_PATH);

        const uint64_t expectedSize =
            sizeof(Header) +
            static_cast<uint64_t>(totalRecords) * sizeof(Record);

        const size_t freeHeap =
            heap_caps_get_free_size(MALLOC_CAP_8BIT);

        const size_t largestBlock =
            heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

        char message[192];

        const int written = std::snprintf(
            message,
            sizeof(message),
            "%s progress=%u%% records=%lu/%lu buffered=%u "
            "size=%lu expected=%llu heap=%u largest=%u",
            code,
            static_cast<unsigned int>(progress),
            static_cast<unsigned long>(records),
            static_cast<unsigned long>(totalRecords),
            static_cast<unsigned int>(buffered),
            static_cast<unsigned long>(candidateSize),
            static_cast<unsigned long long>(expectedSize),
            static_cast<unsigned int>(freeHeap),
            static_cast<unsigned int>(largestBlock)
        );

        storage::appendErrorRecord(
            written > 0 &&
            static_cast<size_t>(written) < sizeof(message)
                ? message
                : code
        );
    }

    bool _validHeader(const Header& header) {
        return
            std::memcmp(header.magic, MAGIC, sizeof(MAGIC)) == 0 &&
            header.formatVersion == FORMAT_VERSION  &&
            header.headerSize    == sizeof(Header)  &&
            header.recordSize    == sizeof(Record)  &&
            header.reserved      == 0U              &&
            header.recordCount   >= MINIMUM_RECORDS &&
            header.sourceSize    > 0U               &&
            header.etag[0]       != '\0'            &&
            std::memchr(header.etag, '\0', sizeof(header.etag)) != nullptr;
    }

    bool _readHeaderChunk(const uint8_t* const data, const size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        HeaderReadContext* const context = static_cast<HeaderReadContext*>(userData);
        if (context->header == nullptr || context->received >= sizeof(Header)) { return false; }

        size_t copyLength = sizeof(Header) - context->received;
        if (copyLength > length) { copyLength = length; }

        std::memcpy(reinterpret_cast<uint8_t*>(context->header) + context->received, data, copyLength);
        context->received += copyLength;

        return context->received < sizeof(Header);
    }

    bool _readHeader(const char* const path, Header& header) {
        header = Header {};
        HeaderReadContext context {&header, 0U};

        if (!storage::readFileChunks(path, _readHeaderChunk, &context) ||
            context.received != sizeof(Header) || !_validHeader(header)
        ) { return false; }

        const uint64_t expectedSize = sizeof(Header) +
            static_cast<uint64_t>(header.recordCount) * sizeof(Record);
        return static_cast<uint64_t>(storage::fileSize(path)) == expectedSize;
    }

    bool _validateCsvLine(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        ValidateCsvContext* const context = static_cast<ValidateCsvContext*>(userData);
        if (!context->valid) { return false; }

        if (context->line++ == 0U) {
            context->valid = text::equals(line, CSV_HEADER);
            return context->valid;
        }
        if (line[0] == '\0') { return true; }

        uPota::Park park;
        bool active = false;
        context->valid = uPota::parseCsvRecord(line, park, active);
        if (context->valid && active) { ++context->records; }

        return context->valid;
    }

    bool _flushRecords(WriteCsvContext& context) {
        if (context.buffered == 0U) { return true; }

        const bool written = storage::writeFileChunk(
            reinterpret_cast<const uint8_t*>(context.buffer),
            context.buffered * sizeof(Record)
        );
        if (written) { context.buffered = 0U; }

        return written;
    }

    bool _writeCsvLine(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        WriteCsvContext* const context = static_cast<WriteCsvContext*>(userData);
        if (!context->valid) { return false; }
        if (context->line == 0U) {
            context->valid = text::equals(line, CSV_HEADER);
            ++context->line;
            return context->valid;
        }

        ++context->line;
        if (line[0] == '\0') { return true; }

        uPota::Park park;
        bool active = false;
        if (!uPota::parseCsvRecord(line, park, active)) {
            context->valid = false;
            return false;
        }
        if (!active) { return true; }

        Record& record = context->buffer[context->buffered++];
        record         = Record {};

        if (!text::copy(record.code, sizeof(record.code), park.code) ||
            !text::copy(record.area, sizeof(record.area), park.area)
        ) {
            context->valid = false;
            return false;
        }

        record.latitude  = static_cast<float>(park.latitude);
        record.longitude = static_cast<float>(park.longitude);
        ++context->records;

        if (context->buffered == RECORD_BUFFER_COUNT && !_flushRecords(*context)) {
            context->valid = false;
            context->writeFailed = true;
            return false;
        }

        if (context->callback != nullptr && context->totalRecords > 0U &&
            (context->records & 0x7FU) == 0U
        ) {
            const uint64_t calculated =
                (static_cast<uint64_t>(context->records) * 100ULL) /
                context->totalRecords;

            context->callback(
                calculated > 100ULL
                    ? 100U
                    : static_cast<uint8_t>(calculated),
                context->userData
            );
        }

        return true;
    }

    uPota::Park _toPark(const Record& record) {
        uPota::Park park;

        text::copy(park.code, sizeof(park.code), record.code);
        text::copy(park.area, sizeof(park.area), record.area);

        park.latitude  = record.latitude;
        park.longitude = record.longitude;

        return park;
    }

    bool _searchCodeChunk(const uint8_t* data, size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        CodeSearchContext* const context =
            static_cast<CodeSearchContext*>(userData);

        if (!context->valid || context->found) { return false; }

        if (context->skipped < sizeof(Header)) {
            size_t skip = sizeof(Header) - context->skipped;
            if (skip > length) { skip = length; }

            context->skipped += skip;
            data             += skip;
            length           -= skip;
        }

        while (length > 0U) {
            size_t copyLength = sizeof(Record) - context->received;
            if (copyLength > length) { copyLength = length; }

            std::memcpy(
                reinterpret_cast<uint8_t*>(&context->record) + context->received,
                data,
                copyLength
            );

            context->received += copyLength;
            data              += copyLength;
            length            -= copyLength;

            if (context->received != sizeof(Record)) { continue; }

            if (
                context->record.code[0] == '\0' ||
                context->record.area[0] == '\0' ||
                std::memchr(
                    context->record.code,
                    '\0',
                    sizeof(context->record.code)
                ) == nullptr ||
                std::memchr(
                    context->record.area,
                    '\0',
                    sizeof(context->record.area)
                ) == nullptr
            ) {
                context->valid = false;
                return false;
            }

            if (text::equals(context->record.code, context->code)) {
                context->park  = _toPark(context->record);
                context->found = true;
                return false;
            }

            context->record   = Record {};
            context->received = 0U;
        }

        return true;
    }

    bool _searchChunk(const uint8_t* data, size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        SearchContext* const context = static_cast<SearchContext*>(userData);
        if (!context->valid) { return false; }

        if (context->cancelCallback != nullptr &&
            context->cancelCallback(context->cancelUserData)
        ) {
            context->cancelled = true;
            return false;
        }

        if (context->skipped < sizeof(Header)) {
            size_t skip = sizeof(Header) - context->skipped;
            if (skip > length) { skip = length; }

            context->skipped += skip;
            data             += skip;
            length           -= skip;
        }

        while (length > 0U) {
            if (context->cancelCallback != nullptr &&
                context->cancelCallback(context->cancelUserData)
            ) {
                context->cancelled = true;
                return false;
            }

            size_t copyLength = sizeof(Record) - context->received;
            if (copyLength > length) { copyLength = length; }

            std::memcpy(reinterpret_cast<uint8_t*>(&context->record) + context->received, data, copyLength);
            context->received += copyLength;
            data              += copyLength;
            length            -= copyLength;
            if (context->received != sizeof(Record)) { continue; }

            if (context->record.code[0] == '\0' ||
                context->record.area[0] == '\0' ||
                std::memchr(context->record.code, '\0', sizeof(context->record.code)) == nullptr ||
                std::memchr(context->record.area, '\0', sizeof(context->record.area)) == nullptr
            ) {
                context->valid = false;
                return false;
            }

            const uPota::Park candidate = _toPark(context->record);

            if (context->nearbyResults == nullptr) {
                context->valid = uPota::selectNearest(
                    context->latitude,
                    context->longitude,
                    candidate,
                    context->found,
                    context->nearest,
                    context->distanceKm
                );
            } else {
                bool candidateValid = false;
                uPota::Park validatedCandidate {};
                double candidateDistanceKm = 0.0;

                context->valid = uPota::selectNearest(
                    context->latitude,
                    context->longitude,
                    candidate,
                    candidateValid,
                    validatedCandidate,
                    candidateDistanceKm
                );

                if (context->valid) {
                    context->found = true;

                    const bool withinRadius =
                        context->radiusKm == 0.0 ||
                        candidateDistanceKm <= context->radiusKm;

                    bool prefixMatches = true;

                    if (context->normalizedPrefix != nullptr) {
                        char normalizedCode[ota::CODE_SIZE] {};

                        if (!ota::normalizeCodePrefix(
                            candidate.code,
                            normalizedCode,
                            sizeof(normalizedCode)
                        )) {
                            context->valid = false;
                            return false;
                        }

                        prefixMatches = text::startsWith(
                            normalizedCode,
                            context->normalizedPrefix
                        );
                    }

                    if (withinRadius && prefixMatches) {
                        ota::SearchResult result {};
                        context->valid = text::copy(
                            result.code,
                            sizeof(result.code),
                            candidate.code
                        );

                        if (context->valid) {
                            result.distanceKm = candidateDistanceKm;

                            ota::retainNearest(
                                *context->nearbyResults,
                                result
                            );
                        }
                    }
                }
            }

            context->received = 0U;
            ++context->records;
            if (!context->valid || context->records > context->expectedRecords) {
                context->valid = false;
                return false;
            }
        }

        return true;
    }

    ota::SearchStatus _findResults(double latitude, double longitude, double radiusKm,
        const char* normalizedPrefix,
        ota::SearchResults &results,
        ota::CancelCallback cancelCallback,
        void* cancelUserData
    ) {
        ota::clear(results);

        if (
            !std::isfinite(latitude) ||
            !std::isfinite(longitude) ||
            !std::isfinite(radiusKm) ||
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0 ||
            radiusKm < 0.0
        ) {
            return ota::SearchStatus::ERROR;
        }

        if (normalizedPrefix != nullptr) {
            char validatedPrefix[ota::CODE_SIZE] {};

            if (
                !ota::normalizeCodePrefix(
                    normalizedPrefix,
                    validatedPrefix,
                    sizeof(validatedPrefix)
                ) ||
                !text::equals(validatedPrefix, normalizedPrefix)
            ) {
                return ota::SearchStatus::ERROR;
            }
        }

        Header header {};

        if (!_readHeader(DATABASE_PATH, header)) {
            return ota::SearchStatus::ERROR;
        }

        SearchContext context {
            0U,
            0U,
            header.recordCount,
            {},
            0U,
            latitude,
            longitude,
            {},
            0.0,
            false,
            true,
            &results,
            normalizedPrefix,
            radiusKm,
            cancelCallback,
            cancelUserData,
            false
        };

        const bool readComplete =
            storage::readFileChunks(
                DATABASE_PATH,
                _searchChunk,
                &context
            );

        if (context.cancelled) {
            ota::clear(results);
            return ota::SearchStatus::CANCELLED;
        }

        if (
            !readComplete ||
            !context.valid ||
            context.received != 0U ||
            context.records != header.recordCount ||
            !context.found
        ) {
            ota::clear(results);
            return ota::SearchStatus::ERROR;
        }

        return ota::SearchStatus::SUCCESS;
    }
}

bool pota::info(Info& value) {
    value = Info {};

    Header header {};

    if (!_readHeader(DATABASE_PATH, header)) {
        if (storage::fileExists(DATABASE_PATH)) { return false; }

        Header backup {};
        if (!_readHeader(BACKUP_PATH, backup) ||
            !storage::renameFile(BACKUP_PATH, DATABASE_PATH)
        ) { return false; }

        header = backup;
        storage::appendErrorRecord("POTA_BACKUP_RECOVERED");
    }

    value.records    = header.recordCount;
    value.sourceSize = header.sourceSize;

    return text::copy(value.etag, sizeof(value.etag), header.etag);
}

bool pota::buildCandidate(const char* const csvPath, const char* const etag,
    const uint64_t sourceSize, const ProgressCallback callback, void* const userData
) {
    discardCandidate();

    if (csvPath == nullptr || csvPath[0] == '\0' ||
        etag == nullptr    || etag[0] == '\0'    ||
        sourceSize == 0U
    ) {
        storage::appendErrorRecord("POTA_CANDIDATE_INPUT_INVALID");
        return false;
    }

    if (static_cast<uint64_t>(storage::fileSize(csvPath)) != sourceSize) {
        storage::appendErrorRecord("POTA_SOURCE_SIZE_MISMATCH");
        return false;
    }

    ValidateCsvContext validation {0U, 0U, true};
    const bool validationRead =
        storage::readFileLines(csvPath, _validateCsvLine, &validation);

    if (!validationRead) {
        storage::appendErrorRecord("POTA_SOURCE_READ_FAILED");
        return false;
    }
    if (!validation.valid) {
        storage::appendErrorRecord("POTA_SOURCE_CSV_INVALID");
        return false;
    }
    if (validation.line < 2U) {
        storage::appendErrorRecord("POTA_SOURCE_TRUNCATED");
        return false;
    }
    if (validation.records < MINIMUM_RECORDS) {
        storage::appendErrorRecord("POTA_SOURCE_RECORDS_INSUFFICIENT");
        return false;
    }

    Header header {};
    std::memcpy(header.magic, MAGIC, sizeof(MAGIC));

    header.formatVersion = FORMAT_VERSION;
    header.headerSize    = sizeof(Header);
    header.recordSize    = sizeof(Record);
    header.recordCount   = validation.records;
    header.sourceSize    = sourceSize;

    if (!text::copy(header.etag, sizeof(header.etag), etag)) {
        storage::appendErrorRecord("POTA_CANDIDATE_METADATA_INVALID");
        return false;
    }

    if (!storage::beginFileWrite(CANDIDATE_PATH)) {
        _logCandidateFailure(
            "POTA_CANDIDATE_OPEN_FAILED",
            0U,
            validation.records,
            0U
        );
        return false;
    }

    const bool headerWritten = storage::writeFileChunk(
        reinterpret_cast<const uint8_t*>(&header), sizeof(header)
    );
    if (!headerWritten) {
        storage::endFileWrite();
        _logCandidateFailure(
            "POTA_CANDIDATE_WRITE_FAILED",
            0U,
            validation.records,
            0U
        );
        discardCandidate();
        return false;
    }

    WriteCsvContext writer {0U, 0U, validation.records, {}, 0U, callback, userData, true, false};
    const bool read = storage::readFileLines(csvPath, _writeCsvLine, &writer);

    bool flushed = false;
    if (read && writer.valid && writer.records == validation.records) {
        flushed = _flushRecords(writer);
        writer.writeFailed = !flushed;
    }

    storage::endFileWrite();
    if (!read || !writer.valid || writer.records != validation.records || !flushed) {
        if (writer.writeFailed) {
            _logCandidateFailure(
                "POTA_CANDIDATE_WRITE_FAILED",
                writer.records,
                validation.records,
                writer.buffered
            );
        } else if (!writer.valid) {
            _logCandidateFailure(
                "POTA_CANDIDATE_PARSE_FAILED",
                writer.records,
                validation.records,
                writer.buffered
            );
        } else if (writer.records != validation.records) {
            _logCandidateFailure(
                "POTA_CANDIDATE_COUNT_MISMATCH",
                writer.records,
                validation.records,
                writer.buffered
            );
        } else {
            _logCandidateFailure(
                "POTA_CANDIDATE_READ_FAILED",
                writer.records,
                validation.records,
                writer.buffered
            );
        }
        discardCandidate();
        return false;
    }

    Header candidate {};
    if (!_readHeader(CANDIDATE_PATH, candidate)) {
        _logCandidateFailure(
            "POTA_CANDIDATE_HEADER_INVALID",
            validation.records,
            validation.records,
            0U
        );
        discardCandidate();
        return false;
    }

    SearchContext verification {
        0U, 0U,
        candidate.recordCount, {}, 0U,
        0.0, 0.0, {}, 0.0, false, true,
        nullptr, nullptr, 0.0, nullptr, nullptr, false
    };

    if (!storage::readFileChunks(CANDIDATE_PATH, _searchChunk, &verification) ||
        !verification.valid         ||
        verification.received != 0U ||
        verification.records != candidate.recordCount ||
        !verification.found
    ) {
        _logCandidateFailure(
            "POTA_CANDIDATE_CONTENT_INVALID",
            verification.records,
            candidate.recordCount,
            0U
        );
        discardCandidate();
        return false;
    }

    if (callback != nullptr) { callback(100U, userData); }
    return true;
}

bool pota::installCandidate() {
    Header candidate {};
    if (!_readHeader(CANDIDATE_PATH, candidate)) {
        storage::appendErrorRecord("POTA_CANDIDATE_INVALID");
        return false;
    }

    if (storage::fileExists(BACKUP_PATH) &&
        !storage::deleteFile(BACKUP_PATH)
    ) {
        storage::appendErrorRecord("POTA_BACKUP_DELETE_FAILED");
        return false;
    }

    const bool hadInstalled = storage::fileExists(DATABASE_PATH);

    if (hadInstalled && !storage::renameFile(DATABASE_PATH, BACKUP_PATH)) {
        storage::appendErrorRecord("POTA_BACKUP_CREATE_FAILED");
        return false;
    }

    if (!storage::renameFile(CANDIDATE_PATH, DATABASE_PATH)) {
        storage::appendErrorRecord("POTA_CANDIDATE_INSTALL_FAILED");

        if (hadInstalled && !storage::renameFile(BACKUP_PATH, DATABASE_PATH))
            { storage::appendErrorRecord("POTA_ROLLBACK_FAILED"); }
        return false;
    }

    Info installed;
    if (!info(installed)) {
        storage::appendErrorRecord("POTA_INSTALLED_INVALID");

        bool rollbackValid = storage::deleteFile(DATABASE_PATH);

        if (rollbackValid && hadInstalled) {
            rollbackValid = storage::renameFile(BACKUP_PATH, DATABASE_PATH);
        }

        if (!rollbackValid) {
            storage::appendErrorRecord("POTA_ROLLBACK_FAILED");
        }
        return false;
    }

    if (hadInstalled && !storage::deleteFile(BACKUP_PATH))
        { storage::appendErrorRecord("POTA_BACKUP_DELETE_FAILED"); }
    return true;
}

void pota::discardCandidate() {
    storage::deleteFile(CANDIDATE_PATH);
}

bool pota::findByCode(
    const char* const code,
    uPota::Park& park
) {
    park = uPota::Park {};

    if (
        code == nullptr ||
        code[0] == '\0' ||
        std::memchr(code, '\0', uPota::CODE_SIZE) == nullptr
    ) {
        return false;
    }

    Header header {};
    if (!_readHeader(DATABASE_PATH, header)) { return false; }

    CodeSearchContext context {
        0U,
        {},
        0U,
        code,
        {},
        false,
        true
    };

    const bool readComplete =
        storage::readFileChunks(DATABASE_PATH, _searchCodeChunk, &context);

    if (
        !readComplete ||
        !context.valid ||
        !context.found
    ) {
        return false;
    }

    park = context.park;
    return true;
}

bool pota::findNearest(const double latitude, const double longitude,
    uPota::Park& park, double& distanceKm, double& bearing
) {
    park       = uPota::Park {};
    distanceKm = 0.0;
    bearing    = 0.0;

    Header header {};
    if (!_readHeader(DATABASE_PATH, header)) { return false; }

    SearchContext context {
        0U, 0U,
        header.recordCount, {}, 0U,
        latitude, longitude, {}, 0.0, false, true,
        nullptr, nullptr, 0.0, nullptr, nullptr, false
    };

    if (!storage::readFileChunks(DATABASE_PATH, _searchChunk, &context) ||
        !context.valid || context.received != 0U ||
        context.records != header.recordCount || !context.found
    ) { return false; }

    park       = context.nearest;
    distanceKm = context.distanceKm;
    bearing    = distance::bearingDegrees(
        latitude,      longitude,
        park.latitude, park.longitude
    );

    return true;
}

ota::SearchStatus pota::findByPrefix(const double latitude, const double longitude,
    const char* const normalizedPrefix,
    ota::SearchResults &results,
    const ota::CancelCallback cancelCallback,
    void* const cancelUserData
) {
    if (
        normalizedPrefix == nullptr ||
        normalizedPrefix[0] == '\0'
    ) {
        ota::clear(results);
        return ota::SearchStatus::ERROR;
    }

    return _findResults(
        latitude,
        longitude,
        0.0,
        normalizedPrefix,
        results,
        cancelCallback,
        cancelUserData
    );
}

ota::SearchStatus pota::findNearby(const double latitude, const double longitude, const double radiusKm,
    ota::SearchResults &results,
    const ota::CancelCallback cancelCallback,
    void* const cancelUserData
) {
    return _findResults(
        latitude,
        longitude,
        radiusKm,
        nullptr,
        results,
        cancelCallback,
        cancelUserData
    );
}
