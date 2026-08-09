/*
 * include/database/sota.h
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
#include <cstdint>

#include "utilities/sota.h"

namespace database::sota {
    /** Information stored in the installed database header. */
    struct Info {
        uint32_t records = 0U;
        char version[utilities::sota::VERSION_SIZE] {};
    };

    /** Callback invoked while a candidate database is being built. */
    using ProgressCallback = void (*)(uint8_t progress, void* userData);

    /** Reads and validates the installed database information. */
    bool info(Info& value);

    /** Builds and validates a binary candidate database from a SOTA CSV file. */
    bool buildCandidate(const char* csvPath, const char* version,
        ProgressCallback callback = nullptr, void* userData = nullptr);

    /** Atomically replaces the installed database with the validated candidate. */
    bool installCandidate();

    /** Deletes the candidate database when it is no longer usable. */
    void discardCandidate();

    /** Finds the nearest stored summit and returns its distance and bearing. */
    bool findNearest(double latitude, double longitude,
        utilities::sota::Summit& summit, double& distanceKm, double& bearing);
}
