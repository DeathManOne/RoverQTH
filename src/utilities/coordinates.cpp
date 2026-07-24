/*
 * src/utilities/coordinates.cpp
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
#include "utilities/coordinates.h"

namespace coordinates = utilities::coordinates;

namespace {
    char _getHemisphere(const double value, const coordinates::Axis axis) {
        if (axis == coordinates::Axis::LATITUDE) { return value >= 0.0 ? 'N' : 'S'; }
        return value >= 0.0 ? 'E' : 'W';
    }

    bool _prepare(const double value, const coordinates::Axis axis, char* const buffer, const size_t size) {
        if (buffer == nullptr || size == 0) { return false; }

        buffer[0] = '\0';
        if (!std::isfinite(value)) { return false; }
        const double limit = axis == coordinates::Axis::LATITUDE ? 90.0 : 180.0;
        return value >= -limit && value <= limit;
    }
}

bool coordinates::formatDD(const double value, const Axis axis, char* const buffer, const size_t size) {
    if (!_prepare(value, axis, buffer, size)) { return false; }

    const char hemisphere = _getHemisphere(value, axis);
    const double absolute = std::abs(value);
    const int written     = axis == Axis::LATITUDE
        ? std::snprintf(buffer, size, "%09.6f° %c",  absolute, hemisphere)
        : std::snprintf(buffer, size, "%010.6f° %c", absolute, hemisphere);
    return written >= 0 && static_cast<size_t>(written) < size;
}

bool coordinates::formatDDM(const double value, const Axis axis, char* const buffer, const size_t size) {
    if (!_prepare(value, axis, buffer, size)) { return false; }

    const char hemisphere = _getHemisphere(value, axis);
    const double absolute = std::abs(value);
    int degrees           = static_cast<int>(absolute);
    double minutes        = (absolute - degrees) * 60.0;

    if (minutes >= 59.99995) {
        ++degrees;
        minutes = 0.0;
    }

    const int written = axis == Axis::LATITUDE
        ? std::snprintf(buffer, size, "%02d°%07.4f' %c", degrees, minutes, hemisphere)
        : std::snprintf(buffer, size, "%03d°%07.4f' %c", degrees, minutes, hemisphere);
    return written >= 0 && static_cast<size_t>(written) < size;
}

bool coordinates::formatDMS(const double value, const Axis axis, char* const buffer, const size_t size) {
    if (!_prepare(value, axis, buffer, size)) { return false; }

    const char hemisphere    = _getHemisphere(value, axis);
    const double absolute    = std::abs(value);
    int degrees              = static_cast<int>(absolute);
    const double minutesFull = (absolute - degrees) * 60.0;
    int minutes              = static_cast<int>(minutesFull);
    double seconds           = (minutesFull - minutes) * 60.0;

    if (seconds >= 59.995) {
        seconds = 0.0;
        ++minutes;

        if (minutes >= 60) {
            minutes = 0;
            ++degrees;
        }
    }

    const int written = axis == Axis::LATITUDE
        ? std::snprintf(buffer, size, "%02d°%02d'%05.2f\" %c", degrees, minutes, seconds, hemisphere)
        : std::snprintf(buffer, size, "%03d°%02d'%05.2f\" %c", degrees, minutes, seconds, hemisphere);
    return written >= 0 && static_cast<size_t>(written) < size;
}