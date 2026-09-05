/*
 * src/utilities/ota/summits.cpp
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

#include "utilities/ota/summits.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "utilities/csv.h"
#include "utilities/ota.h"

namespace summitsDB = database::ota::summits;
namespace csv       = utilities::csv;
namespace ota       = utilities::ota;
namespace summits   = utilities::ota::summits;

namespace {
    constexpr size_t CSV_FIELD_COUNT = 17U;

    bool _parseLong(const char* const value, const long minimum, const long maximum, long& result);
    bool _parseDouble(const char* const value, const double minimum, const double maximum, double& result);
    bool _isDate(const char* const value);

    bool _parseLong(const char* const value, const long minimum, const long maximum, long& result) {
        if (value == nullptr || value[0] == '\0') { return false; }

        errno             = 0;
        char* end         = nullptr;
        const long parsed = std::strtol(value, &end, 10);

        if (errno == ERANGE  ||
            end == value     || *end != '\0' ||
            parsed < minimum || parsed > maximum
        ) { return false; }

        result = parsed;
        return true;
    }

    bool _parseDouble(const char* const value, const double minimum, const double maximum, double& result) {
        if (value == nullptr || value[0] == '\0') { return false; }

        errno               = 0;
        char* end           = nullptr;
        const double parsed = std::strtod(value, &end);

        if (errno == ERANGE  ||
            end == value     || *end != '\0' ||
            !std::isfinite(parsed)           ||
            parsed < minimum || parsed > maximum
        ) { return false; }

        result = parsed;
        return true;
    }

    bool _isDate(const char* const value) {
        if (value == nullptr || std::strlen(value) != summitsDB::VERSION_SIZE - 1U) { return false; }

        for (size_t characterIndex = 0U; characterIndex < summitsDB::VERSION_SIZE - 1U; ++characterIndex) {
            if (characterIndex == 4U || characterIndex == 7U) {
                if (value[characterIndex] != '-')
                    { return false; }
            } else if (value[characterIndex] < '0' || value[characterIndex] > '9')
                { return false; }
        }

        const int year  = (value[0] - '0') * 1000 + (value[1] - '0') * 100 + (value[2] - '0') * 10 + value[3] - '0';
        const int month = (value[5] - '0') * 10   + value[6] - '0';
        const int day   = (value[8] - '0') * 10   + value[9] - '0';
        if (year < 2000 || month < 1 || month > 12 || day < 1) { return false; }

        static constexpr uint8_t DAYS[] = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
        int maximum = DAYS[month - 1];

        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
            { maximum = 29; }
        return day <= maximum;
    }
}

bool summits::parseListVersion(const char* const line, char* const version, const size_t size) {
    if (version == nullptr || size == 0U)       { return false; }
    version[0] = '\0';
    if (line == nullptr || size < summitsDB::VERSION_SIZE) { return false; }

    const char* date = std::strstr(line, "Date=");
    if (date != nullptr) { date += 5; }
    else {
        date = std::strstr(line, "Date: ");
        if (date == nullptr) { return false; }
        date += 6;
    }

    if (std::strlen(date) < 10U) { return false; }
    if (date[0] <  '0' || date[0] > '9' || date[1] < '0' || date[1] > '9' ||
        date[2] != '/' || date[3] < '0' || date[3] > '9' || date[4] < '0' || date[4] > '9' ||
        date[5] != '/' || date[6] < '0' || date[6] > '9' || date[7] < '0' || date[7] > '9' ||
        date[8] <  '0' || date[8] > '9' || date[9] < '0' || date[9] > '9'
    ) { return false; }

    version[0]  = date[6];
    version[1]  = date[7];
    version[2]  = date[8];
    version[3]  = date[9];
    version[4]  = '-';
    version[5]  = date[3];
    version[6]  = date[4];
    version[7]  = '-';
    version[8]  = date[0];
    version[9]  = date[1];
    version[10] = '\0';

    if (!_isDate(version)) {
        version[0] = '\0';
        return false;
    }

    return true;
}

bool summits::parseCsvRecord(const char* const line, summitsDB::Summit& summit) {
    summit = summitsDB::Summit {};
    if (line == nullptr || line[0] == '\0') { return false; }

    const char* cursor = line;
    char field[512];
    char code[summitsDB::CODE_SIZE];
    char area[512];
    char altitude[16];
    char longitude[32];
    char latitude[32];
    char points[8];
    char bonus[8];

    code[0] = area[0] = altitude[0] = longitude[0] = latitude[0] = points[0] = bonus[0] = '\0';
    for (size_t fieldIndex = 0U; fieldIndex < CSV_FIELD_COUNT; ++fieldIndex) {
        char* destination = field;
        size_t destinationSize = sizeof(field);
        if (fieldIndex == 0U) { destination = code;        destinationSize = sizeof(code); }
        if (fieldIndex == 1U) { destination = area;        destinationSize = sizeof(area); }
        if (fieldIndex == 4U) { destination = altitude;    destinationSize = sizeof(altitude); }
        if (fieldIndex == 8U) { destination = longitude;   destinationSize = sizeof(longitude); }
        if (fieldIndex == 9U) { destination = latitude;    destinationSize = sizeof(latitude); }
        if (fieldIndex == 10U){ destination = points;      destinationSize = sizeof(points); }
        if (fieldIndex == 11U){ destination = bonus;       destinationSize = sizeof(bonus); }

        bool delimiter = false;
        if (!csv::readField(cursor, destination, destinationSize, delimiter) ||
            delimiter != (fieldIndex + 1U < CSV_FIELD_COUNT)
        ) { return false; }
    }
    if (*cursor != '\0') { return false; }

    long parsedAltitude = 0;
    long parsedPoints   = 0;
    long parsedBonus    = 0;
    if (code[0] == '\0' || area[0] == '\0' ||
        !_parseLong(altitude, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), parsedAltitude) ||
        !_parseLong(points, 0L, std::numeric_limits<uint8_t>::max(), parsedPoints) ||
        !_parseLong(bonus,  0L, std::numeric_limits<uint8_t>::max(), parsedBonus)  ||
        !_parseDouble(latitude,  -90.0,  90.0,  summit.latitude)                             ||
        !_parseDouble(longitude, -180.0, 180.0, summit.longitude)
    ) { return false; }

    if (!csv::copyField(
        code,        std::strlen(code),
        summit.code, sizeof(summit.code)
    )) { return false; }

    const size_t areaLength = std::strlen(area) < sizeof(summit.area) - 1U
        ? std::strlen(area)
        : sizeof(summit.area) - 1U;

    if (!csv::copyField(
        area,        areaLength,
        summit.area, sizeof(summit.area)
    )) { return false; }

    summit.altitude = static_cast<int16_t>(parsedAltitude);
    summit.points   = static_cast<uint8_t>(parsedPoints);
    summit.bonus    = static_cast<uint8_t>(parsedBonus);

    return true;
}

bool summits::isVersionValid(const char* const version) {
    return _isDate(version);
}

bool summits::selectNearest(const double latitude, const double longitude,
    const summitsDB::Summit& candidate, bool& found,
    summitsDB::Summit& nearest,         double& distanceKm
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
