/*
 * include/services/sota.h
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
#include "utilities/sota.h"

namespace services::sota {
    /** Runtime state of the asynchronous nearest-summit search service. */
    enum class Status : uint8_t {
        UNAVAILABLE, /**< No valid installed SOTA database is available. */
        IDLE,        /**< The service is ready and has no cached result. */
        SEARCHING,   /**< A nearest-summit search is running. */
        READY,       /**< A valid nearest-summit result is cached. */
        ERROR        /**< The last search failed. */
    };

    enum class NearbyStatus : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        EMPTY,
        ERROR
    };

    /** Thread-safe copy of the current SOTA search state and cached result. */
    struct Snapshot {
        Status status = Status::UNAVAILABLE;    /**< Current service state. */
        utilities::sota::Summit summit {};      /**< Cached nearest summit. */
        double distanceKm = 0.0;                /**< Distance to the summit in kilometers. */
        double bearingDeg = 0.0;                /**< Initial bearing to the summit in degrees. */
    };

    struct NearbySnapshot {
        NearbyStatus status = NearbyStatus::UNAVAILABLE;
        size_t count        = 0U;
    };

    /**
     * @brief Initializes the runtime state and checks for an installed database.
     * @return true when the service state has been initialized.
     */
    void begin();

    /**
     * @brief Clears the cached result after the installed database changes.
     * @note The next valid request starts a new asynchronous search.
     */
    void invalidate();

    /**
     * @brief Requests an asynchronous search for the nearest SOTA summit.
     * @param latitude Latitude of the search position in decimal degrees.
     * @param longitude Longitude of the search position in decimal degrees.
     * @return true if the request was started or the cached result remains valid;
     *         false if the position is invalid, the service is unavailable, an
     *         error is active, or another operation prevents the search.
     * @note A cached result remains valid until the position moves at least one kilometer.
     */
    bool requestNearest(double latitude, double longitude);

    bool requestByCode(const char* code, double latitude, double longitude);
    bool requestByPrefix(const char* prefix, double latitude, double longitude);
    bool requestNearby(double latitude, double longitude, double radiusKm);
    bool cancelNearby();
    NearbySnapshot nearbySnapshot();
    bool nearbyResult(size_t index, utilities::ota::SearchResult &result);

    /**
     * @brief Returns a thread-safe copy of the current service state.
     * @return Current search status and cached nearest-summit result.
     */
    Snapshot snapshot();

    /**
     * @brief Indicates whether a nearest-summit task is running.
     * @return true while a search is active, otherwise false.
     */
    bool isBusy();
}
