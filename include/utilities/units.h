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
     * Converts kilometres to metres.
     * @param kilometers Distance in kilometres.
     * @return Distance in metres.
     */
    double kilometersToMeters(double kilometers);

    /**
     * Converts kilometres to miles.
     * @param kilometers Distance in kilometres.
     * @return Distance in miles.
     */
    double kilometersToMiles(double kilometers);

    /**
     * Converts metres to feet.
     * @param meters Distance in metres.
     * @return Distance in feet.
     */
    double metersToFeet(double meters);

    /**
     * Converts a speed from kilometres per hour to miles per hour.
     * @param kilometersPerHour Speed in kilometres per hour.
     * @return Speed in miles per hour.
     */
    double kilometersPerHourToMilesPerHour(double kilometersPerHour);
}
