/*
 * ui/widgets/buttons.cpp
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

#include "ui/widgets/buttons.h"

namespace ui::widgets::buttons {
    ButtonArea BOOT_SEARCH_GPS;
    ButtonArea MARK_QTH;
    ButtonArea SOTA;
    ButtonArea MENU;

    ButtonArea makeArea(int x, int y, int width, int height) {
        ButtonArea area;
        area.x      = x;
        area.y      = y;
        area.width  = width;
        area.height = height;
        return area;
    }

    bool isPressed(const ButtonArea &button, int tx, int ty) {
        return (
            tx >= button.x && tx < (button.x + button.width) &&
            ty >= button.y && ty < (button.y + button.height)
        );
    }
}
