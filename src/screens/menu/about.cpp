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

namespace screens::menu {
    void About::draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field<Action>* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        nameField.value     = PROJECT_NAME;
        versionField.value  = PROJECT_VERSION;
        authorField.value   = PROJECT_AUTHOR;
        licenseField.value  = PROJECT_LICENSE;
        linkField.value     = PROJECT_URL;

        drawTitle(tft, x, y, w, rowH, gap, "about");
        for (Field<Action>* field : fields)
            { drawLine(tft, *field); }
    }
}