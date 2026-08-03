/*
 * include/utilities/text.h
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

namespace utilities::text {
    /**
     * Copies a null-terminated string into a fixed-size buffer.
     *
     * The destination is always null-terminated when it is valid.
     * If the source does not fit, the copied value is truncated and
     * the function returns false. A null source is rejected and clears
     * the destination.
     *
     * @param destination Destination buffer.
     * @param destinationSize Total destination capacity.
     * @param source Null-terminated source string.
     *
     * @return True if the complete source was copied, otherwise false.
     */
    bool copy(char* destination, size_t destinationSize, const char* source);

    /**
     * Compares two null-terminated strings.
     *
     * Two identical pointers, including two null pointers, are considered
     * equal. A single null pointer is never equal to a valid string.
     *
     * @param left First string.
     * @param right Second string.
     *
     * @return True if both strings are equal, otherwise false.
     */
    bool equals(const char* left, const char* right);
}
