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
#include "screens/menu/about.h"
#include "screens/menu/battery.h"
#include "screens/menu/general.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"
#include "screens/menu/navigation.h"
#include "ui/settings/themes/defaults.h"

namespace display::menu {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }

        bool getHandled(int x, int y) {
            switch (screens::menu::current()) {
                case screens::menu::Item::GENERAL:
                    return screens::menu::general::handleTouch(tft(), x, y);
                case screens::menu::Item::UPDATE:
                    return screens::menu::updates::handleTouch(tft(), x, y);
                case screens::menu::Item::STORAGE:
                    return screens::menu::storage::handleTouch(tft(), x, y);
                case screens::menu::Item::BATTERY:
                    return screens::menu::battery::handleTouch(tft(), x, y);
                case screens::menu::Item::ABOUT:
                    return screens::menu::about::handleTouch(tft(), x, y);
                default:
                    return false;
            }
        }
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
        if (screens::menu::isEditing()) {
            bool handled = getHandled(x, y);
            if (handled && !screens::menu::isEditing()) {
                tft().fillScreen(ui::settings::themes::defaults::BLACK);
                draw();
            }
            return true;
        }
        if (screens::menu::navigation::handleTouch(x, y)) {
            draw();
            return true;
        }
        return getHandled(x, y);
    }
}