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
#include "utilities/ota.h"

namespace ota = utilities::ota;

namespace {
    bool _validCode(const char* const code) {
        if (code == nullptr || code[0] == '\0') { return false; }
        for (size_t index = 0U; index < ota::CODE_SIZE; ++index) {
            if (code[index] == '\0')
                { return true; }
        }
        return false;
    }
}

void ota::clear(SearchResults &results) {
    results = {};
}

bool ota::retainNearest(SearchResults &results, const SearchResult &candidate) {
    if (results.count > RESULT_CAPACITY) { return false; }

    if (!_validCode(candidate.code) ||
        !std::isfinite(candidate.distanceKm) ||
        candidate.distanceKm < 0.0
    ) { return false; }

    size_t insertionIndex = 0U;
    while (insertionIndex < results.count &&
           results.items[insertionIndex].distanceKm <= candidate.distanceKm
    ) { ++insertionIndex; }

    if (results.count == RESULT_CAPACITY &&
        insertionIndex == RESULT_CAPACITY
    ) { return false; }

    const size_t lastIndex =
        results.count < RESULT_CAPACITY
            ? results.count
            : RESULT_CAPACITY - 1U;

    for (size_t index = lastIndex; index > insertionIndex; --index)
        { results.items[index] = results.items[index - 1U]; }
    results.items[insertionIndex] = candidate;

    if (results.count < RESULT_CAPACITY) { ++results.count; }
    return true;
}
