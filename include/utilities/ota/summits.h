/*
 * include/utilities/ota/summits.h
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

#include "database/ota/summits.h"

namespace utilities::ota::summits {
    /**
     * @brief Extracts and validates the list version from the SOTA CSV header.
     * @param line Null-terminated SOTA list-header line to parse.
     * @param version Destination buffer receiving the version in `YYYY-MM-DD` format.
     * @param size Capacity of the destination buffer in bytes.
     * @return `true` when a valid list version was extracted.
     */
    bool parseListVersion(const char* line, char* version, size_t size);

    /**
     * @brief Converts one SOTA CSV record into its binary summit representation.
     * @param line Null-terminated CSV record to parse.
     * @param summit Receives the parsed summit.
     * @return `true` when every required field was parsed successfully.
     */
    bool parseCsvRecord(const char* line, database::ota::summits::Summit &summit);

    /**
     * @brief Checks whether a SOTA list version uses the expected date format.
     * @param version Null-terminated version to validate in `YYYY-MM-DD` format.
     * @return `true` when the version represents a valid supported date.
     */
    bool isVersionValid(const char* version);

    /**
     * @brief Updates the nearest-summit selection with a valid candidate.
     * @param latitude Reference latitude in decimal degrees.
     * @param longitude Reference longitude in decimal degrees.
     * @param candidate Summit candidate to evaluate.
     * @param found Indicates whether a previous nearest summit exists and is updated when selected.
     * @param nearest Receives the candidate when it becomes the nearest summit.
     * @param distanceKm Current nearest distance, updated in kilometres when the candidate is closer.
     * @return `true` when the candidate distance was evaluated successfully.
     */
    bool selectNearest(
        double latitude,
        double longitude,
        const database::ota::summits::Summit &candidate,
        bool &found,
        database::ota::summits::Summit &nearest,
        double &distanceKm
    );
}
