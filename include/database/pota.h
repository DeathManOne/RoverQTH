/*
 * include/database/pota.h
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

#include "utilities/ota.h"
#include "utilities/pota.h"

namespace database::pota {
    constexpr size_t ETAG_SIZE = 96U;

    struct Info {
        uint32_t records    = 0U;
        uint64_t sourceSize = 0U;
        char etag[ETAG_SIZE] {};
    };

    using ProgressCallback = void (*)(uint8_t progress, void* userData);

    bool info(Info& value);

    bool buildCandidate(const char* csvPath, const char* etag,
        uint64_t sourceSize, ProgressCallback callback = nullptr, void* userData = nullptr
    );

    bool installCandidate();
    void discardCandidate();

    bool findNearest(double latitude, double longitude,
        utilities::pota::Park& park, double& distanceKm, double& bearing
    );

    bool findByCode(const char* code, utilities::pota::Park& park);

    utilities::ota::SearchStatus findNearby(double latitude, double longitude, double radiusKm,
        utilities::ota::SearchResults &results,
        utilities::ota::CancelCallback cancelCallback = nullptr,
        void* cancelUserData = nullptr
    );
}
