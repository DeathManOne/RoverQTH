/*
 * include/utilities/version.h
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

namespace utilities::version {
    constexpr size_t TEXT_SIZE = 16;
    enum class Comparison : uint8_t {OLDER, EQUAL, NEWER};

    /**
     * Converts a version string to its numeric build representation.
     * @param value Version string.
     * @param build Receives the converted build number.
     * @return true if the conversion succeeded, false otherwise.
     */
    bool toBuild(const char* value, uint32_t& build);

    /**
     * Compares two version strings.
     * @param left   Left version string.
     * @param right  Right version string.
     * @param result Receives the comparison result.
     * @return true if both version strings are valid, false otherwise.
     */
    bool compare(const char* left, const char* right, Comparison& result);
}
