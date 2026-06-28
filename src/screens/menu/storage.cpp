/*
 * screens/menu/storage.cpp
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

#include "screens/menu/storage.h"
#include "screens/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

#include "ui/fonts/RobotoMono_Bold_16.h"

namespace screens::menu::storage {
    namespace {
        enum class Mode {GRID, KEYBOARD};
        Mode mode = Mode::GRID;
    }

    bool isEditing()    { return mode == Mode::KEYBOARD; }
    void reset()        { mode = Mode::GRID; }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX() + (gap * 2);
        const int y     = screens::mockup::grid::innerY() + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth() - (gap * 4);
        const int rowH  = 28;

        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(ui::settings::themes::defaults::GREEN);
        tft.textCenterLeft(x, y, w, rowH, "STORAGE");

        tft.lineH(
            x, y + rowH + gap,
            w, ui::settings::themes::defaults::BORDER
        );

        tft.textCenter(
            screens::mockup::grid::innerX(),
            screens::mockup::grid::innerY(),
            screens::mockup::grid::innerWidth(),
            screens::mockup::grid::innerHeight(),
            "COMING SOON"
        );
    }
    
    bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) { return false; }
}