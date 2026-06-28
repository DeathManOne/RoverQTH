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
#include "screens/menu/battery.h"
#include "screens/menu/general.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"

namespace screens::menu {
    namespace {
        Item currentItem = Item::GENERAL;
    }

    Item current()          { return currentItem; }
    void select(Item item)  { currentItem = item; }
    void preload()          {}

    bool isEditing() {
        switch (currentItem) {
            case Item::GENERAL:
                return screens::menu::general::isEditing();
            case Item::ABOUT:
                return screens::menu::about::isEditing();
            default:
                return false;
        }
    }

    void reset() {
        currentItem = Item::GENERAL;
        screens::menu::general::reset();
        screens::menu::about::reset();
    }

    void draw(ST7796S::MSP4021 &tft) {
        screens::main::title::draw(tft);
        screens::menu::navigation::draw(tft);
        screens::mockup::buttons::draw(tft);

        switch (currentItem) {
            case Item::GENERAL:
                screens::menu::general::draw(tft);
                break;
            case Item::UPDATE:
                screens::menu::updates::draw(tft);
                break;
            case Item::STORAGE:
                screens::menu::storage::draw(tft);
                break;
            case Item::BATTERY:
                screens::menu::battery::draw(tft);
                break;
            case Item::ABOUT:
                screens::menu::about::draw(tft);
                break;
        }
    }

    void update(ST7796S::MSP4021 &tft) {
        switch (currentItem) {
            case Item::BATTERY:
                //screens::menu::battery::update(tft);
                break;
            default:
                break;
        }
    }
}