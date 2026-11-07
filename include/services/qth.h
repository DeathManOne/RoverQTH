/*
 * include/services/qth.h
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

#include <cstdint>

namespace services::qth {
    /** @brief Outcome of recovering an interrupted QTH record. */
    enum class RecoveryStatus : uint8_t {NONE, RECORDING_RESTORED, RECORD_FINALIZED, ERROR};

    /** @brief Timestamped geographic point persisted in a QTH trace. */
    struct TracePoint {
        uint32_t utc     = 0U;
        double latitude  = 0.0;
        double longitude = 0.0;
        double altitude  = 0.0;
    };

    /**
     * Indicates whether the current record is long enough to be saved.
     * @return `true` when the current record is long enough, otherwise `false`.
     */
    bool isCurrentRecordLongEnough();

    /**
     * Saves the current record as a temporary record.
     * @return `true` when the temporary record was successfully saved, otherwise `false`.
     */
    bool saveTemporaryRecord();

    /**
     * Discards the current temporary record.
     * @return `true` when the temporary record was successfully discarded, otherwise `false`.
     */
    bool discardTemporaryRecord();

    /**
     * Recovers a previously saved temporary record.
     * @return Recovery result.
     */
    RecoveryStatus recoverTemporaryRecord();

    /**
     * Discards the current temporary trace.
     * @return `true` when the temporary trace was successfully discarded, otherwise `false`.
     */
    bool discardTemporaryTrace();

    /**
     * @brief Processes a point for the active trace.
     * A valid point is written only if it passes the timestamp, distance, and position-change filters.
     * @param point Trace point to process.
     * @return `true` when the point was accepted, whether written or filtered out;
     *         `false` when the point is invalid or the write failed.
     */
    bool appendTracePoint(const TracePoint &point);

    /**
     * Appends the final trace point to the current trace.
     * @param point Final trace point to append.
     * @return `true` when the final point was accepted, written, or already present;
     *         `false` when it is invalid, older than the last point, or cannot be written.
     */
    bool appendFinalTracePoint(const TracePoint &point);

    /**
     * Clears the current trace.
     * @return `true` when the trace was successfully cleared, otherwise `false`.
     */
    bool resetTrace();

    /**
     * Saves the current QTH record.
     * @return `true` when the record was successfully saved, otherwise `false`.
     */
    bool saveCurrentRecord();
}
