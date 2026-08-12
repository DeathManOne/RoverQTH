/*
 * src/screens/sota.cpp
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
#include "screens/sota.h"
#include "services/dtc.h"
#include "ui/mockup/buttons.h"
#include "ui/mockup/grid.h"
#include "ui/mockup/right.h"
#include "ui/settings/themes/defaults.h"

namespace title   = screens::main::title;
namespace sota    = screens::sota;
namespace dtc     = services::dtc;
namespace buttons = ui::mockup::buttons;
namespace grid    = ui::mockup::grid;
namespace right   = ui::mockup::right;
namespace theme   = ui::settings::themes::defaults;

void sota::preload() {}

void sota::draw(ST7796S::MSP4021& tft) {
    tft.fillScreen(theme::BLACK);

    title::draw(tft);
    grid::draw(tft);
    right::draw(tft);
    buttons::draw(tft);
}

void sota::update(ST7796S::MSP4021& tft, uint32_t& nextRefreshIn) {
    char date[16];
    char time[16];
    char battery[8];

    dtc::getDate(date, sizeof(date));
    dtc::getTime(time, sizeof(time));
    title::getBatteryLevel(battery, sizeof(battery));

    title::updateDate(tft, date);
    title::updateTime(tft, time);
    title::updateBattery(tft, battery);

    nextRefreshIn = 1000;
}
