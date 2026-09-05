/*
 * include/services/ota.h
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

#include "database/ota.h"

namespace services::ota {
    /** @brief Maximum area text size exposed by a unified OTA reference. */
    constexpr size_t AREA_SIZE = 96U;

    /** @brief Selects the summit or park database. */
    enum class Type : uint8_t {
        SUMMITS,
        PARKS
    };

    /** @brief State of a single-reference OTA lookup. */
    enum class Status : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        ERROR
    };

    /** @brief State of a multi-result OTA search. */
    enum class NearbyStatus : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        EMPTY,
        ERROR
    };

    /** @brief Unified summit-or-park reference exposed to consumers. */
    struct Reference {
        char code[database::ota::CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        int16_t altitude = 0;
        uint8_t points   = 0U;
        uint8_t bonus    = 0U;
    };

    /** @brief Thread-safe snapshot of a single-reference lookup. */
    struct Snapshot {
        Status status = Status::UNAVAILABLE;
        Type type     = Type::SUMMITS;
        Reference reference {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
    };

    /** @brief Thread-safe summary of the current multi-result search. */
    struct NearbySnapshot {
        NearbyStatus status = NearbyStatus::UNAVAILABLE;
        size_t count        = 0U;
    };

    /** @brief Initializes the OTA lookup service and its synchronization resources. */
    void begin();
    /** @brief Invalidates cached state associated with one database type. */
    void invalidate(Type type);

    /** @brief Starts an asynchronous nearest-reference lookup. */
    bool requestNearest(
        Type type,
        double latitude,
        double longitude
    );

    /** @brief Starts an asynchronous exact-code lookup. */
    bool requestByCode(
        Type type,
        const char* code,
        double latitude,
        double longitude
    );

    /** @brief Starts an asynchronous code-prefix search. */
    bool requestByPrefix(
        Type type,
        const char* prefix,
        double latitude,
        double longitude
    );

    /** @brief Starts an asynchronous geographic-radius search. */
    bool requestNearby(
        Type type,
        double latitude,
        double longitude,
        double radiusKm
    );

    /** @brief Requests cancellation of the active OTA search. */
    bool cancel();

    /** @brief Returns a synchronized snapshot of a single-reference lookup. */
    Snapshot snapshot(Type type);
    /** @brief Returns a synchronized summary of a multi-result search. */
    NearbySnapshot nearbySnapshot(Type type);

    /** @brief Copies one retained multi-result entry by index. */
    bool nearbyResult(
        Type type,
        size_t index,
        database::ota::SearchResult &result
    );

    /** @brief Reports whether any OTA lookup task is currently active. */
    bool isBusy();
}
