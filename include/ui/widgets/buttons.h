/*
 * include/ui/widgets/buttons.h
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

namespace ui::widgets::buttons {
    /** @brief Rectangular touch target expressed in display coordinates. */
    struct ButtonArea {
        int x       = 0;
        int y       = 0;
        int width   = 0;
        int height  = 0;
    };

    /** @brief Touch target for the boot-screen GNSS search action. */
    extern ButtonArea bootSearchGPS;

    /** @brief Touch target for the persistent marked-QTH action. */
    extern ButtonArea markQTH;

    /** @brief Touch target for the persistent OTA-screen action. */
    extern ButtonArea ota;

    /** @brief Touch target for the persistent menu action. */
    extern ButtonArea menu;

    /**
     * @brief Constructs a rectangular touch target from its geometry.
     * @param x Horizontal origin of the target in pixels.
     * @param y Vertical origin of the target in pixels.
     * @param width Width of the target in pixels.
     * @param height Height of the target in pixels.
     * @return Constructed touch-target area.
     */
    ButtonArea makeArea(int x, int y, int width, int height);

    /**
     * @brief Tests whether mapped touch coordinates fall inside a target.
     * @param button Touch-target area to test.
     * @param tx Horizontal touch coordinate in pixels.
     * @param ty Vertical touch coordinate in pixels.
     * @return `true` when the coordinates lie inside a valid target.
     */
    bool isPressed(const ButtonArea &button, int tx, int ty);
}
