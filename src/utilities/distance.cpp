/*
 * src/utilities/distance.cpp
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

#include <cmath>

#include "utilities/distance.h"

namespace distance = utilities::distance;

namespace {
    constexpr double PI              = 3.14159265358979323846;
    constexpr double EARTH_RADIUS_KM = 6371.0;

    double _toRadians(const double degrees) {
        return degrees * PI / 180.0;
    }

    double _toDegrees(const double radians) {
        return radians * 180.0 / PI;
    }

    double _normalizeDegrees(const double degrees) {
        double normalized = std::fmod(degrees, 360.0);
        if (normalized < 0.0)
            { normalized += 360.0; }
        return normalized;
    }
}

double distance::betweenKilometers(const double latitudeFrom, const double longitudeFrom, const double latitudeTo, const double longitudeTo) {
    const double latitudeFromRadians = _toRadians(latitudeFrom);
    const double latitudeToRadians   = _toRadians(latitudeTo);
    const double latitudeDifference  = _toRadians(latitudeTo - latitudeFrom);
    const double longitudeDifference = _toRadians(longitudeTo - longitudeFrom);
    const double latitudeSin         = std::sin(latitudeDifference / 2.0);
    const double longitudeSin        = std::sin(longitudeDifference / 2.0);

    double haversine =
        latitudeSin                   * latitudeSin +
        std::cos(latitudeFromRadians) * std::cos(latitudeToRadians) *
        longitudeSin                  * longitudeSin;

    if (haversine < 0.0)      { haversine = 0.0; }
    else if (haversine > 1.0) { haversine = 1.0; }

    const double centralAngle = 2.0 * std::atan2(std::sqrt(haversine), std::sqrt(1.0 - haversine));
    return EARTH_RADIUS_KM * centralAngle;
}

double distance::bearingDegrees(const double latitudeFrom, const double longitudeFrom, const double latitudeTo, const double longitudeTo) {
    const double latitudeFromRadians = _toRadians(latitudeFrom);
    const double latitudeToRadians   = _toRadians(latitudeTo);
    const double longitudeDifference = _toRadians(longitudeTo - longitudeFrom);

    const double y = std::sin(longitudeDifference) * std::cos(latitudeToRadians);
    const double x =
        std::cos(latitudeFromRadians) * std::sin(latitudeToRadians) -
        std::sin(latitudeFromRadians) * std::cos(latitudeToRadians) *
        std::cos(longitudeDifference);
    return _normalizeDegrees(_toDegrees(std::atan2(y, x)));
}

const char* distance::cardinal(const double degrees) {
    if (std::isnan(degrees)) { return ""; }
    static constexpr const char* DIRECTIONS[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    const double normalized = _normalizeDegrees(degrees + 22.5);
    const size_t index = static_cast<size_t>(normalized / 45.0) % 8;
    return DIRECTIONS[index];
}