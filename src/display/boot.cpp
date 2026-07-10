/*
 * display/boot.cpp
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

#include "display/boot.h"
#include "display/internal.h"
#include "display/manager.h"
#include "screens/boot.h"
#include "ui/images/logo.h"

namespace display::boot {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }
    }

    void clear() { screens::boot::clear(tft()); }
    void draw() { screens::boot::draw(tft()); }
    void drawLogo() { tft().image(0, 10, ST7796S::logo); }

    void updateWifi (bool* value) { screens::boot::updateWifi(tft(), value); }
    void updateSD   (bool* value) { screens::boot::updateSD(tft(),   value); }
    void updateGPS  (bool* value) { screens::boot::updateGPS(tft(),  value); }

    void updateGPSProgress(uint8_t progress) { screens::boot::updateGPSProgress(tft(), progress); }
}