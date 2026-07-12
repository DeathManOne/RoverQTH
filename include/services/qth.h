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

#include <cstddef>
#include <cstdint>

namespace services::qth {
    constexpr uint32_t MIN_DURATION_SECONDS = 5;

    struct QTHPosition {
        uint32_t time;
        char utc[21];
        double latitude;
        double longitude;
        double altitude;
        char locator[15];
    };

    struct QTHDuration {
        uint32_t seconds;
        char hms[16];
    };

    struct QTHDistance {
        uint32_t airLineMeters;
        double airLineKm;
    };

    struct QTHRecord {
        QTHPosition start;
        QTHPosition stop;
        QTHDuration duration;
        QTHDistance distance;
        double minimumAltitude;
        double maximumAltitude;
    };

    bool isCurrentRecordLongEnough();
    bool buildCurrentRecord(QTHRecord& record);
    bool serializeJSONL(const QTHRecord& record, char* buffer, size_t size);
    bool saveCurrentRecord();
}