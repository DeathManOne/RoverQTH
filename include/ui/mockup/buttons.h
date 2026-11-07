/*
 * include/ui/mockup/buttons.h
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

namespace ui::mockup::buttons {
    /** @brief Identifies each persistent navigation button. */
    enum class Id {MARK_QTH, OTA, MENU, COUNT};

    /** @brief Visual and interaction state of a persistent button. */
    enum class State {UNAVAILABLE, READY, RUNNING};

    /**
     * @brief Changes the state retained for one persistent button.
     * @param id Persistent button whose state is changed.
     * @param state New visual and interaction state of the button.
     */
    void setState(Id id, State state);

    /**
     * @brief Clears the persistent-button area.
     * @param tft Display instance whose button area is cleared.
     */
    void clear(ST7796S::MSP4021 &tft);

    /**
     * @brief Draws all persistent navigation buttons.
     * @param tft Display instance used to draw the buttons.
     */
    void draw(ST7796S::MSP4021 &tft);

    /**
     * @brief Redraws the marked-QTH button.
     * @param tft Display instance used to redraw the button.
     */
    void updateMARK(ST7796S::MSP4021 &tft);

    /**
     * @brief Redraws the OTA button.
     * @param tft Display instance used to redraw the button.
     */
    void updateOTA(ST7796S::MSP4021 &tft);

    /**
     * @brief Redraws the menu button.
     * @param tft Display instance used to redraw the button.
     */
    void updateMENU(ST7796S::MSP4021 &tft);
}
