/*
 * include/utilities/ota.h
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

#pragma once
#include <cstddef>

namespace utilities::ota {
    /** @brief Normalizes user input into an OTA code prefix suitable for searching. */
    bool normalizeCodePrefix(const char* input, char* output, size_t outputSize);

    /** @brief Checks that a code prefix is normalized, terminated and within capacity. */
    bool isNormalizedCodePrefix(const char* prefix, size_t prefixSize);

    /** @brief Validates the coordinates and radius used by a nearby-site search. */
    bool isValidSearchArea(double latitude, double longitude, double radiusKm);

    /**
     * @brief Tests whether a geographic candidate is nearer than the current selection.
     * @param found Whether a previous candidate has already been selected.
     * @param distanceKm Current best distance, updated when the candidate is nearer.
     * @param selected Receives whether the candidate must replace the current selection.
     * @return true when all coordinates and computed values are valid.
     */
    bool selectNearestDistance(
        double latitude,          double longitude,
        double candidateLatitude, double candidateLongitude,
        bool found, double &distanceKm,  bool &selected
    );
}
