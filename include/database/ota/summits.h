/*
 * include/database/ota/summits.h
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

#include "database/ota.h"

namespace database::ota::summits {
    /** @brief Binary capacity of a summit reference code. */
    constexpr size_t CODE_SIZE    = database::ota::CODE_SIZE;
    /** @brief Binary capacity of the source entity tag. */
    constexpr size_t ETAG_SIZE    = database::ota::ETAG_SIZE;
    /** @brief Binary capacity of a summit association name. */
    constexpr size_t AREA_SIZE    = 64U;
    /** @brief Binary capacity of the SOTA list version. */
    constexpr size_t VERSION_SIZE = 11U;

    /** @brief Progress callback accepted while building a SOTA candidate. */
    using ProgressCallback = database::ota::ProgressCallback;

    /** @brief Persistent representation of one SOTA summit. */
    struct Summit {
        char code[CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        int16_t altitude = 0;
        uint8_t points   = 0U;
        uint8_t bonus    = 0U;
    };

    /** @brief Metadata stored in the SOTA database header. */
    struct Info {
        uint32_t records    = 0U;
        uint64_t sourceSize = 0U;
        char version[VERSION_SIZE] {};
        char etag[database::ota::ETAG_SIZE] {};
    };

    /** @brief Reads and validates the installed SOTA database metadata. */
    bool info(Info &value);

    /** @brief Converts a downloaded SOTA CSV file into a validated binary candidate. */
    bool buildCandidate(
        const char* csvPath,
        const char* version,
        const char* etag,
        uint64_t sourceSize,
        database::ota::ProgressCallback callback = nullptr,
        void* userData = nullptr
    );

    /** @brief Atomically promotes the prepared SOTA candidate database. */
    bool installCandidate();
    /** @brief Removes any prepared SOTA candidate database. */
    void discardCandidate();

    /** @brief Finds the summit nearest to the supplied coordinates. */
    bool findNearest(
        double latitude,
        double longitude,
        Summit &summit,
        double &distanceKm,
        double &bearing
    );

    /** @brief Finds a summit by its exact reference code. */
    bool findByCode(
        const char* code,
        Summit &summit
    );

    /** @brief Finds the nearest summits whose codes match a normalized prefix. */
    database::ota::SearchStatus findByPrefix(
        double latitude,
        double longitude,
        const char* normalizedPrefix,
        database::ota::SearchResults &results,
        database::ota::CancelCallback cancelCallback = nullptr,
        void* cancelUserData = nullptr
    );

    /** @brief Finds the nearest summits within a geographic radius. */
    database::ota::SearchStatus findNearby(
        double latitude,
        double longitude,
        double radiusKm,
        database::ota::SearchResults &results,
        database::ota::CancelCallback cancelCallback = nullptr,
        void* cancelUserData = nullptr
    );
}
