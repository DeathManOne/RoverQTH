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
    constexpr size_t CODE_SIZE = 16U;

    /** @brief Maximum storage size of an HTTP entity tag, including its terminator. */
    constexpr size_t ETAG_SIZE = 96U;

    /** @brief Maximum number of retained search results. */
    constexpr size_t RESULT_CAPACITY = 64U;

    /** @brief Final outcome of a synchronous database search. */
    enum class SearchStatus : uint8_t {SUCCESS, CANCELLED, ERROR};

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

    /**
     * @brief Validates an installed database candidate at the supplied path.
     * @param path Path of the database candidate to validate.
     * @return `true` when the candidate is valid, otherwise `false`.
     */
    using CandidateValidator = bool (*)(const char* path);

    /**
     * @brief Reports a percentage during a long database operation.
     * @param progress Current operation progress as a percentage.
     * @param userData Optional caller-defined context.
     */
    using ProgressCallback = void (*)(uint8_t progress, void* userData);

    /**
     * @brief Requests cooperative cancellation of a database search.
     * @param userData Optional caller-defined context.
     * @return `true` when cancellation is requested, otherwise `false`.
     */
    using CancelCallback = bool (*)(void* userData);

    /**
     * @brief Clears every retained result and resets the result count.
     * @param results Collection to reset.
     */
    void clear(SearchResults &results);

    /**
     * @brief Retains a candidate among the nearest fixed-capacity results.
     * @param results Collection updated with the candidate when applicable.
     * @param candidate Candidate to evaluate and retain.
     * @return `true` when the candidate was processed successfully.
     */
    bool retainNearest(SearchResults &results, const SearchResult &candidate);

    /**
     * @brief Builds and retains a candidate from its individual display fields.
     * @param results Collection updated with the candidate when applicable.
     * @param code OTA reference code of the candidate.
     * @param distanceKm Distance to the candidate in kilometres.
     * @param points Number of points awarded by the candidate.
     * @param bonus Number of bonus points awarded by the candidate.
     * @return `true` when the candidate was built and processed successfully.
     */
    bool retainNearest(SearchResults &results, const char* code, double distanceKm, uint8_t points, uint8_t bonus);

    /**
     * @brief Consumes a fixed-size prefix from a streamed byte block.
     * @param data Pointer advanced past the consumed bytes.
     * @param length Remaining number of bytes in the streamed block.
     * @param prefixSize Total number of prefix bytes to skip.
     * @param skipped Number of prefix bytes already skipped, updated on return.
     * @return `true` when the supplied stream state is valid.
     */
    bool skipPrefix(const uint8_t* &data, size_t &length, size_t prefixSize, size_t &skipped);

    /**
     * @brief Copies a streamed byte block into a partially received record.
     * @param data Pointer advanced past the copied bytes.
     * @param length Remaining number of bytes in the streamed block.
     * @param record Destination record buffer.
     * @param recordSize Total size of the destination record in bytes.
     * @param received Number of record bytes already received, updated on return.
     * @return `true` when bytes were copied successfully.
     */
    bool readRecordChunk(const uint8_t* &data, size_t &length, void* record, size_t recordSize, size_t &received);

    /**
     * @brief Validates the terminated code and area fields of an OTA record.
     * @param code Null-terminated OTA reference code.
     * @param codeSize Capacity of the code field in bytes.
     * @param area Null-terminated OTA area name.
     * @param areaSize Capacity of the area field in bytes.
     * @return `true` when both identity fields are valid and properly terminated.
     */
    bool hasValidIdentity(const char* code, size_t codeSize, const char* area, size_t areaSize);

    /**
     * @brief Checks whether an OTA reference code is syntactically usable.
     * @param code Reference code to validate.
     * @return `true` when the code is non-empty and terminated within `CODE_SIZE`.
     */
    bool isValidCode(const char* code);

    /**
     * @brief Checks the consistency of the final state of a streamed search.
     * @param readComplete Whether the underlying streamed read completed.
     * @param valid Whether every processed record remained valid.
     * @param received Number of bytes retained in a partial record.
     * @param records Number of complete records processed.
     * @param expectedRecords Number of records declared by the database.
     * @param found Whether at least one matching record was found.
     * @return `true` when the streamed search completed consistently.
     */
    bool isCompleteSearchRead(bool readComplete, bool valid, size_t received, uint32_t records, uint32_t expectedRecords, bool found);

    /**
     * @brief Appends buffered fixed-size records to the active candidate file.
     * @param buffer Buffer containing the records to append.
     * @param recordSize Size of one record in bytes.
     * @param buffered Number of buffered records, reset to zero after a successful write.
     * @return `true` when the buffer was empty or written successfully.
     */
    bool flushRecords(const void* buffer, size_t recordSize, size_t &buffered);

    /**
     * @brief Computes and reports record-based progress when a callback is supplied.
     * @param callback Optional callback receiving the calculated percentage.
     * @param userData Optional context forwarded unchanged to `callback`.
     * @param records Number of records already processed.
     * @param totalRecords Total number of records to process.
     */
    void reportProgress(ProgressCallback callback, void* userData, uint32_t records, uint32_t totalRecords);

    /**
     * @brief Atomically validates and promotes a candidate database with rollback support.
     * @param descriptor Paths and diagnostic events used by the installation.
     * @param validator Callback used to validate the candidate and installed database.
     * @return `true` when the candidate was installed and validated successfully.
     */
    bool installCandidate(const CandidateInstallDescriptor &descriptor, CandidateValidator validator);

    /**
     * @brief Removes an obsolete candidate database when present.
     * @param candidatePath Path of the candidate database to remove.
     */
    void discardCandidate(const char* const candidatePath);

    /**
     * @brief Restores a backup database and records the supplied recovery event.
     * @param databasePath Destination path of the installed database.
     * @param backupPath Path of the backup database to restore.
     * @param recoveredEvent Diagnostic event recorded after successful recovery.
     * @return `true` when the backup was restored successfully.
     */
    bool recoverBackup(const char* databasePath, const char* backupPath, const char* recoveredEvent);

    /**
     * @brief Reads an exact binary header from a database file.
     * @param path Path of the database file to read.
     * @param header Destination buffer receiving the binary header.
     * @param headerSize Exact number of header bytes to read.
     * @return `true` when the complete header was read successfully.
     */
    bool readHeader(const char* path, void* header, size_t headerSize);

    /**
     * @brief Verifies that a database file size matches its declared record count.
     * @param path Path of the database file to inspect.
     * @param headerSize Size of the binary header in bytes.
     * @param recordSize Size of one binary record in bytes.
     * @param recordCount Number of records declared by the database header.
     * @return `true` when the actual and expected file sizes match.
     */
    bool hasExpectedSize(const char* path, size_t headerSize, size_t recordSize, uint32_t recordCount);

    /**
     * @brief Records detailed diagnostics after a candidate write failure.
     * @param code Diagnostic event code identifying the failure.
     * @param candidatePath Path of the partially written candidate database.
     * @param headerSize Size of the candidate header in bytes.
     * @param recordSize Size of one candidate record in bytes.
     * @param records Number of records already processed.
     * @param totalRecords Total number of records expected.
     * @param buffered Number of records still retained in the write buffer.
     */
    void logCandidateFailure(const char* code, const char* candidatePath, size_t headerSize, size_t recordSize, uint32_t records, uint32_t totalRecords, size_t buffered);
}
