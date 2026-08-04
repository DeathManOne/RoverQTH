/*
 * include/utilities/clock.h
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

namespace utilities::clock {
    /**
     * Converts a UTC calendar date and time to a Unix epoch.
     * The date must be valid and within the range supported by
     * uint32_t Unix timestamps. Invalid values return zero.
     * @param year   Full year, starting from 1970.
     * @param month  Month from 1 to 12.
     * @param day    Day valid for the selected month and year.
     * @param hour   Hour from 0 to 23.
     * @param minute Minute from 0 to 59.
     * @param second Second from 0 to 59.
     * @return Unix epoch in seconds, or zero on failure.
     */
    uint32_t toEpochUTC(
        int year, int month,  int day,
        int hour, int minute, int second
    );

    /**
     * Formats a Unix epoch as an ISO 8601 UTC timestamp.
     * The resulting format is YYYY-MM-DDTHH:MM:SSZ.
     * The destination buffer is cleared if the operation fails.
     * @param utcEpoch Unix epoch in seconds.
     * @param buffer   Destination buffer.
     * @param size     Total destination capacity.
     * @return True on success, otherwise false.
     */
    bool formatISO8601(uint32_t utcEpoch, char* buffer, size_t size);

    /**
     * Synchronizes the monotonic UTC clock.
     * A zero or older epoch is ignored. An identical or newer
     * epoch refreshes the internal millis() reference.
     * @param utcEpoch UTC Unix epoch in seconds.
     */
    void sync(uint32_t utcEpoch);

    /**
     * Indicates whether the clock has received a valid UTC synchronization.
     * @return True when synchronized, otherwise false.
     */
    bool isSynced();

    /**
     * Returns the current monotonic UTC epoch.
     * The value advances from the most recent synchronization using
     * millis(). It never decreases and saturates at UINT32_MAX.
     * @return Current UTC Unix epoch, or zero if not synchronized.
     */
    uint32_t now();

    /**
     * Formats an hour, minute and second for display.
     * Invalid values produce a placeholder string. The output can
     * include or omit seconds.
     * @param hour       Hour from 0 to 23.
     * @param minute     Minute from 0 to 59.
     * @param second     Second from 0 to 59.
     * @param valid      Whether the provided time is valid.
     * @param buffer     Destination buffer.
     * @param size       Total destination capacity.
     * @param withSecond True to include seconds.
     * @return True if a complete string was produced, otherwise false.
     */
    bool formatTime(
        uint8_t hour, uint8_t minute, uint8_t second,
        bool valid,   char* buffer,   size_t size,
        bool withSecond = true
    );

    /**
     * Formats the current UTC date for display.
     * The resulting format is YYYY MM DD. If the clock is not
     * synchronized, a placeholder string is produced.
     * @param buffer Destination buffer.
     * @param size   Total destination capacity.
     * @return True if a complete string was produced, otherwise false.
     */
    bool getDate(char* buffer, size_t size);

    /**
     * Formats the current UTC time for display.
     * If the clock is not synchronized, a placeholder string is
     * produced. Seconds can optionally be omitted.
     * @param buffer     Destination buffer.
     * @param size       Total destination capacity.
     * @param withSecond True to include seconds.
     * @return True if a complete string was produced, otherwise false.
     */
    bool getTime(char* buffer, size_t size, bool withSecond = true);

    /**
     * Clears the current UTC synchronization.
     * After reset(), isSynced() returns false and now() returns zero
     * until a new valid synchronization is received.
     */
    void reset();
}
