/*
 * src/database/sota.cpp
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

#include <cstring>
#include "database/sota.h"
#include "services/storage.h"
#include "utilities/distance.h"
#include "utilities/text.h"

namespace sota     = database::sota;
namespace storage  = services::storage;
namespace distance = utilities::distance;
namespace text     = utilities::text;
namespace uSota    = utilities::sota;

namespace {
    constexpr const char* DATABASE_PATH  = "/RoverQTH/database/sota.bin";
    constexpr const char* CANDIDATE_PATH = "/RoverQTH/tmp/sota.bin";
    constexpr const char* BACKUP_PATH    = "/RoverQTH/tmp/sota.bak";

    constexpr uint8_t MAGIC[4] = {'R', 'Q', 'S', 'T'};
    constexpr uint16_t FORMAT_VERSION = 3U;
    constexpr uint32_t MINIMUM_RECORDS = 1000U;
    constexpr size_t RECORD_BUFFER_COUNT = 8U;

    struct __attribute__((packed)) Header {
        uint8_t magic[4];
        uint16_t formatVersion;
        uint16_t headerSize;
        uint16_t recordSize;
        uint16_t reserved;
        uint32_t recordCount;
        uint64_t sourceSize;
        char version[uSota::VERSION_SIZE];
        char etag[sota::ETAG_SIZE];
    };

    struct __attribute__((packed)) Record {
        char code[uSota::CODE_SIZE];
        char area[uSota::AREA_SIZE];
        float latitude;
        float longitude;
        int16_t altitude;
        uint8_t points;
        uint8_t bonus;
    };

    struct HeaderReadContext {
        Header* header;
        size_t received;
    };

    struct ValidateCsvContext {
        const char* version;
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
        sota::ProgressCallback callback;
        void* userData;
        bool valid;
    };

    struct SearchContext {
        size_t skipped;
        uint32_t records;
        uint32_t expectedRecords;
        Record record;
        size_t received;
        double latitude;
        double longitude;
        uSota::Summit nearest;
        double distanceKm;
        bool found;
        bool valid;
    };

    bool _validHeader(const Header& header);
    bool _readHeader(const char* const path, Header& header);
    bool _readHeaderChunk(const uint8_t* const data, const size_t length, void* const userData);
    bool _validateCsvLine(const char* const line, void* const userData);
    bool _flushRecords(WriteCsvContext& context);
    bool _writeCsvLine(const char* const line, void* const userData);
    uSota::Summit _toSummit(const Record& record);
    bool _searchChunk(const uint8_t* data, size_t length, void* const userData);

    bool _validHeader(const Header& header) {
        return std::memcmp(header.magic, MAGIC, sizeof(MAGIC)) == 0 &&
            header.formatVersion == FORMAT_VERSION  &&
            header.headerSize    == sizeof(Header)  &&
            header.recordSize    == sizeof(Record)  &&
            header.recordCount   >= MINIMUM_RECORDS &&
            header.sourceSize    > 0U               &&
            uSota::isVersionValid(header.version)   &&
            header.etag[0] != '\0'                  &&
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

        const size_t expectedSize = sizeof(Header) + static_cast<size_t>(header.recordCount) * sizeof(Record);
        return storage::fileSize(path) == expectedSize;
    }

    bool _validateCsvLine(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        ValidateCsvContext* const context = static_cast<ValidateCsvContext*>(userData);
        if (!context->valid) { return false; }

        if (context->line == 0U) {
            char version[uSota::VERSION_SIZE];
            context->valid = uSota::parseListVersion(line, version, sizeof(version)) && text::equals(version, context->version);
        } else if (context->line == 1U) {
            context->valid = std::strstr(line, "SummitCode") != nullptr;
        } else if (line[0] != '\0') {
            uSota::Summit summit;
            context->valid = uSota::parseCsvRecord(line, summit);
            if (context->valid)
                { ++context->records; }
        }
        ++context->line;
        return context->valid;
    }

    bool _flushRecords(WriteCsvContext& context) {
        if (context.buffered == 0U) { return true; }

        const bool written = storage::writeFileChunk(
            reinterpret_cast<const uint8_t*>(context.buffer),
            context.buffered * sizeof(Record)
        );

        if (written)
            { context.buffered = 0U; }
        return written;
    }

    bool _writeCsvLine(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        WriteCsvContext* const context = static_cast<WriteCsvContext*>(userData);
        if (!context->valid)                         { return false; }
        if (context->line++ < 2U || line[0] == '\0') { return true; }

        uSota::Summit summit;
        if (!uSota::parseCsvRecord(line, summit)) {
            context->valid = false;
            return false;
        }

        Record& record = context->buffer[context->buffered++];
        record         = Record {};

        text::copy(record.code, sizeof(record.code), summit.code);
        text::copy(record.area, sizeof(record.area), summit.area);

        record.latitude  = static_cast<float>(summit.latitude);
        record.longitude = static_cast<float>(summit.longitude);
        record.altitude  = summit.altitude;
        record.points    = summit.points;
        record.bonus     = summit.bonus;
        ++context->records;

        if (context->buffered == RECORD_BUFFER_COUNT && !_flushRecords(*context)) {
            context->valid = false;
            return false;
        }
        if (context->callback != nullptr && context->totalRecords > 0U && (context->records & 0x7FU) == 0U) {
            context->callback(static_cast<uint8_t>(
                (static_cast<uint64_t>(context->records) * 100ULL) / context->totalRecords),
                context->userData
            );
        }
        return true;
    }

    uSota::Summit _toSummit(const Record& record) {
        uSota::Summit summit;

        text::copy(summit.code, sizeof(summit.code), record.code);
        text::copy(summit.area, sizeof(summit.area), record.area);

        summit.latitude  = record.latitude;
        summit.longitude = record.longitude;
        summit.altitude  = record.altitude;
        summit.points    = record.points;
        summit.bonus     = record.bonus;
        return summit;
    }

    bool _searchChunk(const uint8_t* data, size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        SearchContext* const context = static_cast<SearchContext*>(userData);
        if (!context->valid) { return false; }

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

            std::memcpy(reinterpret_cast<uint8_t*>(&context->record) + context->received, data, copyLength);
            context->received += copyLength;
            data              += copyLength;
            length            -= copyLength;
            if (context->received != sizeof(Record)) { continue; }

            const uSota::Summit candidate = _toSummit(context->record);
            context->valid                = uSota::selectNearest(
                context->latitude, context->longitude, candidate,
                context->found,    context->nearest,   context->distanceKm
            );
            context->received = 0U;
            ++context->records;
            if (!context->valid || context->records > context->expectedRecords)
                { context->valid = false; return false; }
        }
        return true;
    }
}

bool sota::info(Info& value) {
    value = Info {};

    Header header {};
    if (!_readHeader(DATABASE_PATH, header)) { return false; }

    value.records    = header.recordCount;
    value.sourceSize = header.sourceSize;

    return
        text::copy(value.version, sizeof(value.version), header.version) &&
        text::copy(value.etag,    sizeof(value.etag),    header.etag);
}

bool sota::buildCandidate(const char* const csvPath, const char* const version, const char* const etag,
    const uint64_t sourceSize, const ProgressCallback callback, void* const userData
) {
    discardCandidate();

    if (
        csvPath == nullptr      || csvPath[0] == '\0' ||
        etag == nullptr         || etag[0] == '\0'    ||
        sourceSize == 0U        ||
        !uSota::isVersionValid(version)               ||
        static_cast<uint64_t>(storage::fileSize(csvPath)) != sourceSize
    ) { return false; }

    ValidateCsvContext validation {version, 0U, 0U, true};
    if (!storage::readFileLines(csvPath, _validateCsvLine, &validation) ||
        !validation.valid    ||
        validation.line < 3U ||
        validation.records < MINIMUM_RECORDS
    ) { return false; }

    Header header {};
    std::memcpy(header.magic, MAGIC, sizeof(MAGIC));

    header.formatVersion = FORMAT_VERSION;
    header.headerSize    = sizeof(Header);
    header.recordSize    = sizeof(Record);
    header.recordCount   = validation.records;
    header.sourceSize    = sourceSize;

    if (!text::copy(header.version, sizeof(header.version), version) ||
        !text::copy(header.etag,    sizeof(header.etag),    etag)
    ) { return false; }

    if (!storage::beginFileWrite(CANDIDATE_PATH)) { return false; }

    const bool headerWritten = storage::writeFileChunk(
        reinterpret_cast<const uint8_t*>(&header),
        sizeof(header)
    );

    if (!headerWritten) {
        storage::endFileWrite();
        discardCandidate();
        return false;
    }

    WriteCsvContext writer {0U, 0U, validation.records, {}, 0U, callback, userData, true};
    const bool read     = storage::readFileLines(csvPath, _writeCsvLine, &writer);
    const bool complete = read && writer.valid &&
        writer.records == validation.records   &&
        _flushRecords(writer);

    storage::endFileWrite();
    if (!complete) {
        discardCandidate();
        return false;
    }

    Header candidate {};
    if (!_readHeader(CANDIDATE_PATH, candidate)) {
        discardCandidate();
        return false;
    }

    if (callback != nullptr)
        { callback(100U, userData); }
    return true;
}

bool sota::installCandidate() {
    Header candidate {};
    if (!_readHeader(CANDIDATE_PATH, candidate)) { return false; }

    storage::deleteFile(BACKUP_PATH);
    const bool hadInstalled = storage::fileExists(DATABASE_PATH);

    if (hadInstalled && !storage::renameFile(DATABASE_PATH, BACKUP_PATH)) { return false; }
    if (!storage::renameFile(CANDIDATE_PATH, DATABASE_PATH)) {
        if (hadInstalled)
            { storage::renameFile(BACKUP_PATH, DATABASE_PATH); }
        return false;
    }

    Info installed;
    if (!info(installed)) {
        storage::deleteFile(DATABASE_PATH);
        if (hadInstalled)
            { storage::renameFile(BACKUP_PATH, DATABASE_PATH); }
        return false;
    }
    storage::deleteFile(BACKUP_PATH);
    return true;
}

void sota::discardCandidate() {
    storage::deleteFile(CANDIDATE_PATH);
}

bool sota::findNearest(const double latitude, const double longitude,
    uSota::Summit& summit, double& distanceKm, double& bearing
) {
    summit     = uSota::Summit {};
    distanceKm = 0.0;
    bearing    = 0.0;

    Header header {};
    if (!_readHeader(DATABASE_PATH, header)) { return false; }

    SearchContext context {
        0U, 0U,
        header.recordCount,  {}, 0U,
        latitude, longitude, {}, 0.0, false, true
    };
    if (!storage::readFileChunks(DATABASE_PATH, _searchChunk, &context) ||
        !context.valid         ||
        context.received != 0U ||
        context.records != header.recordCount ||
        !context.found
    ) { return false; }

    summit     = context.nearest;
    distanceKm = context.distanceKm;
    bearing    = distance::bearingDegrees(
        latitude,        longitude,
        summit.latitude, summit.longitude
    );

    return true;
}
