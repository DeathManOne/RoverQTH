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
#include <cstdint>

namespace utilities::ota {
    constexpr size_t CODE_SIZE       = 16U;
    constexpr size_t RESULT_CAPACITY = 64U;

    enum class SearchStatus : uint8_t {
        SUCCESS,
        CANCELLED,
        ERROR
    };

    struct SearchResult {
        char code[CODE_SIZE] {};
        double distanceKm = 0.0;
        uint8_t points    = 0U;
        uint8_t bonus     = 0U;
    };

    struct SearchResults {
        SearchResult items[RESULT_CAPACITY] {};
        size_t count = 0U;
    };

    using CancelCallback = bool (*)(void* userData);

    void clear(SearchResults &results);
    bool retainNearest(SearchResults &results, const SearchResult &candidate);
}