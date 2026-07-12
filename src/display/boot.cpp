/*
 * src/display/boot.cpp
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
#include "display/boot.h"
#include "display/internal.h"
#include "display/manager.h"
#include "screens/boot.h"
#include "ui/images/logo.h"

namespace boot     = display::boot;
namespace internal = display::internal;
namespace sBoot    = screens::boot;

namespace {
    ST7796S::MSP4021& _tft() { return *internal::TFT; }
}

void boot::clear() { sBoot::clear(_tft()); }
void boot::draw () { sBoot::draw(_tft()); }

void boot::drawLogo() { _tft().image(0, 10, ST7796S::logo); }

void boot::updateWifi(bool* value) { sBoot::updateWifi(_tft(), value); }
void boot::updateSD  (bool* value) { sBoot::updateSD(_tft(),   value); }
void boot::updateGPS (bool* value) { sBoot::updateGPS(_tft(),  value); }

void boot::updateGPSProgress(uint8_t progress) { sBoot::updateGPSProgress(_tft(), progress); }
