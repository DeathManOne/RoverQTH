/*
 * include/screens/screen.h
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

#include <cstdint>

#include <MSP4021.h>

namespace screens {
    /** @brief Abstract contract implemented by every navigable application screen. */
    class Screen {
        public:
            /** @brief Enables safe polymorphic destruction. */
            virtual ~Screen() = default;

            /** @brief Loads the initial values required before drawing the screen. */
            virtual void preload() = 0;

            /**
             * @brief Draws the complete screen.
             * @param tft Display instance used to draw the screen.
             */
            virtual void draw(ST7796S::MSP4021 &tft) = 0;

            /**
             * @brief Refreshes dynamic content and supplies the next refresh delay.
             * @param tft Display instance used to refresh the screen.
             * @param nextRefreshIn Receives the delay in milliseconds before the next refresh.
             */
            virtual void update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) = 0;

            /**
             * @brief Handles a mapped touch event.
             * @param tft Display instance available to redraw affected controls.
             * @param x Horizontal touch coordinate in pixels.
             * @param y Vertical touch coordinate in pixels.
             * @return `true` when the touch event was handled.
             */
            virtual bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) = 0;

            /**
             * @brief Reports whether the screen currently owns an editing overlay.
             * @return `true` when an editing overlay is active.
             */
            virtual bool isEditing() const { return false; }
    };
}
