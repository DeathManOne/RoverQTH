/*
 * src/display/sota.cpp
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
#include "display/sota.h"
#include "screens/sota.h"

namespace internal = display::internal;
namespace sota     = display::sota;
namespace sSota    = screens::sota;

namespace {
    ST7796S::MSP4021& _tft() { return *internal::TFT; }
}

void sota::preload() { sSota::preload(); }

void sota::draw() { sSota::draw(_tft()); }

void sota::update(uint32_t& nextRefreshIn) {
    sSota::update(_tft(), nextRefreshIn);
}

bool sota::handleTouch(const int x, const int y) {
    return sSota::handleTouch(_tft(), x, y);
}
