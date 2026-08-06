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

    struct TracePoint {
        Coordinate coordinate;
        uint32_t utc;
    };

    struct MarkSnapshot {
        Coordinate start;
        Coordinate end;
        uint32_t startUTC;
        uint32_t stopUTC;
        uint32_t startedAtMillis;
        uint32_t stoppedAtMillis;
        bool hasEnd;
    };

    struct MarkDisplaySnapshot {
        MarkState state         = MarkState::IDLE;
        Coordinate start        {};
        uint32_t elapsedSeconds = 0U;
        double distanceKm       = -1.0;
        double bearingDeg       = -1.0;
    };

    /**
     * Initializes the navigation service.
     */
    void begin();

   /**
     * @brief Updates the current GPS fix.
     * A fix marked as valid is ignored if either coordinate is non-finite
     * or outside its geographic range.
     * @param coordinate Current coordinate. Its latitude must be from -90 to 90 and its longitude from -180 to 180.
     * @param fixValid Whether the receiver reports a valid fix.
     */
    void updateGPSFix(const Coordinate& coordinate, bool fixValid);

    /**
     * Retrieves the next pending trace point without removing it.
     * @param point Receives the pending trace point.
     * @return true if a trace point is available, false otherwise.
     */
    bool peekPendingTracePoint(TracePoint& point);

    /**
     * Removes the current pending trace point.
     * @return true if a trace point was discarded, false otherwise.
     */
    bool discardPendingTracePoint();

    /**
     * Starts a new mark.
     * @return true if the mark was successfully started, false otherwise.
     */
    bool startMark();

    /**
     * Restores a previously started mark.
     * @param start    Starting coordinate.
     * @param startUTC UTC timestamp of the mark start.
     * @return true if the mark was restored, false if its timestamps, coordinates, or state are invalid.
     */
    bool restoreMark(const Coordinate& start, uint32_t startUTC);

    /**
     * Stops the current mark.
     * @return true if the mark was successfully stopped, false otherwise.
     */
    bool stopMark();

    /**
     * Clears the current mark.
     */
    void clearMark();

    /**
     * Returns the current mark state.
     * @return Current mark state.
     */
    MarkState markState();

    /**
     * @brief Returns the duration of the completed mark.
     * @return Mark duration in seconds, or zero while idle or recording.
     */
    uint32_t markDurationSeconds();

    /**
     * Retrieves a complete snapshot of the current mark.
     * @param snapshot Receives the mark snapshot.
     * @return true if a snapshot is available, false otherwise.
     */
    bool getMarkSnapshot(MarkSnapshot& snapshot);

    /**
     * Retrieves the information required to display the current mark.
     * @param snapshot Receives the display snapshot.
     * @return true if a display snapshot is available, false otherwise.
     */
    bool getMarkDisplaySnapshot(MarkDisplaySnapshot& snapshot);
}
