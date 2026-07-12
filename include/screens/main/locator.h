/*
 * include/screens/main/locator.h
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

namespace screens::main::locator {
    void setLocator     (const char* value);
    void setStatusTop   (const char* value);
    void setStatusBottom(const char* value);
    void setSOTACode    (const char* value);
    void setSOTABearing (const char* value);
    void setSOTADistance(const char* value);
    void setSOTAPoints  (const char* value);
    void setSOTAAltitude(const char* value);
    void setMarkLocator (const char* value);
    void setMarkBearing (const char* value);
    void setMarkDistance(const char* value);
    void setMarkTimer   (const char* value);

    void updateLocator      (ST7796S::MSP4021 &tft, const char* value);
    void updateStatusTop    (ST7796S::MSP4021 &tft, const char* value);
    void updateStatusBottom (ST7796S::MSP4021 &tft, const char* value);
    void updateSOTACode     (ST7796S::MSP4021 &tft, const char* value);
    void updateSOTABearing  (ST7796S::MSP4021 &tft, const char* value);
    void updateSOTADistance (ST7796S::MSP4021 &tft, const char* value);
    void updateSOTAPoints   (ST7796S::MSP4021 &tft, const char* value);
    void updateSOTAAltitude (ST7796S::MSP4021 &tft, const char* value);
    void updateMarkLocator  (ST7796S::MSP4021 &tft, const char* value);
    void updateMarkBearing  (ST7796S::MSP4021 &tft, const char* value);
    void updateMarkDistance (ST7796S::MSP4021 &tft, const char* value);
    void updateMarkTimer    (ST7796S::MSP4021 &tft, const char* value);

    void draw(ST7796S::MSP4021 &tft);
}
