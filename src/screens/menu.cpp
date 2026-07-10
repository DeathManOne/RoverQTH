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
#include "screens/menu/about.h"
#include "screens/menu/battery.h"
#include "screens/menu/displayer.h"
#include "screens/menu/general.h"
#include "screens/menu/network.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"
#include "services/gps.h"

namespace screens::menu {
    namespace {
        Item currentItem = Item::GENERAL;

        General generalPage;
        Displayer displayerPage;
        Network networkPage;
        Updates updatesPage;
        Storage storagePage;
        Battery batteryPage;
        About aboutPage;

        Page& pageFromItem(Item item) {
            switch (item) {
                case Item::GENERAL:     return generalPage;
                case Item::DISPLAYER:   return displayerPage;
                case Item::NETWORK:     return networkPage;
                case Item::UPDATES:     return updatesPage;
                case Item::STORAGE:     return storagePage;
                case Item::BATTERY:     return batteryPage;
                case Item::ABOUT:       return aboutPage;
                case Item::COUNT:
                default:                return generalPage;
            }
        }
        Page& currentPage() { return pageFromItem(currentItem); }
    }

    bool isEditing() { return currentPage().isEditing(); }
    Item current() { return currentItem; }

    void select(Item item) {
        if (item == Item::COUNT)
            { return; }
        currentItem = item;
    }

    void reset() {
        currentItem = Item::GENERAL;

        generalPage.reset();
        displayerPage.reset();
        networkPage.reset();
        updatesPage.reset();
        storagePage.reset();
        batteryPage.reset();
        aboutPage.reset();
    }

    void draw(ST7796S::MSP4021 &tft) {
        screens::main::title::draw(tft);
        screens::menu::navigation::draw(tft);
        screens::mockup::buttons::draw(tft);
        currentPage().draw(tft);
    }

    void update(ST7796S::MSP4021 &tft) {
        if (isEditing()) { return; }

        char date[16];
        char uptime[16];
        char battery[8];

        services::gps::getDate(date, sizeof(date));
        screens::main::title::getUptime(uptime, sizeof(uptime));
        screens::main::title::getBatteryLevel(battery, sizeof(battery));
        screens::main::title::updateDate(tft, date);
        screens::main::title::updateTime(tft, uptime);
        screens::main::title::updateBattery(tft, battery);

        currentPage().update(tft);
    }

    bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) { return currentPage().handleTouch(tft, x, y); }
}