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
#include "screens/menu/about.h"

namespace screens::menu {
    namespace {
        Item currentItem = Item::GENERAL;
    }

    Item current()          { return currentItem; }
    void select(Item item)  { currentItem = item; }
    void reset()            { currentItem = Item::GENERAL; }
    void preload()          {}

    void draw(ST7796S::MSP4021 &tft) {
        screens::main::title::draw(tft);
        screens::menu::navigation::draw(tft);
        screens::mockup::buttons::draw(tft);

        switch (currentItem) {
            case Item::GENERAL:
                screens::mockup::grid::draw(tft);
                break;
            case Item::UPDATE:
                screens::mockup::grid::draw(tft);
                break;
            case Item::STORAGE:
                screens::mockup::grid::draw(tft);
                break;
            case Item::BATTERY:
                screens::mockup::grid::draw(tft);
                break;
            case Item::ABOUT:
                screens::menu::about::draw(tft);
                break;
        }
    }

    void update(ST7796S::MSP4021 &tft) {
        (void)tft;
        switch (currentItem) {
            case Item::BATTERY:
                screens::mockup::grid::draw(tft);
                break;
            default:
                break;
        }
    }
}