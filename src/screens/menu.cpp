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
#include "screens/menu/wifi.h"
#include "screens/menu/page.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"
#include "services/dtc.h"
#include "ui/mockup/buttons.h"
#include "ui/settings/themes/defaults.h"

using screens::Menu;
using screens::menu::Page;

namespace title      = screens::main::title;
namespace menu       = screens::menu;
namespace navigation = screens::menu::navigation;
namespace dtc        = services::dtc;
namespace buttons    = ui::mockup::buttons;
namespace theme      = ui::settings::themes::defaults;

Page& Menu::_pageFromItem(const menu::Item item) {
    switch (item) {
        case menu::Item::GENERAL:   return _general;
        case menu::Item::DISPLAYER: return _displayer;
        case menu::Item::WIFI:      return _wifi;
        case menu::Item::UPDATES:   return _updates;
        case menu::Item::STORAGE:   return _storage;
        case menu::Item::BATTERY:   return _battery;
        case menu::Item::ABOUT:     return _about;
        case menu::Item::COUNT:
        default:                    return _general;
    }
}

Page& Menu::_currentPage() {
    return _pageFromItem(_currentItem);
}

const Page& Menu::_currentPage() const {
    switch (_currentItem) {
        case menu::Item::GENERAL:   return _general;
        case menu::Item::DISPLAYER: return _displayer;
        case menu::Item::WIFI:      return _wifi;
        case menu::Item::UPDATES:   return _updates;
        case menu::Item::STORAGE:   return _storage;
        case menu::Item::BATTERY:   return _battery;
        case menu::Item::ABOUT:     return _about;
        case menu::Item::COUNT:
        default:                    return _general;
    }
}

bool Menu::isEditing() const {
    return _currentPage().isEditing();
}

menu::Displayer::Request Menu::takeDisplayRequest() {
    return _displayer.takeRequest();
}

void Menu::select(const menu::Item item) {
    if (item == menu::Item::COUNT)
        { return; }
    _currentItem = item;
}

void Menu::reset() {
    _currentItem = menu::Item::GENERAL;

    _general.reset();
    _displayer.reset();
    _wifi.reset();
    _updates.reset();
    _storage.reset();
    _battery.reset();
    _about.reset();
}

void Menu::preload() {
    reset();
}

void Menu::draw(ST7796S::MSP4021 &tft) {
    title::draw(tft);
    navigation::draw(tft, _currentItem);
    buttons::draw(tft);
    _currentPage().draw(tft);
}

void Menu::update(
    ST7796S::MSP4021 &tft,
    uint32_t &nextRefreshIn
) {
    nextRefreshIn = 1000U;

    if (isEditing()) { return; }

    char date[16];
    char time[16];
    char battery[8];

    dtc::getDate(date, sizeof(date));
    dtc::getTime(time, sizeof(time));
    title::getBatteryLevel(battery, sizeof(battery));
    title::updateDate(tft, date);
    title::updateTime(tft, time);
    title::updateBattery(tft, battery);

    _currentPage().update(tft);
}

bool Menu::handleTouch(
    ST7796S::MSP4021 &tft,
    const int x,
    const int y
) {
    if (isEditing()) {
        const bool handled = _currentPage().handleTouch(tft, x, y);

        if (handled && !isEditing()) {
            tft.fillScreen(theme::BLACK);
            draw(tft);
        }

        return true;
    }

    menu::Item selected = _currentItem;

    if (navigation::handleTouch(x, y, selected)) {
        select(selected);
        draw(tft);
        return true;
    }

    const bool handled = _currentPage().handleTouch(tft, x, y);
    if (handled && _currentItem == menu::Item::DISPLAYER) { draw(tft); }
    return handled;
}
