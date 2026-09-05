/*
 * src/utilities/ota/parks.cpp
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

#include "utilities/ota/parks.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "utilities/csv.h"
#include "utilities/ota.h"

namespace parksDB = database::ota::parks;
namespace csv     = utilities::csv;
namespace ota     = utilities::ota;
namespace parks   = utilities::ota::parks;

namespace {
    constexpr size_t CSV_FIELD_COUNT = 8U;

    bool _parseActive(const char* const value, bool& active);
    bool _parseCoordinate(const char* const value, const double minimum, const double maximum, double& coordinate);

    bool _parseActive(const char* const value, bool& active) {
        if (value == nullptr || value[0] == '\0') { return false; }

        errno             = 0;
        char* end         = nullptr;
        const long parsed = std::strtol(value, &end, 10);
        if (errno == ERANGE || end == value || *end != '\0' || (parsed != 0L && parsed != 1L))
            { return false; }

        active = parsed == 1L;
        return true;
    }

    bool _parseCoordinate(const char* const value, const double minimum,
        const double maximum, double& coordinate
    ) {
        if (value == nullptr || value[0] == '\0') { return false; }

        errno               = 0;
        char* end           = nullptr;
        const double parsed = std::strtod(value, &end);
        if (errno == ERANGE || end == value || *end != '\0' ||
            !std::isfinite(parsed) || parsed < minimum || parsed > maximum
        ) { return false; }

        coordinate = parsed;
        return true;
    }
}

bool parks::parseCsvRecord(const char* const line, parksDB::Park& park, bool& active) {
    park   = parksDB::Park {};
    active = false;
    if (line == nullptr || line[0] == '\0') { return false; }

    const char* cursor = line;
    char unused[512];
    char code[parksDB::CODE_SIZE];
    char activeValue[2];
    char area[512];
    char latitude[32];
    char longitude[32];

    for (size_t fieldIndex = 0U; fieldIndex < CSV_FIELD_COUNT; ++fieldIndex) {
        char* destination      = unused;
        size_t destinationSize = sizeof(unused);

        if (fieldIndex == 0U) { destination = code;        destinationSize = sizeof(code); }
        if (fieldIndex == 2U) { destination = activeValue; destinationSize = sizeof(activeValue); }
        if (fieldIndex == 4U) { destination = area;        destinationSize = sizeof(area); }
        if (fieldIndex == 5U) { destination = latitude;    destinationSize = sizeof(latitude); }
        if (fieldIndex == 6U) { destination = longitude;   destinationSize = sizeof(longitude); }

        bool delimiter = false;
        if (!csv::readField(cursor, destination, destinationSize, delimiter) ||
            delimiter != (fieldIndex + 1U < CSV_FIELD_COUNT)
        ) { return false; }
    }

    if (*cursor != '\0' || !_parseActive(activeValue, active)) { return false; }
    if (!active) { return true; }

    if (code[0] == '\0' || area[0] == '\0' ||
        latitude[0] == '\0' || longitude[0] == '\0'
    ) {
        active = false;
        return true;
    }

    if (!_parseCoordinate(latitude,  -90.0,  90.0,  park.latitude) ||
        !_parseCoordinate(longitude, -180.0, 180.0, park.longitude)
    ) {
        active = false;
        park   = parksDB::Park {};
        return true;
    }

    const size_t areaLength = std::strlen(area) < sizeof(park.area) - 1U
        ? std::strlen(area)
        : sizeof(park.area) - 1U;

    if (!csv::copyField(code, std::strlen(code), park.code, sizeof(park.code)) ||
        !csv::copyField(area, areaLength,        park.area, sizeof(park.area))
    ) { return false; }
    return true;
}

bool parks::selectNearest(const double latitude, const double longitude,
    const parksDB::Park& candidate, bool& found,
    parksDB::Park& nearest,         double& distanceKm
) {
    bool selected = false;

    if (!ota::selectNearestDistance(
        latitude,           longitude,
        candidate.latitude, candidate.longitude,
        found, distanceKm, selected
    )) { return false; }

    if (selected) {
        nearest = candidate;
        found   = true;
    }

    return true;
}
