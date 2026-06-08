/*
 * screens/main/datas.h
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

namespace screens::main::datas {
    void setLatitude    (const char* value);
    void setLongitude   (const char* value);
    void setSpeed       (const char* value);
    void setHeading     (const char* value);
    void setASL         (const char* value);
    void setUpdate      (const char* value);

    void updateLatitude (ST7796S::MSP4021 &tft, const char* value);
    void updateLongitude(ST7796S::MSP4021 &tft, const char* value);
    void updateSpeed    (ST7796S::MSP4021 &tft, const char* value);
    void updateHeading  (ST7796S::MSP4021 &tft, const char* value);
    void updateASL      (ST7796S::MSP4021 &tft, const char* value);
    void updateUpdate   (ST7796S::MSP4021 &tft, const char* value);

    void draw(ST7796S::MSP4021 &tft);
}
