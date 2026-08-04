/*
 * include/utilities/distance.h
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

namespace utilities::distance {
    /**
     * Computes the great-circle distance between two coordinates.
     * @param latitudeFrom  Latitude of the starting point in decimal degrees.
     * @param longitudeFrom Longitude of the starting point in decimal degrees.
     * @param latitudeTo    Latitude of the destination point in decimal degrees.
     * @param longitudeTo   Longitude of the destination point in decimal degrees.
     * @return Distance in kilometers.
     */
    double betweenKilometers(double latitudeFrom, double longitudeFrom, double latitudeTo, double longitudeTo);

    /**
     * Computes the initial bearing from one coordinate to another.
     * @param latitudeFrom  Latitude of the starting point in decimal degrees.
     * @param longitudeFrom Longitude of the starting point in decimal degrees.
     * @param latitudeTo    Latitude of the destination point in decimal degrees.
     * @param longitudeTo   Longitude of the destination point in decimal degrees.
     * @return Initial bearing in degrees within the range [0, 360].
     */
    double bearingDegrees(double latitudeFrom, double longitudeFrom, double latitudeTo, double longitudeTo);

    /**
     * Returns the cardinal direction corresponding to a bearing.
     * @param degrees Bearing in degrees.
     * @return Cardinal direction ("N", "NE", "E", "SE", "S", "SW", "W" or "NW").
     */
    const char* cardinal(double degrees);
}
