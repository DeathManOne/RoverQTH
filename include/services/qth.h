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

    bool isCurrentRecordLongEnough();
    
    bool saveTemporaryRecord();
    bool discardTemporaryRecord();
    RecoveryStatus recoverTemporaryRecord();

    bool discardTemporaryTrace();

    bool appendTracePoint     (const TracePoint& point);
    bool appendFinalTracePoint(const TracePoint& point);
    bool resetTrace();

    bool saveCurrentRecord();

}
