/*
 * src/utilities/csv.cpp
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

#include <cstring>

#include "utilities/csv.h"

namespace csv = utilities::csv;

bool csv::copyField(const char* const source, const size_t length, char* const destination, const size_t destinationSize) {
    if (source == nullptr || destination == nullptr || destinationSize == 0U || length >= destinationSize)
        { return false; }

    std::memcpy(destination, source, length);
    destination[length] = '\0';

    return true;
}

bool csv::readField(const char*& cursor, char* const destination, const size_t destinationSize, bool& delimiter) {
    delimiter = false;

    if (cursor == nullptr || destination == nullptr || destinationSize == 0U)
        { return false; }

    size_t length = 0U;
    bool quoted   = false;

    if (*cursor == '"') {
        quoted = true;
        ++cursor;
    }

    while (*cursor != '\0') {
        if (quoted) {
            if (*cursor == '"') {
                if (cursor[1] == '"') {
                    if (length + 1U >= destinationSize)
                        { return false; }
                    destination[length++] = '"';
                    cursor += 2;
                    continue;
                }

                ++cursor;
                quoted = false;

                while (*cursor == ' ' || *cursor == '\t')
                    { ++cursor; }
                if (*cursor != ',' && *cursor != '\0')
                    { return false; }
                break;
            }
        } else {
            if (*cursor == ',') { break; }
            if (*cursor == '"') { return false; }
        }

        if (length + 1U >= destinationSize)
            {  return false; }
        destination[length++] = *cursor++;
    }

    if (quoted)
        { return false; }
    destination[length] = '\0';

    if (*cursor == ',') {
        delimiter = true;
        ++cursor;
    }

    return true;
}
