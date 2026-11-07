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
     * @param year Four-digit calendar year, starting from 1970.
     * @param month Calendar month from 1 to 12.
     * @param day Calendar day valid for the supplied month and year.
     * @param hour Hour from 0 to 23.
     * @param minute Minute from 0 to 59.
     * @param second Second from 0 to 59.
     * @return Seconds elapsed since the Unix epoch, or zero for an invalid date.
     */
    uint32_t toEpochUTC(int year, int month, int day, int hour, int minute, int second);

    /**
     * @brief Formats a Unix timestamp as an ISO 8601 UTC date and time.
     * @param utcEpoch Unix timestamp expressed in UTC seconds.
     * @param buffer Destination buffer for the null-terminated formatted value.
     * @param size Total destination capacity, including the null terminator.
     * @return `true` when the timestamp is valid and the complete value was written,
     * otherwise `false`.
     */
    bool formatISO8601(uint32_t utcEpoch, char* buffer, size_t size);

    /**
     * @brief Formats a Unix timestamp as a calendar date.
     * An invalid timestamp produces the placeholder `---- -- --`.
     * @param utcEpoch Unix timestamp expressed in UTC seconds.
     * @param buffer Destination buffer for the null-terminated formatted value.
     * @param size Total destination capacity, including the null terminator.
     * @return `true` when the complete date or placeholder was written, otherwise `false`.
     */
    bool formatDate(uint32_t utcEpoch, char* buffer, size_t size);

    /**
     * @brief Formats explicit time components.
     * Invalid components produce a placeholder using the requested format.
     * @param hour Hour from 0 to 23.
     * @param minute Minute from 0 to 59.
     * @param second Second from 0 to 59.
     * @param valid Whether the supplied time is valid.
     * @param buffer Destination buffer for the null-terminated formatted value.
     * @param size Total destination capacity, including the null terminator.
     * @param withSecond Whether seconds must be included.
     * @return `true` when the complete time or placeholder was written, otherwise `false`.
     */
    bool formatTime(uint8_t hour, uint8_t minute, uint8_t second, bool valid, char* buffer, size_t size, bool withSecond = true);

    /**
     * @brief Formats the time portion of a Unix timestamp.
     * An invalid timestamp produces a placeholder using the requested format.
     * @param utcEpoch Unix timestamp expressed in UTC seconds.
     * @param buffer Destination buffer for the null-terminated formatted value.
     * @param size Total destination capacity, including the null terminator.
     * @param withSecond Whether seconds must be included.
     * @return `true` when the complete time or placeholder was written, otherwise `false`.
     */
    bool formatTime(uint32_t utcEpoch, char* buffer, size_t size, bool withSecond = true);
}
