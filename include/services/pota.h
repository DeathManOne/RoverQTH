/*
 * include/services/pota.h
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

namespace services::pota {
    enum class Status : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        ERROR
    };

    enum class NearbyStatus : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        EMPTY,
        ERROR
    };

    struct Snapshot {
        Status status = Status::UNAVAILABLE;
        utilities::pota::Park park {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
    };

    struct NearbySnapshot {
        NearbyStatus status = NearbyStatus::UNAVAILABLE;
        size_t count        = 0U;
    };

    void begin();
    void invalidate();
    bool requestNearest(double latitude, double longitude);
    bool requestByCode(const char* code, double latitude, double longitude);
    bool requestByPrefix(const char* prefix, double latitude, double longitude);
    bool requestNearby(double latitude, double longitude, double radiusKm);
    bool cancelNearby();
    NearbySnapshot nearbySnapshot();
    bool nearbyResult(size_t index, utilities::ota::SearchResult &result);
    Snapshot snapshot();
    bool isBusy();
}
