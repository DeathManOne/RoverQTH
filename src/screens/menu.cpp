/*
 * screens/menu.cpp
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

#include "screens/main/title.h"
#include "screens/menu.h"
#include "screens/menu/navigation.h"
#include "screens/mockup/buttons.h"
#include "screens/mockup/grid.h"

namespace screens::menu {
    namespace {
        Item currentItem = Item::FIRMWARE;
    }

    Item current() {
        return currentItem;
    }

    void select(Item item) {
        currentItem = item;
    }

    void preload() {
    }

    void draw(ST7796S::MSP4021 &tft) {
        screens::main::title::draw(tft);
        screens::mockup::grid::draw(tft);
        screens::menu::navigation::draw(tft);
        screens::mockup::buttons::draw(tft);
    }

    void update(ST7796S::MSP4021 &tft) {
    }
}