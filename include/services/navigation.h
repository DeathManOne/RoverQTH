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
    /** @brief Lifecycle state of the marked-QTH operation. */
    enum class MarkState : uint8_t {IDLE, RECORDING, READY_TO_SAVE};

    /** @brief Geographic position used by navigation calculations. */
    struct Coordinate {
        double latitude;
        double longitude;
        double altitude;
    };

    /** @brief Timestamped coordinate queued for trace persistence. */
    struct TracePoint {
        Coordinate coordinate;
        uint32_t utc;
    };

    /** @brief Complete synchronized state of the active marked-QTH operation. */
    struct MarkSnapshot {
        Coordinate start;
        Coordinate end;
        uint32_t startUTC;
        uint32_t stopUTC;
        uint32_t startedAtMillis;
        uint32_t stoppedAtMillis;
        bool hasEnd;
    };

    /** @brief Reduced marked-QTH state prepared for screen rendering. */
    struct MarkDisplaySnapshot {
        MarkState state         = MarkState::IDLE;
        Coordinate start        {};
        uint32_t elapsedSeconds = 0U;
        double distanceKm       = -1.0;
        double bearingDeg       = -1.0;
    };

    /** @brief Initializes the navigation service. */
    void begin();

    /**
     * @brief Updates the current GPS fix.
     *
     * A fix reported as valid is rejected when its latitude, longitude
     * or altitude is non-finite, or when its coordinates are outside
     * their geographic ranges.
     *
     * @param coordinate Current geographic coordinate.
     * @param fixValid Whether the receiver reports a valid position fix.
     */
    void updateGPSFix(const Coordinate &coordinate, bool fixValid);

    /**
     * @brief Retrieves the next pending trace point without removing it.
     * @param point Receives the pending trace point.
     * @return `true` when a trace point is available.
     */
    bool peekPendingTracePoint(TracePoint &point);

    /**
     * @brief Removes the current pending trace point.
     * @return `true` when a trace point was discarded.
     */
    bool discardPendingTracePoint();

    /**
     * @brief Starts a new marked-QTH operation.
     * @return `true` when the mark was started successfully.
     */
    bool startMark();

    /**
     * @brief Restores a previously started marked-QTH operation.
     * @param start Starting coordinate of the restored mark.
     * @param startUTC UTC timestamp of the mark start.
     * @return `true` when the mark was restored successfully.
     */
    bool restoreMark(const Coordinate &start, uint32_t startUTC);

    /**
     * @brief Stops the current marked-QTH operation.
     * @return `true` when the mark was stopped successfully.
     */
    bool stopMark();

    /** @brief Clears the current mark and its pending trace points. */
    void clearMark();

    /**
     * @brief Returns the current marked-QTH lifecycle state.
     * @return Current mark state.
     */
    MarkState markState();

    /**
     * @brief Returns the duration of the completed mark.
     * @return Mark duration in seconds, or zero while idle or recording.
     */
    uint32_t markDurationSeconds();

    /**
     * @brief Retrieves a complete snapshot of the current mark.
     * @param snapshot Receives the synchronized mark snapshot.
     * @return `true` when a mark is active or waiting to be saved.
     */
    bool getMarkSnapshot(MarkSnapshot &snapshot);

    /**
     * @brief Retrieves the information required to display the current mark.
     * @param snapshot Receives the prepared display snapshot.
     * @return `true` when a mark is active or waiting to be saved.
     */
    bool getMarkDisplaySnapshot(MarkDisplaySnapshot &snapshot);
}
