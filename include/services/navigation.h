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
    struct Coordinate {
        double latitude;
        double longitude;
        double altitude;
    };

    struct SOTATarget {
        Coordinate coordinate;
        char code[24];
        int points;
        int altitude;
    };

    enum class MarkState {
        IDLE,
        RECORDING,
        READY_TO_SAVE
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

    bool startMark();
    bool stopMark();
    bool isMarkRecording();
    MarkState markState();

    uint32_t markElapsedSeconds();
    uint32_t markDurationSeconds();

    Coordinate markStartPosition();
    Coordinate markEndPosition();

    const MarkSnapshot& markSnapshot();

    double markTotalDistanceKm();
    double markCurrentDistanceKm();
    double markCurrentBearingDeg();

    void getMarkStartLocator(char* buffer, size_t size);
    void getMarkEndLocator  (char* buffer, size_t size);

    void updateGPSFix(bool fixValid);
    bool hasCurrentPosition();
    Coordinate currentPosition();

    double distanceKm(const Coordinate &from, const Coordinate &to);
    double bearingDeg(const Coordinate &from, const Coordinate &to);

    void setMark(double latitude, double longitude);
    bool setMarkFromCurrentPosition();
    void clearMark();
    bool hasMark();

    Coordinate markPosition();
    double markDistanceKm();
    double markBearingDeg();

    void formatDistance(const double km, char* const buffer, const size_t size);
    void formatBearing (double deg, char* buffer, size_t size);

    void setSOTA(const char* code, double latitude, double longitude, int points, int altitude);

    void clearSOTA();
    bool hasSOTA();
    double sotaDistanceKm();
    double sotaBearingDeg();
    void getSOTACode(char* buffer, size_t size);
    int getSOTAPoints();
    int getSOTAAltitude();
}
