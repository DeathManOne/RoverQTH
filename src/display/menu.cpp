/*
 * display/menu.cpp
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
#include "display/menu.h"
#include "screens/menu.h"
#include "screens/menu/navigation.h"

namespace display::menu {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }
    }

    void preload() {
    }

    void draw() {
        screens::menu::preload();
        screens::menu::draw(tft());
    }

    void update(uint32_t &nextRefreshIn) {
        screens::menu::update(tft());
        nextRefreshIn = 1000;
    }

    bool handleTouch(int x, int y) {
        if (!screens::menu::navigation::handleTouch(x, y))
            { return false; }
        draw();
        return true;
    }
}