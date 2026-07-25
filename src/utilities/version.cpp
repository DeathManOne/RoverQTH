/*
 * src/utilities/version.cpp
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

#include "utilities/version.h"

namespace version = utilities::version;

bool version::toBuild(const char* const value, uint32_t& build) {
    build = 0;
    if (value == nullptr || value[0] == '\0') { return false; }

    uint16_t parts[3] {0, 0, 0};
    uint8_t partIndex = 0;
    uint8_t digits    = 0;

    for (size_t index = 0; value[index] != '\0'; ++index) {
        const char character = value[index];
        if (character == '.') {
            if (digits == 0 || partIndex >= 2)
                { return false; }
            ++partIndex;
            digits = 0;
            continue;
        }
        if (character < '0' || character > '9' || digits >= 3)
            { return false; }
        parts[partIndex] = static_cast<uint16_t>(
            parts[partIndex] * 10U +
            static_cast<uint16_t>(character - '0')
        );
        ++digits;
    }

    if (partIndex != 2 || digits == 0) { return false; }
    build =
        static_cast<uint32_t>(parts[0]) * 1000000UL +
        static_cast<uint32_t>(parts[1]) * 1000UL    +
        static_cast<uint32_t>(parts[2]);
    return true;
}

bool version::compare(const char* const left, const char* const right, Comparison& result) {
    uint32_t leftBuild  = 0;
    uint32_t rightBuild = 0;

    if (!toBuild(left, leftBuild) || !toBuild(right, rightBuild)) { return false; }

    if (leftBuild < rightBuild)      { result = Comparison::OLDER; }
    else if (leftBuild > rightBuild) { result = Comparison::NEWER; }
    else                             { result = Comparison::EQUAL; }

    return true;
}