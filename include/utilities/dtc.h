/*
 * include/utilities/dtc.h
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

namespace utilities::dtc {
    /**
     * @brief Converts UTC calendar components to a Unix timestamp.
     * @return Seconds elapsed since the Unix epoch, or zero for an invalid date.
     */
    uint32_t toEpochUTC(int year, int month, int day,
        int hour, int minute, int second
    );

    /** @brief Formats a Unix timestamp as an ISO 8601 UTC date and time. */
    bool formatISO8601(uint32_t utcEpoch, char* buffer, size_t size);

    /** @brief Formats a Unix timestamp as a calendar date. */
    bool formatDate(uint32_t utcEpoch, char* buffer, size_t size);

    /**
     * @brief Formats explicit time components.
     * @param valid Whether the supplied time is valid.
     * @param withSecond Whether seconds must be included.
     * @return true when the formatted value fits in the destination buffer.
     */
    bool formatTime(uint8_t hour, uint8_t minute, uint8_t second,
        bool valid, char* buffer, size_t size, bool withSecond = true
    );

    /** @brief Formats the time portion of a Unix timestamp. */
    bool formatTime(uint32_t utcEpoch, char* buffer, size_t size, bool withSecond = true);
}
