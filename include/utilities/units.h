/*
 * include/utilities/units.h
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

namespace utilities::units {
    /**
     * Converts kilometers to meters.
     * @param kilometers Distance in kilometers.
     * @return Distance in meters.
     */
    double kilometersToMeters(double kilometers);

    /**
     * Converts kilometers to miles.
     * @param kilometers Distance in kilometers.
     * @return Distance in miles.
     */
    double kilometersToMiles(double kilometers);

    /**
     * Converts meters to feet.
     * @param meters Distance in meters.
     * @return Distance in feet.
     */
    double metersToFeet(double meters);

    /**
     * Converts a speed from kilometers per hour to miles per hour.
     * @param kilometersPerHour Speed in kilometers per hour.
     * @return Speed in miles per hour.
     */
    double kilometersPerHourToMilesPerHour(double kilometersPerHour);
}
