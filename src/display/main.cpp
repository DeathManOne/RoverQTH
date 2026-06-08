/*
 * display/main.cpp
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

#include "display/internal.h"
#include "display/main.h"
#include "display/manager.h"
#include "screens/main.h"

namespace display::main {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }
    }

    void preload()      { screens::main::preload(); }
    void preloadGPS()   { screens::main::preloadGPS(); }
    void preloadSOTA()  { screens::main::preloadSOTA(); }
    void preloadMARK()  { screens::main::preloadMARK(); }

    void draw() { screens::main::draw(tft()); }

    void update(uint32_t &nextRefreshIn) { screens::main::update(tft(), nextRefreshIn); }
    void updateMARK() { screens::main::updateMARK(tft()); }
}