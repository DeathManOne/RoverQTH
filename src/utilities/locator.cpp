/*
 * src/utilities/locator.cpp
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
#include <cstdio>
#include "utilities/locator.h"

namespace locator = utilities::locator;

bool locator::fromCoordinates(const double latitude, const double longitude, char* const buffer, const size_t size) {
    if (buffer == nullptr || size == 0) { return false; }

    buffer[0] = '\0';
    if (!std::isfinite(latitude) || !std::isfinite(longitude) ||
        latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0
    ) { return false; }

    char grid[11];
    double lat = latitude;
    double lon = longitude;

    if (lat >= 90.0)  { lat = 89.999999; }
    if (lon >= 180.0) { lon = 179.999999; }
    lon += 180.0;
    lat += 90.0;

    grid[0] = static_cast<char>('A' + static_cast<int>(lon / 20.0));
    grid[1] = static_cast<char>('A' + static_cast<int>(lat / 10.0));
    lon = std::fmod(lon, 20.0);
    lat = std::fmod(lat, 10.0);

    grid[2] = static_cast<char>('0' + static_cast<int>(lon / 2.0));
    grid[3] = static_cast<char>('0' + static_cast<int>(lat));
    lon = std::fmod(lon, 2.0);
    lat = std::fmod(lat, 1.0);

    grid[4] = static_cast<char>('A' + static_cast<int>(lon * 12.0));
    grid[5] = static_cast<char>('A' + static_cast<int>(lat * 24.0));
    lon = std::fmod(lon, 2.0 / 24.0);
    lat = std::fmod(lat, 1.0 / 24.0);

    grid[6] = static_cast<char>('0' + static_cast<int>(lon * 120.0));
    grid[7] = static_cast<char>('0' + static_cast<int>(lat * 240.0));
    lon = std::fmod(lon, 2.0 / 240.0);
    lat = std::fmod(lat, 1.0 / 240.0);

    grid[8] = static_cast<char>('A' + static_cast<int>(lon * 2880.0));
    grid[9] = static_cast<char>('A' + static_cast<int>(lat * 5760.0));

    grid[10] = '\0';
    const int written = std::snprintf(buffer, size, "%c%c%c%c %c%c %c%c %c%c",
        grid[0], grid[1], grid[2], grid[3],
        grid[4], grid[5],
        grid[6], grid[7],
        grid[8], grid[9]
    );
    return written > 0 && static_cast<size_t>(written) < size;
}