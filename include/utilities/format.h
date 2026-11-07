/*
 * include/utilities/format.h
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

namespace utilities::format {
    /**
     * Formats a duration using a compact representation.
     * @param seconds Duration in seconds.
     * @param buffer  Destination buffer.
     * @param size    Size of the destination buffer in bytes.
     * @return `true` when the formatted duration was successfully written, otherwise `false`.
     */
    bool durationCompact(uint32_t seconds, char* buffer, size_t size);

    /**
     * Formats a speed.
     * @param kilometersPerHour Speed in kilometres per hour.
     * @param imperial          `true` to use imperial units, `false` for metric units.
     * @param buffer            Destination buffer.
     * @param size              Size of the destination buffer in bytes.
     * @return `true` when the formatted speed was successfully written, otherwise `false`.
     */
    bool speed(double kilometersPerHour, bool imperial, char* buffer, size_t size);

    /**
     * Formats an altitude.
     * @param meters   Altitude in metres.
     * @param imperial `true` to use imperial units, `false` for metric units.
     * @param buffer   Destination buffer.
     * @param size     Size of the destination buffer in bytes.
     * @return `true` when the formatted altitude was successfully written, otherwise `false`.
     */
    bool altitude(double meters, bool imperial, char* buffer, size_t size);

    /**
     * Formats a distance.
     * @param kilometers Distance in kilometres.
     * @param imperial   `true` to use imperial units, `false` for metric units.
     * @param buffer     Destination buffer.
     * @param size       Size of the destination buffer in bytes.
     * @return `true` when the formatted distance was successfully written, otherwise `false`.
     */
    bool distance(double kilometers, bool imperial, char* buffer, size_t size);

    /**
     * Formats a bearing.
     * @param degrees Bearing in degrees.
     * @param buffer  Destination buffer.
     * @param size    Size of the destination buffer in bytes.
     * @return `true` when the formatted bearing was successfully written, otherwise `false`.
     */
    bool bearing(double degrees, char* buffer, size_t size);

    /**
     * Formats a heading.
     * @param degrees Heading in degrees.
     * @param buffer  Destination buffer.
     * @param size    Size of the destination buffer in bytes.
     * @return `true` when the formatted heading was successfully written, otherwise `false`.
     */
    bool heading(double degrees, char* buffer, size_t size);

    /**
     * Formats a percentage.
     * @param value  Percentage value.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return `true` when the formatted percentage was successfully written, otherwise `false`.
     */
    bool percentage(uint64_t value, char* buffer, size_t size);

    /**
     * Formats a voltage.
     * @param volts  Voltage in volts.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return `true` when the formatted voltage was successfully written, otherwise `false`.
     */
    bool voltage(double volts, char* buffer, size_t size);

    /**
     * Formats a storage capacity.
     * @param bytes  Capacity in bytes.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return `true` when the formatted storage capacity was successfully written, otherwise `false`.
     */
    bool storageCapacity(uint64_t bytes, char* buffer, size_t size);
}
