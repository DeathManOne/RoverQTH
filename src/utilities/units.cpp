/*
 * src/utilities/units.cpp
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

#include "utilities/units.h"

namespace units = utilities::units;

namespace {
    constexpr double METERS_PER_KILOMETER = 1000.0;
    constexpr double METERS_PER_MILE      = 1609.344;
    constexpr double FEET_PER_METER       = 3.280839895013123;
}

double units::kilometersToMeters(const double kilometers) {
    return kilometers * METERS_PER_KILOMETER;
}

double units::kilometersToMiles(const double kilometers) {
    return kilometers * METERS_PER_KILOMETER / METERS_PER_MILE;
}

double units::metersToFeet(const double meters) {
    return meters * FEET_PER_METER;
}

double units::kilometersPerHourToMilesPerHour(const double kilometersPerHour) {
    return kilometersToMiles(kilometersPerHour);
}