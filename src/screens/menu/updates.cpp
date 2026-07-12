/*
 * src/screens/menu/updates.cpp
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

#include "screens/menu/updates.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::Updates;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

void Updates::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()       + (gap * 2);
    const int y     = grid::innerY()       + (gap * 2);
    const int w     = grid::innerWidth()   - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    _sotaVersionField.value = _sotaVersionValue;
    _sotaUpdateField.value  = _sotaUpdateValue;
    _potaVersionField.value = _potaVersionValue;
    _potaUpdateField.value  = _potaUpdateValue;

    _drawTitle(tft, x, y, w, rowH, gap, "updates");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}
