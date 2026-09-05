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

#include <cstring>
#include "utilities/text.h"

namespace text = utilities::text;

bool text::copy(char* const destination, const size_t destinationSize, const char* const source) {
    if (destination == nullptr || destinationSize == 0U)
        { return false; }
    destination[0] = '\0';

    if (source == nullptr)
        { return false; }
    size_t copyIndex = 0U;

    while (copyIndex + 1U < destinationSize && source[copyIndex] != '\0') {
        destination[copyIndex] = source[copyIndex];
        ++copyIndex;
    }

    destination[copyIndex] = '\0';
    return source[copyIndex] == '\0';
}

bool text::isTerminated(const char* const value, const size_t capacity) {
    return
        value != nullptr &&
        capacity > 0U    &&
        std::memchr(value, '\0', capacity) != nullptr;
}

bool text::startsWith(const char* const value, const char* const prefix) {
    if (value == nullptr || prefix == nullptr) { return false; }

    size_t prefixIndex = 0U;
    while (prefix[prefixIndex] != '\0') {
        if (value[prefixIndex] == '\0' || value[prefixIndex] != prefix[prefixIndex])
            { return false; }
        ++prefixIndex;
    }
    return true;
}

bool text::equals(const char* const left, const char* const right) {
    if (left == right)                       { return true; }
    if (left == nullptr || right == nullptr) { return false; }

    size_t characterIndex = 0U;
    while (left[characterIndex] != '\0' && right[characterIndex] != '\0') {
        if (left[characterIndex] != right[characterIndex])
            { return false; }
        ++characterIndex;
    }
    return left[characterIndex] == '\0' && right[characterIndex] == '\0';
}
