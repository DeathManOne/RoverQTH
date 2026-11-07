/*
 * include/screens/menu/navigation.h
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

#pragma once

#include <MSP4021.h>

#include "screens/menu/item.h"

namespace screens::menu::navigation {
    /**
     * @brief Draws the menu navigation bar and highlights the current item.
     * @param tft Display instance used to draw the navigation bar.
     * @param current Menu item to highlight as currently selected.
     */
    void draw(ST7796S::MSP4021 &tft, Item current);

    /**
     * @brief Maps a navigation-bar touch to a selected menu item.
     * @param x Horizontal touch coordinate in pixels.
     * @param y Vertical touch coordinate in pixels.
     * @param selected Receives the menu item matching the touched row.
     * @return `true` when the touch selected a navigation item.
     */
    bool handleTouch(int x, int y, Item &selected);
}
