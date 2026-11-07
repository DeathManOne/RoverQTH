/*
 * src/database/ota.cpp
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

#include "database/ota.h"
#include "services/storage.h"

namespace ota     = database::ota;
namespace storage = services::storage;

namespace {
    struct HeaderReadContext {
        uint8_t* header;
        size_t headerSize;
        size_t received;
    };

    bool _readHeaderChunk(const uint8_t* data, size_t length, void* userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        HeaderReadContext* const context = static_cast<HeaderReadContext*>(userData);
        if (context->header == nullptr || context->headerSize == 0U  || context->received >= context->headerSize)
            { return false; }

        size_t copyLength =  context->headerSize - context->received;
        if (copyLength > length) { copyLength = length; }

        std::memcpy(context->header + context->received, data, copyLength);
        context->received += copyLength;
        return context->received < context->headerSize;
    }

    bool _validText(const char* const value) {
        return value != nullptr && value[0] != '\0';
    }
}

void ota::clear(SearchResults &results) {
    results = {};
}

bool ota::retainNearest(SearchResults &results, const SearchResult &candidate) {
    if (results.count > RESULT_CAPACITY) { return false; }
    if (!isValidCode(candidate.code) || !std::isfinite(candidate.distanceKm) || candidate.distanceKm < 0.0)
        { return false; }

    size_t insertionIndex = 0U;

    while (insertionIndex < results.count && results.items[insertionIndex].distanceKm <= candidate.distanceKm)
        { ++insertionIndex; }

    if (
        results.count  == RESULT_CAPACITY &&
        insertionIndex == RESULT_CAPACITY
    ) { return true; }

    const size_t lastIndex = results.count < RESULT_CAPACITY ? results.count : RESULT_CAPACITY - 1U;

    for (size_t shiftIndex = lastIndex; shiftIndex > insertionIndex; --shiftIndex)
        { results.items[shiftIndex] = results.items[shiftIndex - 1U]; }
    results.items[insertionIndex] = candidate;

    if (results.count < RESULT_CAPACITY) { ++results.count; }
    return true;
}

bool ota::retainNearest(SearchResults& results, const char* const code, const double distanceKm, const uint8_t points, const uint8_t bonus) {
    if (!isValidCode(code)) { return false; }

    SearchResult candidate {};
    size_t codeIndex = 0U;

    do { candidate.code[codeIndex] = code[codeIndex]; }
    while (code[codeIndex++] != '\0');

    candidate.distanceKm = distanceKm;
    candidate.points     = points;
    candidate.bonus      = bonus;

    return retainNearest(results, candidate);
}

bool ota::skipPrefix(const uint8_t*& data, size_t& length, const size_t prefixSize, size_t& skipped) {
    if (data == nullptr || skipped > prefixSize) { return false; }
    if (skipped == prefixSize)                   { return true; }

    size_t skip = prefixSize - skipped;
    if (skip > length) { skip = length; }

    skipped += skip;
    data    += skip;
    length  -= skip;

    return true;
}

bool ota::readRecordChunk(const uint8_t*& data, size_t& length, void* const record, const size_t recordSize, size_t& received) {
    if (
        data       == nullptr    ||
        record     == nullptr    ||
        recordSize == 0U         ||
        received   >= recordSize ||
        length     == 0U
    ) {  return false; }

    size_t copyLength = recordSize - received;
    if (copyLength > length) { copyLength = length; }

    std::memcpy(static_cast<uint8_t*>(record) + received, data, copyLength);

    received += copyLength;
    data     += copyLength;
    length   -= copyLength;

    return true;
}

bool ota::hasValidIdentity(const char* const code, const size_t codeSize, const char* const area, const size_t areaSize) {
    if (
        codeSize != CODE_SIZE ||
        !isValidCode(code)    ||
        area     == nullptr   ||
        areaSize == 0U        ||
        area[0]  == '\0'
    ) { return false; }

    for (size_t areaIndex = 0U; areaIndex < areaSize; ++areaIndex) {
        if (area[areaIndex] == '\0') { return true; }
    }

    return false;
}

bool ota::isValidCode(const char* const code) {
    if (code == nullptr || code[0] == '\0') { return false; }

    for (size_t codeIndex = 0U; codeIndex < CODE_SIZE; ++codeIndex) {
        if (code[codeIndex] == '\0') { return true; }
    }

    return false;
}

bool ota::isCompleteSearchRead(const bool readComplete, const bool valid,
    const size_t received,          const uint32_t records,
    const uint32_t expectedRecords, const bool found
) {
    return
        readComplete                &&
        valid                       &&
        received == 0U              &&
        records  == expectedRecords &&
        found;
}

bool ota::flushRecords(const void* const buffer, const size_t recordSize, size_t& buffered) {
    if (buffered == 0U) { return true; }
    if (buffer == nullptr || recordSize == 0U || buffered > SIZE_MAX / recordSize) { return false; }

    const bool written = storage::writeFileChunk(static_cast<const uint8_t*>(buffer), buffered * recordSize);
    if (written) { buffered = 0U; }
    return written;
}

void ota::reportProgress(const ProgressCallback callback, void* const userData, const uint32_t records, const uint32_t totalRecords) {
    if (callback == nullptr || totalRecords == 0U || (records & 0x7FU) != 0U) { return; }

    const uint64_t calculated = (static_cast<uint64_t>(records) * 100ULL) / totalRecords;
    callback(calculated > 100ULL ? 100U : static_cast<uint8_t>(calculated), userData);
}

bool ota::installCandidate(const CandidateInstallDescriptor& descriptor, const CandidateValidator validator) {
    if (
        !_validText(descriptor.databasePath)           ||
        !_validText(descriptor.candidatePath)          ||
        !_validText(descriptor.backupPath)             ||
        !_validText(descriptor.candidateInvalid)       ||
        !_validText(descriptor.backupDeleteFailed)     ||
        !_validText(descriptor.backupCreateFailed)     ||
        !_validText(descriptor.candidateInstallFailed) ||
        !_validText(descriptor.rollbackFailed)         ||
        !_validText(descriptor.installedInvalid)       ||
        validator == nullptr
    ) { return false; }

    if (!validator(descriptor.candidatePath)) {
        storage::appendErrorRecord(descriptor.candidateInvalid);
        return false;
    }

    if (storage::fileExists(descriptor.backupPath) && !storage::deleteFile(descriptor.backupPath)) {
        storage::appendErrorRecord(descriptor.backupDeleteFailed);
        return false;
    }

    const bool hadInstalled = storage::fileExists(descriptor.databasePath);
    if (hadInstalled && !storage::renameFile(descriptor.databasePath, descriptor.backupPath)) {
        storage::appendErrorRecord(descriptor.backupCreateFailed);
        return false;
    }

    if (!storage::renameFile(descriptor.candidatePath, descriptor.databasePath)) {
        storage::appendErrorRecord(descriptor.candidateInstallFailed);

        if (hadInstalled && !storage::renameFile(descriptor.backupPath, descriptor.databasePath))
            { storage::appendErrorRecord(descriptor.rollbackFailed); }
        return false;
    }

    if (!validator(descriptor.databasePath)) {
        storage::appendErrorRecord(descriptor.installedInvalid);

        bool rollbackValid = storage::deleteFile(descriptor.databasePath);
        if (rollbackValid && hadInstalled)
            { rollbackValid = storage::renameFile(descriptor.backupPath, descriptor.databasePath); }
        if (!rollbackValid)
            { storage::appendErrorRecord(descriptor.rollbackFailed); }
        return false;
    }

    if (hadInstalled && !storage::deleteFile(descriptor.backupPath))
        { storage::appendErrorRecord(descriptor.backupDeleteFailed); }
    return true;
}

void ota::discardCandidate(const char* const candidatePath) {
    if (!_validText(candidatePath)) {
        storage::appendErrorRecord("DATABASE_CANDIDATE_ARGUMENT_INVALID");
        return;
    }

    if (storage::fileExists(candidatePath) && !storage::deleteFile(candidatePath))
        { storage::appendErrorRecord("DATABASE_CANDIDATE_DELETE_FAILED"); }
}

bool ota::recoverBackup(const char* const databasePath, const char* const backupPath, const char* const recoveredEvent) {
    if (
        !_validText(databasePath)   ||
        !_validText(backupPath)     ||
        !_validText(recoveredEvent) ||
        storage::fileExists(databasePath)
    ) { return false; }

    if (!storage::renameFile(backupPath, databasePath)) { return false; }

    storage::appendErrorRecord(recoveredEvent);
    return true;
}

bool ota::readHeader(const char* const path, void* const header, const size_t headerSize) {
    if (
        path       == nullptr ||
        path[0]    == '\0'    ||
        header     == nullptr ||
        headerSize == 0U
    ) { return false; }

    std::memset(header, 0, headerSize);

    HeaderReadContext context {
        static_cast<uint8_t*>(header), headerSize, 0U
    };

    return storage::readFileChunks(path, _readHeaderChunk, &context) && context.received == headerSize;
}

bool ota::hasExpectedSize(const char* const path, const size_t headerSize, const size_t recordSize, const uint32_t recordCount) {
    if (!_validText(path) || headerSize == 0U || recordSize == 0U) { return false; }

    const uint64_t expectedSize =
        static_cast<uint64_t>(headerSize)  +
        static_cast<uint64_t>(recordCount) *
        static_cast<uint64_t>(recordSize);
    return static_cast<uint64_t>(storage::fileSize(path)) == expectedSize;
}

void ota::logCandidateFailure(const char* const code, const char* const candidatePath, const size_t headerSize, const size_t recordSize, const uint32_t records, const uint32_t totalRecords, const size_t buffered) {
    if (code == nullptr || code[0] == '\0' || candidatePath == nullptr || candidatePath[0] == '\0') { return; }

    uint8_t progress = 60U;
    if (totalRecords > 0U) {
        const uint64_t calculated = 60ULL + (static_cast<uint64_t>(records) * 35ULL) / totalRecords;
        progress                  = calculated > 95ULL ? 95U : static_cast<uint8_t>(calculated);
    }

    const size_t candidateSize  = storage::fileSize(candidatePath);
    const uint64_t expectedSize =
        static_cast<uint64_t>(headerSize)   +
        static_cast<uint64_t>(totalRecords) *
        static_cast<uint64_t>(recordSize);

    const size_t freeHeap     = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    const size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    char message[192];

    const int written = std::snprintf(
        message, sizeof(message),
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

    storage::appendErrorRecord(written > 0 && static_cast<size_t>(written) < sizeof(message) ? message : code);
}
