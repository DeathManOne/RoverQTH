/*
 * src/utilities/pota.cpp
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

#include "utilities/pota.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "utilities/distance.h"

namespace pota = utilities::pota;
namespace distance = utilities::distance;

namespace {
    constexpr size_t CSV_FIELD_COUNT = 8U;

    bool _copyField(const char* const source, const size_t length, char* const destination, const size_t size);
    bool _readField(const char*& cursor, char* const destination, const size_t size, bool& delimiter);
    bool _parseActive(const char* const value, bool& active);
    bool _parseCoordinate(const char* const value, const double minimum, const double maximum, double& coordinate);

    bool _copyField(const char* const source, const size_t length, char* const destination, const size_t size) {
        if (source == nullptr || destination == nullptr ||
            size == 0U || length >= size
        ) { return false; }

        std::memcpy(destination, source, length);
        destination[length] = '\0';
        return true;
    }

    bool _readField(const char*& cursor, char* const destination, const size_t size, bool& delimiter) {
        delimiter = false;
        if (cursor == nullptr || destination == nullptr || size == 0U) { return false; }

        size_t length = 0U;
        bool quoted   = false;
        if (*cursor == '"') {
            quoted = true;
            ++cursor;
        }

        while (*cursor != '\0') {
            if (quoted) {
                if (*cursor == '"') {
                    if (cursor[1] == '"') {
                        if (length + 1U >= size) { return false; }
                        destination[length++] = '"';
                        cursor += 2;
                        continue;
                    }

                    ++cursor;
                    quoted = false;
                    while (*cursor == ' ' || *cursor == '\t') { ++cursor; }
                    if (*cursor != ',' && *cursor != '\0') { return false; }
                    break;
                }
            } else {
                if (*cursor == ',') { break; }
                if (*cursor == '"') { return false; }
            }

            if (length + 1U >= size) { return false; }
            destination[length++] = *cursor++;
        }

        if (quoted) { return false; }
        destination[length] = '\0';

        if (*cursor == ',') {
            delimiter = true;
            ++cursor;
        }
        return true;
    }

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

bool pota::parseCsvRecord(const char* const line, Park& park, bool& active) {
    park   = Park {};
    active = false;
    if (line == nullptr || line[0] == '\0') { return false; }

    const char* cursor = line;
    char unused[512];
    char code[CODE_SIZE];
    char activeValue[2];
    char area[512];
    char latitude[32];
    char longitude[32];

    for (size_t index = 0U; index < CSV_FIELD_COUNT; ++index) {
        char* destination      = unused;
        size_t destinationSize = sizeof(unused);

        if (index == 0U) { destination = code;        destinationSize = sizeof(code); }
        if (index == 2U) { destination = activeValue; destinationSize = sizeof(activeValue); }
        if (index == 4U) { destination = area;        destinationSize = sizeof(area); }
        if (index == 5U) { destination = latitude;    destinationSize = sizeof(latitude); }
        if (index == 6U) { destination = longitude;   destinationSize = sizeof(longitude); }

        bool delimiter = false;
        if (!_readField(cursor, destination, destinationSize, delimiter) ||
            delimiter != (index + 1U < CSV_FIELD_COUNT)
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
        park   = Park {};
        return true;
    }

    const size_t areaLength = std::strlen(area) < sizeof(park.area) - 1U
        ? std::strlen(area)
        : sizeof(park.area) - 1U;

    if (!_copyField(code, std::strlen(code), park.code, sizeof(park.code)) ||
        !_copyField(area, areaLength, park.area, sizeof(park.area))
    ) { return false; }

    return true;
}

bool pota::selectNearest(const double latitude, const double longitude,
    const Park& candidate, bool& found,
    Park& nearest,         double& distanceKm
) {
    if (!std::isfinite(latitude)           || !std::isfinite(longitude)           ||
        latitude  < -90.0  || latitude  > 90.0  ||
        longitude < -180.0 || longitude > 180.0 ||
        !std::isfinite(candidate.latitude) || !std::isfinite(candidate.longitude) ||
        candidate.latitude  < -90.0  || candidate.latitude  > 90.0                ||
        candidate.longitude < -180.0 || candidate.longitude > 180.0               ||
        (found && (!std::isfinite(distanceKm) || distanceKm < 0.0))
    ) { return false; }

    const double candidateDistance = distance::betweenKilometers(
        latitude,           longitude,
        candidate.latitude, candidate.longitude
    );

    if (!std::isfinite(candidateDistance)) { return false; }
    if (!found || candidateDistance < distanceKm) {
        nearest    = candidate;
        distanceKm = candidateDistance;
        found      = true;
    }

    return true;
}
