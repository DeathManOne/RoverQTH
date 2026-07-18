/*
 * src/screens/menu/navigation.cpp
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

#include "screens/menu.h"
#include "screens/menu/navigation.h"
#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"
#include "ui/mockup/right.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

namespace menu       = screens::menu;
namespace navigation = screens::menu::navigation;
namespace right      = ui::mockup::right;
namespace uiMockup   = ui::settings::mockup;
namespace theme      = ui::settings::themes::defaults;

namespace {
    struct Row {
        menu::Item item;
        const char* label;
        int outerX = 0;
        int outerY = 0;
        int outerW = 0;
        int outerH = 0;
        int innerX = 0;
        int innerY = 0;
        int innerW = 0;
        int innerH = 0;
    };

    Row _makeRow(menu::Item item, const char* label);
    bool _isPressed(const Row &row, int tx, int ty);

    Row rows[] = {
        _makeRow(menu::Item::GENERAL,   "General"),
        _makeRow(menu::Item::DISPLAYER, "Display"),
        _makeRow(menu::Item::WIFI,      "WiFi"),
        _makeRow(menu::Item::UPDATES,   "Updates"),
        _makeRow(menu::Item::STORAGE,   "Storage"),
        _makeRow(menu::Item::BATTERY,   "Battery"),
        _makeRow(menu::Item::ABOUT,     "About")
    };

    constexpr size_t ROW_COUNT = static_cast<size_t>(menu::Item::COUNT);
    static_assert(
        ROW_COUNT == sizeof(rows) / sizeof(rows[0]),
        "Menu navigation rows mismatch"
    );

    Row _makeRow(menu::Item item, const char* label) {
        Row row;
        row.item   = item;
        row.label  = label;
        row.outerX = 0;
        row.outerY = 0;
        row.outerW = 0;
        row.outerH = 0;
        row.innerX = 0;
        row.innerY = 0;
        row.innerW = 0;
        row.innerH = 0;
        return row;
    }

    bool _isPressed(const Row &row, int tx, int ty) {
        return (
            tx >= row.innerX && tx < (row.innerX + row.innerW) &&
            ty >= row.innerY && ty < (row.innerY + row.innerH)
        );
    }
}

void navigation::draw(ST7796S::MSP4021 &tft) {
    right::draw(tft);

    const int gap       = uiMockup::GAP;
    const int outerX    = right::innerX();
    const int outerY    = right::innerY();
    const int outerW    = right::innerWidth();
    const int outerH    = right::innerHeight();
    const int rowH      = outerH / ROW_COUNT;

    const auto current = menu::current();

    for (size_t i = 0; i < ROW_COUNT; i++) {
        Row &row = rows[i];

        row.outerX = outerX;
        row.outerY = outerY + (rowH * i);
        row.outerW = outerW;
        row.outerH = rowH;

        row.innerX = row.outerX + gap;
        row.innerY = row.outerY + gap;
        row.innerW = row.outerW - (gap * 2);
        row.innerH = row.outerH - (gap * 2);

        const bool selected = (row.item == current);
        if (selected) {
            tft.rectFill(
                row.innerX, row.innerY,
                row.innerW, row.innerH,
                theme::GREEN
            );
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(theme::BLACK);
        } else {
            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(theme::WHITE);
        }

        tft.textCenter(
            row.innerX, row.innerY,
            row.innerW, row.innerH,
            row.label
        );

        if (i > 0) {
            tft.lineH(
                row.innerX, row.outerY,
                row.innerW, theme::BORDER);
        }
    }
}

bool navigation::handleTouch(int x, int y) {
    for (size_t i = 0; i < ROW_COUNT; i++) {
        if (!_isPressed(rows[i], x, y)) { continue; }
        menu::select(rows[i].item);
        return true;
    }
    return false;
}
