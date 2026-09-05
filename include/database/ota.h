/*
 * include/database/ota.h
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

#pragma once

#include <cstddef>
#include <cstdint>

namespace database::ota {
    /** @brief Maximum storage size of an OTA reference code, including its terminator. */
    constexpr size_t CODE_SIZE       = 16U;
    /** @brief Maximum storage size of an HTTP entity tag, including its terminator. */
    constexpr size_t ETAG_SIZE       = 96U;
    /** @brief Maximum number of retained search results. */
    constexpr size_t RESULT_CAPACITY = 64U;

    /** @brief Final outcome of a synchronous database search. */
    enum class SearchStatus : uint8_t {
        SUCCESS,
        CANCELLED,
        ERROR
    };

    /** @brief Common display data retained for one OTA search result. */
    struct SearchResult {
        char code[CODE_SIZE] {};
        double distanceKm = 0.0;
        uint8_t points    = 0U;
        uint8_t bonus     = 0U;
    };

    /** @brief Fixed-capacity collection of OTA search results. */
    struct SearchResults {
        SearchResult items[RESULT_CAPACITY] {};
        size_t count = 0U;
    };

    /** @brief Paths and diagnostic events required for an atomic database installation. */
    struct CandidateInstallDescriptor {
        const char* databasePath;
        const char* candidatePath;
        const char* backupPath;
        const char* candidateInvalid;
        const char* backupDeleteFailed;
        const char* backupCreateFailed;
        const char* candidateInstallFailed;
        const char* rollbackFailed;
        const char* installedInvalid;
    };

    /** @brief Validates an installed database candidate at the supplied path. */
    using CandidateValidator = bool (*)(const char* path);
    /** @brief Reports a percentage during a long database operation. */
    using ProgressCallback   = void (*)(uint8_t progress, void* userData);
    /** @brief Requests cooperative cancellation of a database search. */
    using CancelCallback     = bool (*)(void* userData);

    /** @brief Clears every retained result and resets the result count. */
    void clear(SearchResults &results);
    /** @brief Retains a candidate among the nearest fixed-capacity results. */
    bool retainNearest(
        SearchResults &results,
        const SearchResult &candidate
    );
    /** @brief Builds and retains a candidate from its individual display fields. */
    bool retainNearest(
        SearchResults &results,
        const char* code,
        double distanceKm,
        uint8_t points,
        uint8_t bonus
    );
    /** @brief Consumes a fixed-size prefix from a streamed byte block. */
    bool skipPrefix(
        const uint8_t*& data,
        size_t& length,
        size_t prefixSize,
        size_t& skipped
    );
    /** @brief Copies a streamed byte block into a partially received record. */
    bool readRecordChunk(
        const uint8_t*& data,
        size_t& length,
        void* record,
        size_t recordSize,
        size_t& received
    );
    /** @brief Validates the terminated code and area fields of an OTA record. */
    bool hasValidIdentity(
        const char* code,
        size_t codeSize,
        const char* area,
        size_t areaSize
    );
    /** @brief Checks whether an OTA reference code is syntactically usable. */
    bool isValidCode(const char* code);
    /** @brief Checks the consistency of the final state of a streamed search. */
    bool isCompleteSearchRead(bool readComplete, bool valid,
        size_t received,          uint32_t records,
        uint32_t expectedRecords, bool found
    );
    /** @brief Appends buffered fixed-size records to the active candidate file. */
    bool flushRecords(
        const void* buffer,
        size_t recordSize,
        size_t& buffered
    );
    /** @brief Computes and reports record-based progress when a callback is supplied. */
    void reportProgress(
        ProgressCallback callback,
        void* userData,
        uint32_t records,
        uint32_t totalRecords
    );
    /** @brief Atomically validates and promotes a candidate database with rollback support. */
    bool installCandidate(
        const CandidateInstallDescriptor& descriptor,
        CandidateValidator validator
    );
    /** @brief Removes an obsolete candidate database when present. */
    void discardCandidate(const char* const candidatePath);
    /** @brief Restores a backup database and records the supplied recovery event. */
    bool recoverBackup(
        const char* databasePath,
        const char* backupPath,
        const char* recoveredEvent
    );
    /** @brief Reads an exact binary header from a database file. */
    bool readHeader(
        const char* path,
        void* header,
        size_t headerSize
    );
    /** @brief Verifies that a database file size matches its declared record count. */
    bool hasExpectedSize(
        const char* path,
        size_t headerSize,
        size_t recordSize,
        uint32_t recordCount
    );
    /** @brief Records detailed diagnostics after a candidate write failure. */
    void logCandidateFailure(
        const char* code,
        const char* candidatePath,
        size_t headerSize,
        size_t recordSize,
        uint32_t records,
        uint32_t totalRecords,
        size_t buffered
    );
}
