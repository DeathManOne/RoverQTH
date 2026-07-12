/*
 * src/screens/menu.cpp
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
#include "screens/menu/about.h"
#include "screens/menu/battery.h"
#include "screens/menu/displayer.h"
#include "screens/menu/general.h"
#include "screens/menu/navigation.h"
#include "screens/menu/network.h"
#include "screens/menu/page.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"
#include "services/gps.h"
#include "ui/mockup/buttons.h"

using screens::menu::About;
using screens::menu::Battery;
using screens::menu::Displayer;
using screens::menu::General;
using screens::menu::Network;
using screens::menu::Page;
using screens::menu::Storage;
using screens::menu::Updates;

namespace title      = screens::main::title;
namespace menu       = screens::menu;
namespace navigation = screens::menu::navigation;
namespace gps        = services::gps;
namespace buttons    = ui::mockup::buttons;

namespace {
    menu::Item _currentItem = menu::Item::GENERAL;

    General _general;
    Displayer _displayer;
    Network _network;
    Updates _updates;
    Storage _storage;
    Battery _battery;
    About _about;

    Page& _pageFromItem(menu::Item item) {
        switch (item) {
            case menu::Item::GENERAL:   return _general;
            case menu::Item::DISPLAYER: return _displayer;
            case menu::Item::NETWORK:   return _network;
            case menu::Item::UPDATES:   return _updates;
            case menu::Item::STORAGE:   return _storage;
            case menu::Item::BATTERY:   return _battery;
            case menu::Item::ABOUT:     return _about;
            case menu::Item::COUNT:
            default:                    return _general;
        }
    }
    Page& _currentPage() { return _pageFromItem(_currentItem); }
}

bool menu::isEditing    () { return _currentPage().isEditing(); }
menu::Item menu::current() { return _currentItem; }

void menu::select(Item item) {
    if (item == Item::COUNT)
        { return; }
    _currentItem = item;
}

void menu::reset() {
    _currentItem = Item::GENERAL;

    _general.reset();
    _displayer.reset();
    _network.reset();
    _updates.reset();
    _storage.reset();
    _battery.reset();
    _about.reset();
}

void menu::draw(ST7796S::MSP4021 &tft) {
    title::draw(tft);
    navigation::draw(tft);
    buttons::draw(tft);
    _currentPage().draw(tft);
}

void menu::update(ST7796S::MSP4021 &tft) {
    if (isEditing()) { return; }

    char date[16];
    char uptime[16];
    char battery[8];

    gps::getDate(date, sizeof(date));
    title::getUptime(uptime, sizeof(uptime));
    title::getBatteryLevel(battery, sizeof(battery));
    title::updateDate(tft, date);
    title::updateTime(tft, uptime);
    title::updateBattery(tft, battery);

    _currentPage().update(tft);
}

bool menu::handleTouch(ST7796S::MSP4021 &tft, int x, int y) { return _currentPage().handleTouch(tft, x, y); }
