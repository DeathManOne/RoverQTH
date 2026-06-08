/*
 * display/mockup.cpp
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
#include "display/mockup.h"
#include "display/manager.h"
#include "screens/mockup/buttons.h"
#include "screens/mockup/grid.h"
#include "screens/mockup/header.h"
#include "screens/mockup/right.h"

namespace display::mockup {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }
    }

    void clearHeader() { screens::mockup::header::clear(tft()); }
    void drawHeader()  { screens::mockup::header::draw(tft()); }

    void clearGrid() { screens::mockup::grid::clear(tft()); }
    void drawGrid()  { screens::mockup::grid::draw(tft()); }

    void clearRight() { screens::mockup::right::clear(tft()); }
    void drawRight()  { screens::mockup::right::draw(tft()); }

    void clearButtons() { screens::mockup::buttons::clear(tft()); }
    void drawButtons()  { screens::mockup::buttons::draw(tft()); }

    void updateMARK() { screens::mockup::buttons::updateMARK(tft()); }
    void updateSOTA() { screens::mockup::buttons::updateSOTA(tft()); }
    void updateMENU() { screens::mockup::buttons::updateMENU(tft()); }
}