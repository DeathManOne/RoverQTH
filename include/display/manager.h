/*
 * include/display/manager.h
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
#include "services/settings.h"

namespace display {
    void begin(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t touchCS, uint8_t screenCS, uint8_t screenDC, uint8_t screenRST, uint16_t width, uint16_t height);

    bool TLoad();
    bool TCalibrate();
    void TCalibrate(services::settings::Calibration &calibration);
    void TCalibrateInfo(services::settings::Calibration &calibration);

    bool TRead(int &x, int &y);
    void clearScreen();
}