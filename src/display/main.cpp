/*
 * src/display/main.cpp
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

#include <MSP4021.h>
#include "display/internal.h"
#include "display/main.h"
#include "display/manager.h"
#include "screens/main.h"

namespace internal = display::internal;
namespace main     = display::main;
namespace sMain    = screens::main;

namespace {
    ST7796S::MSP4021& _tft() { return *internal::TFT; }
}

void main::preload    () { sMain::preload(); }
void main::preloadGPS () { sMain::preloadGPS(); }
void main::preloadSOTA() { sMain::preloadSOTA(); }
void main::preloadMARK() { sMain::preloadMARK(); }

void main::draw() { sMain::draw(_tft()); }

void main::update(uint32_t &nextRefreshIn) { sMain::update(_tft(), nextRefreshIn); }
void main::updateMARK() { sMain::updateMARK(_tft()); }
