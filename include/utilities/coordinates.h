/*
 * include/utilities/coordinates.h
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

namespace utilities::coordinates {
    enum class Axis : uint8_t {LATITUDE, LONGITUDE};

    /**
     * Formats a coordinate in decimal degrees.
     * Output format:
     *   Latitude : DD.dddddd° H
     *   Longitude: DDD.dddddd° H
     * @param value  Coordinate in decimal degrees.
     * @param axis   Coordinate axis.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return True if the formatted coordinate fits in the destination buffer, otherwise false.
     */
    bool formatDD(double value, Axis axis, char* buffer, size_t size);

    /**
     * Formats a coordinate in degrees and decimal minutes.
     * Output format:
     *   Latitude : DD°MM.mmmm' H
     *   Longitude: DDD°MM.mmmm' H
     * @param value  Coordinate in decimal degrees.
     * @param axis   Coordinate axis.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return True if the formatted coordinate fits in the destination buffer, otherwise false.
     */
    bool formatDDM(double value, Axis axis, char* buffer, size_t size);

    /**
     * Formats a coordinate in degrees, minutes and seconds.
     * Output format:
     *   Latitude : DD°MM'SS.ss" H
     *   Longitude: DDD°MM'SS.ss" H
     * @param value  Coordinate in decimal degrees.
     * @param axis   Coordinate axis.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return if the formatted coordinate fits in the destination buffer, otherwise false.
     */
    bool formatDMS(double value, Axis axis, char* buffer, size_t size);
}
