/*
 * src/display/menu.cpp
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

#include <MSP4021.h>
#include "display/internal.h"
#include "display/menu.h"
#include "screens/menu.h"
#include "screens/menu/navigation.h"
#include "ui/settings/themes/defaults.h"

namespace internal   = display::internal;
namespace menu       = display::menu;
namespace sMenu      = screens::menu;
namespace navigation = screens::menu::navigation;
namespace theme      = ui::settings::themes::defaults;

namespace {
    ST7796S::MSP4021& _tft() { return *internal::TFT; }
}

void menu::preload() { sMenu::reset(); }
void menu::draw()    { sMenu::draw(_tft()); }

void menu::update(uint32_t &nextRefreshIn) {
    sMenu::update(_tft());
    nextRefreshIn = 1000;
}

bool menu::handleTouch(int x, int y) {
    if (sMenu::isEditing()) {
        bool handled = sMenu::handleTouch(_tft(), x, y);
        if (handled && !sMenu::isEditing()) {
            _tft().fillScreen(theme::BLACK);
            draw();
        }
        return true;
    }
    if (sMenu::navigation::handleTouch(x, y)) {
        draw();
        return true;
    }
    return sMenu::handleTouch(_tft(), x, y);
}
