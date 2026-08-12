/*
 * include/utilities/sota.h
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

namespace utilities::sota {
    /** Maximum sizes of the null-terminated SOTA fields. */
    constexpr size_t CODE_SIZE    = 16U;
    constexpr size_t AREA_SIZE    = 64U;
    constexpr size_t VERSION_SIZE = 11U;

    /** Application representation of a SOTA summit. */
    struct Summit {
        char code[CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        int16_t altitude = 0;
        uint8_t points   = 0;
        uint8_t bonus    = 0;
    };

    /** Extracts and normalizes the list date from the first SOTA CSV line. */
    bool parseListVersion(const char* line, char* version, size_t size);

    /** Parses the required fields from one SOTA CSV record. */
    bool parseCsvRecord(const char* line, Summit& summit);

    /** Validates a normalized SOTA version in YYYY-MM-DD format. */
    bool isVersionValid(const char* version);

    /** Indicates whether a candidate version is newer than the installed version. */
    bool isVersionNewer(const char* candidate, const char* installed);

    /** Updates a nearest-summit result when the candidate is geographically closer. */
    bool selectNearest(double latitude, double longitude, const Summit& candidate, bool& found, Summit& nearest, double& distanceKm);
}
