/*
 * src/utilities/format.cpp
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

#include "utilities/distance.h"
#include "utilities/format.h"
#include "utilities/units.h"

namespace format   = utilities::format;
namespace distance = utilities::distance;
namespace units    = utilities::units;

namespace {
    constexpr double BYTES_PER_GIGABYTE = 1024.0 * 1024.0 * 1024.0;

    bool _isValidBuffer(const char* const buffer, const size_t size) {
        return buffer != nullptr && size > 0;
    }

    bool _isWritten(const int written, const size_t size) {
        return written >= 0 && static_cast<size_t>(written) < size;
    }
}

bool format::durationCompact(uint32_t seconds, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }

    const uint32_t hours   = seconds / 3600U;
    seconds %= 3600U;
    const uint32_t minutes = seconds / 60U;
    seconds %= 60U;

    int written = 0;
    if (hours > 0U) {
        written = std::snprintf(buffer, size,
            "%luh %02lum",
            static_cast<unsigned long>(hours),
            static_cast<unsigned long>(minutes)
        );
    } else {
        written = std::snprintf(buffer, size,
            "%lum %02lus",
            static_cast<unsigned long>(minutes),
            static_cast<unsigned long>(seconds)
        );
    }
    return _isWritten(written, size);
}

bool format::durationHMS(const uint32_t seconds, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) {return false; }

    const uint32_t hours            = seconds / 3600U;
    const uint32_t minutes          = (seconds % 3600U) / 60U;
    const uint32_t remainingSeconds = seconds % 60U;

    const int written = std::snprintf(buffer, size,
        "%02lu:%02lu:%02lu",
        static_cast<unsigned long>(hours),
        static_cast<unsigned long>(minutes),
        static_cast<unsigned long>(remainingSeconds)
    );
    return _isWritten(written, size);
}

bool format::speed(const double kilometersPerHour, const bool imperial, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }

    const int written = imperial
        ? std::snprintf(buffer, size,
            "%.1f mph",
            units::kilometersPerHourToMilesPerHour(kilometersPerHour)
        ) : std::snprintf(buffer, size, "%.1f km/h", kilometersPerHour);
    return _isWritten(written, size);
}

bool format::altitude(const double meters, const bool imperial, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    const int written = imperial
        ? std::snprintf(buffer, size, "%.0f ft", units::metersToFeet(meters))
        : std::snprintf(buffer, size, "%.0f m",  meters);
    return _isWritten(written, size);
}

bool format::distance(const double kilometers, const bool imperial, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }

    if (kilometers < 0.0) { return _isWritten(std::snprintf(buffer, size, "--"), size); }
    if (imperial) {
        const double miles = units::kilometersToMiles(kilometers);
        if (miles < 0.1) {
            const double feet = units::metersToFeet(units::kilometersToMeters(kilometers));
            return _isWritten(std::snprintf(buffer, size, "%.0f ft", feet), size);
        }
        return _isWritten(std::snprintf(buffer, size, "%.1f mi", miles), size);
    }

    if (kilometers < 1.0)
        { return _isWritten(std::snprintf(buffer, size, "%.0f m", units::kilometersToMeters(kilometers)), size); }
    if (kilometers < 10.0)
        { return _isWritten(std::snprintf(buffer, size, "%.2f km", kilometers), size); }
    return _isWritten(std::snprintf(buffer, size, "%.1f km", kilometers), size);
}

bool format::bearing(const double degrees, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    if (!std::isfinite(degrees) || degrees < 0.0)
        { return _isWritten(std::snprintf(buffer, size, "--"), size); }
    return _isWritten(std::snprintf(buffer, size, "%.0f°", degrees), size);
}

bool format::heading(const double degrees, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    if (!std::isfinite(degrees)) {
        buffer[0] = '\0';
        return true;
    }
    return _isWritten(std::snprintf(buffer, size, "%.0f° %s", degrees, distance::cardinal(degrees)), size);
}

bool format::percentage(const uint64_t value, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    const int written = std::snprintf(buffer, size, "%llu %%", static_cast<unsigned long long>(value));
    return _isWritten(written, size);
}

bool format::voltage(const double volts, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    const int written = std::snprintf(buffer, size, "%.2f V", volts);
    return _isWritten(written, size);
}

bool format::capacityMilliAmpHours(const uint32_t milliAmpHours, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    const int written = std::snprintf(buffer, size, "%lu mAh", static_cast<unsigned long>(milliAmpHours));
    return _isWritten(written, size);
}

bool format::storageCapacity(const uint64_t bytes, char* const buffer, const size_t size) {
    if (!_isValidBuffer(buffer, size)) { return false; }
    const int written = std::snprintf(buffer, size, "%.1f GB", static_cast<double>(bytes) / BYTES_PER_GIGABYTE);
    return _isWritten(written, size);
}
