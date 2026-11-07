/*
 * include/ui/mockup/grid.h
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

namespace ui::mockup::grid {
    /**
     * @brief Clears the main grid area.
     * @param tft Display instance whose grid area is cleared.
     */
    void clear(ST7796S::MSP4021 &tft);

    /**
     * @brief Draws the main grid frame.
     * @param tft Display instance used to calculate and draw the grid.
     */
    void draw(ST7796S::MSP4021 &tft);

    /**
     * @brief Returns the outer left coordinate.
     * @return Outer horizontal origin in pixels.
     */
    int outerX();

    /**
     * @brief Returns the outer top coordinate.
     * @return Outer vertical origin in pixels.
     */
    int outerY();

    /**
     * @brief Returns the outer width.
     * @return Outer width in pixels.
     */
    int outerWidth();

    /**
     * @brief Returns the outer height.
     * @return Outer height in pixels.
     */
    int outerHeight();

    /**
     * @brief Returns the inner left coordinate.
     * @return Inner horizontal origin in pixels.
     */
    int innerX();

    /**
     * @brief Returns the inner top coordinate.
     * @return Inner vertical origin in pixels.
     */
    int innerY();

    /**
     * @brief Returns the inner width.
     * @return Inner width in pixels.
     */
    int innerWidth();

    /**
     * @brief Returns the inner height.
     * @return Inner height in pixels.
     */
    int innerHeight();
}
