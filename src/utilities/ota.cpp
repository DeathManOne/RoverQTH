/*
 * src/utilities/ota.cpp
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
#include "utilities/ota.h"

namespace distance = utilities::distance;
namespace ota      = utilities::ota;

bool ota::normalizeCodePrefix(const char* const input, char* const output, const size_t outputSize) {
    if (output == nullptr || outputSize == 0U) { return false; }

    output[0] = '\0';
    if (input == nullptr) { return false; }

    size_t outputIndex = 0U;
    for (size_t inputIndex = 0U; input[inputIndex] != '\0'; ++inputIndex) {
        char character = input[inputIndex];

        if (character == ' ' || character == '/' || character == '-')
            { continue; }

        if (character >= 'a' && character <= 'z')
            { character = static_cast<char>(character - 'a' + 'A'); }

        const bool valid = (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9');
        if (!valid || outputIndex + 1U >= outputSize) {
            output[0] = '\0';
            return false;
        }
        output[outputIndex++] = character;
    }

    if (outputIndex == 0U)
        { return false; }
    output[outputIndex] = '\0';
    return true;
}

bool ota::isNormalizedCodePrefix(const char* const prefix, const size_t prefixSize) {
    if (prefix == nullptr || prefixSize == 0U || prefix[0] == '\0')
        { return false; }

    for (size_t index = 0U; index < prefixSize; ++index) {
        const char character = prefix[index];

        if (character == '\0') { return true; }
        if (!((character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9')))
            { return false; }
    }

    return false;
}

bool ota::isValidSearchArea(const double latitude, const double longitude, const double radiusKm) {
    return
        std::isfinite(latitude)  &&
        std::isfinite(longitude) &&
        std::isfinite(radiusKm)  &&
        latitude  >= -90.0       && latitude  <= 90.0  &&
        longitude >= -180.0      && longitude <= 180.0 &&
        radiusKm  >= 0.0;
}

bool ota::selectNearestDistance(const double latitude, const double longitude, const double candidateLatitude, const double candidateLongitude, const bool found, double &distanceKm, bool &selected) {
    selected = false;

    if (!isValidSearchArea(latitude,          longitude,          0.0) ||
        !isValidSearchArea(candidateLatitude, candidateLongitude, 0.0) ||
        (found && (!std::isfinite(distanceKm) || distanceKm < 0.0))
    ) { return false; }

    const double candidateDistance = distance::betweenKilometers(latitude, longitude, candidateLatitude, candidateLongitude);

    if (!std::isfinite(candidateDistance)) { return false; }
    if (!found || candidateDistance < distanceKm) {
        distanceKm = candidateDistance;
        selected   = true;
    }
    return true;
}
