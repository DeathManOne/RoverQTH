/*
 * include/services/navigation.h
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

namespace services::navigation {
    enum class MarkState : uint8_t {IDLE, RECORDING, READY_TO_SAVE};
    struct Coordinate {
        double latitude;
        double longitude;
        double altitude;
    };

    struct MarkSnapshot {
        Coordinate start;
        Coordinate end;
        uint32_t startUTC;
        uint32_t stopUTC;
        uint32_t startedAtMillis;
        uint32_t stoppedAtMillis;
        double minAltitude;
        double maxAltitude;
        bool hasEnd;
    };

    void begin();
    void updateGPSFix(const Coordinate& coordinate, bool fixValid);

    bool startMark();
    bool stopMark();
    void clearMark();
    bool hasMark();
    MarkState markState();

    uint32_t markElapsedSeconds();
    uint32_t markDurationSeconds();

    bool getMarkSnapshot(MarkSnapshot& snapshot);

    double markTotalDistanceKm();
    double markCurrentDistanceKm();
    double markCurrentBearingDeg();

    void getMarkStartLocator(char* buffer, size_t size);
}
