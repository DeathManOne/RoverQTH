/*
 * include/database/ota/parks.h
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

namespace database::ota::parks {
    /** @brief Binary capacity of a park reference code. */
    constexpr size_t CODE_SIZE = database::ota::CODE_SIZE;

    /** @brief Binary capacity of the source entity tag. */
    constexpr size_t ETAG_SIZE = database::ota::ETAG_SIZE;

    /** @brief Binary capacity of a park area name. */
    constexpr size_t AREA_SIZE = 96U;

    /** @brief Progress callback accepted while building a POTA candidate. */
    using ProgressCallback = database::ota::ProgressCallback;

    /** @brief Persistent representation of one POTA park. */
    struct Park {
        char code[CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
    };

    /** @brief Metadata stored in the POTA database header. */
    struct Info {
        uint32_t records    = 0U;
        uint64_t sourceSize = 0U;
        char etag[database::ota::ETAG_SIZE] {};
    };

    /**
     * @brief Reads and validates the installed POTA database metadata.
     * @param value Receives the validated database metadata.
     * @return `true` when the installed database and its metadata are valid.
     */
    bool info(Info &value);

    /**
     * @brief Converts a downloaded POTA CSV file into a validated binary candidate.
     * @param csvPath Path to the downloaded source CSV file.
     * @param etag Entity tag associated with the downloaded source.
     * @param sourceSize Size of the downloaded source in bytes.
     * @param callback Optional callback receiving build progress updates.
     * @param userData Optional context forwarded unchanged to `callback`.
     * @return `true` when the candidate was built and validated successfully.
     */
    bool buildCandidate(const char* csvPath, const char* etag, uint64_t sourceSize, database::ota::ProgressCallback callback = nullptr, void* userData = nullptr);

    /**
     * @brief Atomically promotes the prepared POTA candidate database.
     * @return `true` when the candidate was installed and validated successfully.
     */
    bool installCandidate();

    /** @brief Removes any prepared POTA candidate database. */
    void discardCandidate();

    /**
     * @brief Finds the park nearest to the supplied coordinates.
     * @param latitude Reference latitude in decimal degrees.
     * @param longitude Reference longitude in decimal degrees.
     * @param park Receives the nearest park.
     * @param distanceKm Receives the distance to the park in kilometres.
     * @param bearing Receives the initial bearing to the park in degrees.
     * @return `true` when a nearest park was found.
     */
    bool findNearest(double latitude, double longitude, Park &park, double &distanceKm, double &bearing);

    /**
     * @brief Finds a park by its exact reference code.
     * @param code Exact POTA reference code to find.
     * @param park Receives the matching park.
     * @return `true` when a matching park was found.
     */
    bool findByCode(const char* code, Park &park);

    /**
     * @brief Finds the nearest parks whose codes match a normalized prefix.
     * @param latitude Reference latitude in decimal degrees.
     * @param longitude Reference longitude in decimal degrees.
     * @param normalizedPrefix Normalized park-code prefix to match.
     * @param results Receives the retained matching parks.
     * @param cancelCallback Optional callback requesting cooperative cancellation.
     * @param cancelUserData Optional context forwarded unchanged to `cancelCallback`.
     * @return Status describing completion, cancellation, or failure.
     */
    database::ota::SearchStatus findByPrefix(double latitude, double longitude, const char* normalizedPrefix, database::ota::SearchResults &results, database::ota::CancelCallback cancelCallback = nullptr, void* cancelUserData = nullptr);

    /**
     * @brief Finds the nearest parks within a geographic radius.
     * @param latitude Reference latitude in decimal degrees.
     * @param longitude Reference longitude in decimal degrees.
     * @param radiusKm Maximum search radius in kilometres.
     * @param results Receives the retained nearby parks.
     * @param cancelCallback Optional callback requesting cooperative cancellation.
     * @param cancelUserData Optional context forwarded unchanged to `cancelCallback`.
     * @return Status describing completion, cancellation, or failure.
     */
    database::ota::SearchStatus findNearby(double latitude, double longitude, double radiusKm, database::ota::SearchResults &results, database::ota::CancelCallback cancelCallback = nullptr, void* cancelUserData = nullptr);
}
