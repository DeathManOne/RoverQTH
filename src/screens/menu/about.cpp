/*
 * src/screens/menu/about.cpp
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
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::About;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

void About::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()     + (gap * 2);
    const int y     = grid::innerY()     + (gap * 2);
    const int w     = grid::innerWidth() - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    _nameField.value     = PROJECT_NAME;
    _versionField.value  = PROJECT_VERSION;
    _authorField.value   = PROJECT_AUTHOR;
    _licenseField.value  = PROJECT_LICENSE;
    _linkField.value     = PROJECT_URL;

    _drawTitle(tft, x, y, w, rowH, gap, "about");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}
