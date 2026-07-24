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
#include <cstddef>
#include <cstdint>

namespace services::gps {
    bool begin  (HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout = 10);
    bool restart(HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout);
    bool update (uint32_t timeoutMs = 10);
    bool poll();

    void getTime(char* buffer, size_t size, bool withSecond = true);
    uint8_t getAcquisitionProgress();
    const char* headingToCardinal(const double heading);

    double getAltitude();
    void getSat(int &fix, int &count);
    void getPrecision(double &MASL, double &HDG, double &speed);
    void getDOP(double &HDOP, double &VDOP, double &PDOP);
    void getPosition(double &latitude, double &longitude);
}
