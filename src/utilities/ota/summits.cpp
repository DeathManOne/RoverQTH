#include "utilities/ota/summits.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "utilities/distance.h"

namespace summits  = utilities::ota::summits;
namespace summitsDB = database::ota::summits;
namespace distance = utilities::distance;

namespace {
    constexpr size_t CSV_FIELD_COUNT = 17U;

    bool _copyField(const char* const source, const size_t length, char* const destination, const size_t size);
    bool _readField(const char*& cursor, char* const destination, const size_t size, bool& delimiter);
    bool _parseLong(const char* const value, const long minimum, const long maximum, long& result);
    bool _parseDouble(const char* const value, const double minimum, const double maximum, double& result);
    bool _isDate(const char* const value);

    bool _copyField(const char* const source, const size_t length, char* const destination, const size_t size) {
        if (source == nullptr || destination == nullptr || size == 0U || length >= size) { return false; }

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
                        if (length + 1U >= size)
                            { return false; }
                        destination[length++] = '"';
                        cursor += 2;
                        continue;
                    }
                    ++cursor;
                    quoted = false;
                    while (*cursor == ' ' || *cursor == '\t')
                        { ++cursor; }
                    if (*cursor != ',' && *cursor != '\0')
                        { return false; }
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

        for (size_t index = 0U; index < summitsDB::VERSION_SIZE - 1U; ++index) {
            if (index == 4U || index == 7U) {
                if (value[index] != '-')
                    { return false; }
            } else if (value[index] < '0' || value[index] > '9')
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
    for (size_t index = 0U; index < CSV_FIELD_COUNT; ++index) {
        char* destination = field;
        size_t destinationSize = sizeof(field);
        if (index == 0U) { destination = code;        destinationSize = sizeof(code); }
        if (index == 1U) { destination = area;        destinationSize = sizeof(area); }
        if (index == 4U) { destination = altitude;    destinationSize = sizeof(altitude); }
        if (index == 8U) { destination = longitude;   destinationSize = sizeof(longitude); }
        if (index == 9U) { destination = latitude;    destinationSize = sizeof(latitude); }
        if (index == 10U){ destination = points;      destinationSize = sizeof(points); }
        if (index == 11U){ destination = bonus;       destinationSize = sizeof(bonus); }

        bool delimiter = false;
        if (!_readField(cursor, destination, destinationSize, delimiter) ||
            delimiter != (index + 1U < CSV_FIELD_COUNT)
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
    if (!_copyField(code, std::strlen(code), summit.code, sizeof(summit.code))) { return false; }

    const size_t areaLength = std::strlen(area) < sizeof(summit.area) - 1U
        ? std::strlen(area)
        : sizeof(summit.area) - 1U;

    if (!_copyField(
        area,        areaLength,
        summit.area, sizeof(summit.area))
    ) { return false; }

    summit.altitude = static_cast<int16_t>(parsedAltitude);
    summit.points   = static_cast<uint8_t>(parsedPoints);
    summit.bonus    = static_cast<uint8_t>(parsedBonus);

    return true;
}

bool summits::isVersionNewer(const char* const candidate, const char* const installed) {
    if (!_isDate(candidate)) { return false; }
    if (!_isDate(installed)) { return true; }
    return std::strcmp(candidate, installed) > 0;
}

bool summits::isVersionValid(const char* const version) {
    return _isDate(version);
}

bool summits::selectNearest(const double latitude, const double longitude,
    const summitsDB::Summit& candidate, bool& found,
    summitsDB::Summit& nearest,         double& distanceKm
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
