/*
 * src/utilities/text.cpp
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

#include "utilities/text.h"

namespace text = utilities::text;

bool text::copy(char* const destination, const size_t destinationSize, const char* const source) {
    if (destination == nullptr || destinationSize == 0U)
        { return false; }
    destination[0] = '\0';

    if (source == nullptr)
        { return false; }
    size_t index = 0U;

    while (index + 1U < destinationSize && source[index] != '\0') {
        destination[index] = source[index];
        ++index;
    }

    destination[index] = '\0';
    return source[index] == '\0';
}

bool text::equals(const char* const left, const char* const right) {
    if (left == right)                       { return true; }
    if (left == nullptr || right == nullptr) { return false; }

    size_t index = 0U;
    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index])
            { return false; }
        ++index;
    }
    return left[index] == '\0' && right[index] == '\0';
}
