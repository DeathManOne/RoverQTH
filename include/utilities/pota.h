/*
 * include/utilities/pota.h
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

namespace utilities::pota {
    constexpr size_t CODE_SIZE = 16U;
    constexpr size_t AREA_SIZE = 96U;

    struct Park {
        char code[CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
    };

    bool parseCsvRecord(const char* line, Park& park, bool& active);
    bool selectNearest(double latitude, double longitude,
        const Park& candidate, bool& found,
        Park& nearest,         double& distanceKm);
}
