/*
 * include/services/dtc.h
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

namespace services::dtc {
    /**
     * @brief Synchronizes the internal UTC clock and the ESP32 system clock.
     * A zero or older epoch is ignored. An identical or newer epoch refreshes
     * the internal millis() reference and updates the system clock used by TLS.
     * @param utcEpoch UTC epoch in seconds.
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
