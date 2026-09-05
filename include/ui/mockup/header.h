/*
 * include/ui/mockup/header.h
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

namespace ui::mockup::header {
    /** @brief Clears the shared header area. */
    void clear(ST7796S::MSP4021 &tft);
    /** @brief Draws the shared header frame. */
    void draw(ST7796S::MSP4021 &tft);

    /** @brief Returns the outer left coordinate. */
    int outerX();
    /** @brief Returns the outer top coordinate. */
    int outerY();
    /** @brief Returns the outer width. */
    int outerWidth();
    /** @brief Returns the outer height. */
    int outerHeight();

    /** @brief Returns the inner left coordinate. */
    int innerX();
    /** @brief Returns the inner top coordinate. */
    int innerY();
    /** @brief Returns the inner width. */
    int innerWidth();
    /** @brief Returns the inner height. */
    int innerHeight();
}
