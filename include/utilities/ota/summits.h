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
    /** @brief Extracts and validates the list version from the SOTA CSV header. */
    bool parseListVersion(
        const char* line,
        char* version,
        size_t size
    );

    /** @brief Converts one SOTA CSV record into its binary summit representation. */
    bool parseCsvRecord(
        const char* line,
        database::ota::summits::Summit &summit
    );

    /** @brief Checks whether a SOTA list version uses the expected date format. */
    bool isVersionValid(const char* version);

    /** @brief Updates the nearest-summit selection with a valid candidate. */
    bool selectNearest(
        double latitude,
        double longitude,
        const database::ota::summits::Summit &candidate,
        bool &found,
        database::ota::summits::Summit &nearest,
        double &distanceKm
    );
}
