/*
 * src/ui/mockup/header.cpp
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

#include "ui/mockup/header.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

namespace header   = ui::mockup::header;
namespace uiMockup = ui::settings::mockup;
namespace theme    = ui::settings::themes::defaults;

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
    Area _area;
}

void header::clear(ST7796S::MSP4021 &tft) {
    tft.rectFill(
        outerX(),     outerY(),
        outerWidth(), outerHeight(),
        theme::BLACK
    );
}

void header::draw(ST7796S::MSP4021 &tft) {
    const int gap = uiMockup::GAP;

    _area.outerX = 0;
    _area.outerY = 0;
    _area.outerW = tft.width();
    _area.outerH = uiMockup::HEADER_HEIGHT;

    _area.innerX = _area.outerX + gap;
    _area.innerY = _area.outerY + gap;
    _area.innerW = _area.outerW - (gap * 2);
    _area.innerH = _area.outerH - (gap * 2);

    tft.rectFill(
        _area.outerX, _area.outerY,
        _area.outerW, _area.outerH,
        theme::BLACK
    );
    tft.rectRound(
        _area.outerX,      _area.outerY,
        _area.outerW,      _area.outerH,
        uiMockup::RADIUS, theme::BORDER
    );
}

int header::outerX()        { return _area.outerX; }
int header::outerY()        { return _area.outerY; }
int header::outerWidth()    { return _area.outerW; }
int header::outerHeight()   { return _area.outerH; }

int header::innerX()        { return _area.innerX; }
int header::innerY()        { return _area.innerY; }
int header::innerWidth()    { return _area.innerW; }
int header::innerHeight()   { return _area.innerH; }
