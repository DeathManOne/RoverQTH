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
    enum class RecoveryStatus : uint8_t {NONE, RECORDING_RESTORED, RECORD_FINALIZED, ERROR};

    struct TracePoint {
        uint32_t utc     = 0U;
        double latitude  = 0.0;
        double longitude = 0.0;
        double altitude  = 0.0;
    };

    /**
     * Indicates whether the current record is long enough to be saved.
     * @return true if the current record is long enough, false otherwise.
     */
    bool isCurrentRecordLongEnough();

    /**
     * Saves the current record as a temporary record.
     * @return true if the temporary record was successfully saved, false otherwise.
     */
    bool saveTemporaryRecord();

    /**
     * Discards the current temporary record.
     * @return true if the temporary record was successfully discarded, false otherwise.
     */
    bool discardTemporaryRecord();

    /**
     * Recovers a previously saved temporary record.
     * @return Recovery result.
     */
    RecoveryStatus recoverTemporaryRecord();

    /**
     * Discards the current temporary trace.
     * @return true if the temporary trace was successfully discarded, false otherwise.
     */
    bool discardTemporaryTrace();

    /**
     * Appends a trace point to the current trace.
     * @param point Trace point to append.
     * @return true if the trace point was successfully appended, false otherwise.
     */
    bool appendTracePoint(const TracePoint& point);

    /**
     * Appends the final trace point to the current trace.
     * @param point Final trace point to append.
     * @return true if the trace point was successfully appended, false otherwise.
     */
    bool appendFinalTracePoint(const TracePoint& point);

    /**
     * Clears the current trace.
     * @return true if the trace was successfully cleared, false otherwise.
     */
    bool resetTrace();

    /**
     * Saves the current QTH record.
     * @return true if the record was successfully saved, false otherwise.
     */
    bool saveCurrentRecord();
}
