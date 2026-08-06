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

    /**
     * Initializes the GPS receiver.
     * @param uart      Hardware serial interface.
     * @param rx        UART RX pin.
     * @param tx        UART TX pin.
     * @param finalBaud Target baud rate.
     * @param timeout Initialization timeout in seconds.
     * @return true if the GPS was successfully initialized, false otherwise.
     */
    bool begin  (HardwareSerial& uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout = 10);

    /**
     * Restarts the GPS receiver.
     * @param uart      Hardware serial interface.
     * @param rx        UART RX pin.
     * @param tx        UART TX pin.
     * @param finalBaud Target baud rate.
     * @param timeout Restart timeout in seconds.
     * @return true if the GPS was successfully restarted, false otherwise.
     */
    bool restart(HardwareSerial& uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout);

    /**
     * @brief Reads and processes new GPS data.
     * @param timeoutMs Maximum receiver polling time in milliseconds.
     * @return true if a snapshot with a valid GNSS fix was processed, false otherwise.
     */
    bool update(uint32_t timeoutMs = 10);

    /**
     * Retrieves the latest GPS data snapshot.
     * @param snapshot Receives the current GPS data.
     * @return true if receiver data has been cached, false otherwise.
     */
    bool getSnapshot(Snapshot& snapshot);

    /**
     * Returns the current GPS acquisition progress.
     * @return Acquisition progress as a percentage.
     */
    uint8_t getAcquisitionProgress();
}
