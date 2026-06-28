/*
 * screens/menu/about.cpp
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

#include "screens/menu/about.h"
#include "screens/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace screens::menu::about {
    namespace {
        void nextRow(int &rowY, int rowH) { rowY += rowH; }
        void drawLine(ST7796S::MSP4021 &tft, int x, int y, int w, int h, const char* label, const char* value) {
            const int gap = ui::settings::mockup::GAP;

            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textCenterLeft(x, y, w / 2, h, label);

            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textCenterRight(x + (w / 2), y, (w / 2) - gap, h, value);
        }
    }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX() + (gap * 2);
        const int y     = screens::mockup::grid::innerY() + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth() - (gap * 4);
        const int rowH  = 28;

        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(ui::settings::themes::defaults::GREEN);
        tft.textCenterLeft(x, y, w, rowH, "ABOUT");

        tft.lineH(
            x, y + rowH + gap,
            w, ui::settings::themes::defaults::BORDER
        );

        int rowY = y + rowH + (gap * 3);

        drawLine(tft, x, rowY, w, rowH, "Name", PROJECT_NAME);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Version", PROJECT_VERSION);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Author", PROJECT_AUTHOR);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "License", PROJECT_LICENSE);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Link", "");
        nextRow(rowY, rowH);
        drawLine(tft, x, rowY, w, rowH, "", PROJECT_URL);
    }
}