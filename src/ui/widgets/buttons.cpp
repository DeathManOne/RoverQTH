/*
 * src/ui/widgets/buttons.cpp
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

#include <cstdint>
#include "ui/widgets/buttons.h"

namespace buttons = ui::widgets::buttons;

buttons::ButtonArea buttons::bootSearchGPS {};
buttons::ButtonArea buttons::markQTH {};
buttons::ButtonArea buttons::ota {};
buttons::ButtonArea buttons::menu {};

buttons::ButtonArea buttons::makeArea(int x, int y, int width, int height) {
    ButtonArea area;
    area.x      = x;
    area.y      = y;
    area.width  = width;
    area.height = height;
    return area;
}

bool buttons::isPressed(const ButtonArea& button, const int tx, const int ty) {
    if (button.width <= 0 || button.height <= 0) { return false; }

    const int64_t left   = button.x;
    const int64_t top    = button.y;
    const int64_t right  = left + button.width;
    const int64_t bottom = top  + button.height;

    return
        static_cast<int64_t>(tx) >= left    &&
        static_cast<int64_t>(tx) <  right   &&
        static_cast<int64_t>(ty) >= top     &&
        static_cast<int64_t>(ty) <  bottom;
}
