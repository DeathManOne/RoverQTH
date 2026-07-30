/*
 * include/services/gps.h
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

#include <HardwareSerial.h>
#include <cstdint>

namespace services::gps {
    struct Snapshot {
        double latitude;
        double longitude;
        double altitude;

        double heading;
        double speed;

        double hdop;
        double pdop;

        uint8_t fixType;
        uint8_t satellites;

        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        bool positionValid;
        bool fixValid;
        bool timeValid;
    };

    bool begin  (HardwareSerial& uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout = 10);
    bool restart(HardwareSerial& uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout);

    bool update(uint32_t timeoutMs = 10);
    bool getSnapshot(Snapshot& snapshot);

    uint8_t getAcquisitionProgress();
}
