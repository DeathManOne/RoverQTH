/*
 * src/display/mockup.cpp
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
#include "display/mockup.h"
#include "display/manager.h"
#include "ui/mockup/buttons.h"
#include "ui/mockup/grid.h"
#include "ui/mockup/header.h"
#include "ui/mockup/right.h"

namespace internal = display::internal;
namespace mockup   = display::mockup;
namespace buttons  = ui::mockup::buttons;
namespace grid     = ui::mockup::grid;
namespace header   = ui::mockup::header;
namespace right    = ui::mockup::right;

namespace {
    ST7796S::MSP4021& _tft() { return *display::internal::TFT; }
}

void mockup::clearHeader() { header::clear(_tft()); }
void mockup::drawHeader () { header::draw(_tft()); }

void mockup::clearGrid() { grid::clear(_tft()); }
void mockup::drawGrid () { grid::draw(_tft()); }

void mockup::clearRight() { right::clear(_tft()); }
void mockup::drawRight () { right::draw(_tft()); }

void mockup::clearButtons() { buttons::clear(_tft()); }
void mockup::drawButtons () { buttons::draw(_tft()); }

void mockup::updateMARK() { buttons::updateMARK(_tft()); }
void mockup::updateSOTA() { buttons::updateSOTA(_tft()); }
void mockup::updateMENU() { buttons::updateMENU(_tft()); }
