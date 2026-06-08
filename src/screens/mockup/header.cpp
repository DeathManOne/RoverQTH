/*
 * screens/mockup/header.cpp
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

#include "screens/mockup/header.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"

namespace screens::mockup::header {
    namespace {
        struct Area {
            int outerX = 0;
            int outerY = 0;
            int outerW = 0;
            int outerH = 0;

            int innerX = 0;
            int innerY = 0;
            int innerW = 0;
            int innerH = 0;
        };
        Area area;
    }

    void clear(ST7796S::MSP4021 &tft) {
        tft.rectFill(
            outerX(),       outerY(),
            outerWidth(),   outerHeight(),
            ui::settings::themes::defaults::BLACK
        );
    }

    void draw(ST7796S::MSP4021 &tft) {
        const int gap = ui::settings::mockup::GAP;

        area.outerX = 0;
        area.outerY = 0;
        area.outerW = tft.width();
        area.outerH = ui::settings::mockup::HEADER_HEIGHT;

        area.innerX = area.outerX + gap;
        area.innerY = area.outerY + gap;
        area.innerW = area.outerW - (gap * 2);
        area.innerH = area.outerH - (gap * 2);

        tft.rectFill(
            area.outerX, area.outerY,
            area.outerW, area.outerH,
            ui::settings::themes::defaults::BLACK
        );
        tft.rectRound(
            area.outerX, area.outerY,
            area.outerW, area.outerH,
            ui::settings::mockup::RADIUS,
            ui::settings::themes::defaults::BORDER
        );
    }

    int outerX()        { return area.outerX; }
    int outerY()        { return area.outerY; }
    int outerWidth()    { return area.outerW; }
    int outerHeight()   { return area.outerH; }

    int innerX()        { return area.innerX; }
    int innerY()        { return area.innerY; }
    int innerWidth()    { return area.innerW; }
    int innerHeight()   { return area.innerH; }
}