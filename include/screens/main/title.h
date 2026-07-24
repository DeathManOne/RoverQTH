/*
 * include/screens/main/title.h
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
#include <MSP4021.h>

namespace screens::main::title {
    void getBatteryLevel(char* buffer, size_t size);

    void setCallsign(const char* value);
    void setDate    (const char* value);
    void setTime    (const char* value);
    void setBattery (const char* value);

    void updateCallsign(ST7796S::MSP4021 &tft, const char* value);
    void updateDate    (ST7796S::MSP4021 &tft, const char* value);
    void updateTime    (ST7796S::MSP4021 &tft, const char* value);
    void updateBattery (ST7796S::MSP4021 &tft, const char* value);

    void draw(ST7796S::MSP4021 &tft);
}
