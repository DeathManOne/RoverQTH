/*
 * include/services/display.h
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

namespace services::display {
    /** @brief Persistent navigation buttons managed by the display service. */
    enum class Button {MARK_QTH, OTA, MENU, COUNT};

    /** @brief Visual and interaction state of a persistent button. */
    enum class ButtonState {UNAVAILABLE, READY, RUNNING};

    /** @brief Initializes the physical display, touch input and screen objects. */
    void begin();

    /** @brief Displays the initial application screen after boot completes. */
    void start();

    /** @brief Places the display hardware into its shutdown state. */
    void shutdown();

    /**
     * @brief Refreshes the active screen and returns its next refresh delay.
     * @param nextRefreshIn Receives the delay in milliseconds before the next refresh.
     */
    void update(uint32_t &nextRefreshIn);

    /** @brief Dispatches one pending touch event to the active screen. */
    void handleTouch();

    /**
     * @brief Returns the current state of a persistent navigation button.
     * @param button Persistent button whose state is requested.
     * @return Current state of the requested button.
     */
    ButtonState buttonState(Button button);

    /**
     * @brief Updates and redraws a persistent navigation button when required.
     * @param button Persistent button whose state is updated.
     * @param state New interaction and visual state of the button.
     */
    void setButtonState(Button button, ButtonState state);

    /**
     * @brief Reads and maps the current touch coordinates.
     * @param x Receives the mapped horizontal touch coordinate.
     * @param y Receives the mapped vertical touch coordinate.
     * @return `true` when a touch position was read successfully.
     */
    bool readTouch(int &x, int &y);

    /** @brief Clears the boot-screen drawing area. */
    void clearBoot();

    /** @brief Draws the complete boot screen. */
    void drawBoot();

    /** @brief Draws the boot logo. */
    void drawBootLogo();

    /**
     * @brief Updates the Wi-Fi boot status.
     * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
     */
    void updateBootWifi(const bool* value);

    /**
     * @brief Updates the SD-card boot status.
     * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
     */
    void updateBootSD(const bool* value);

    /**
     * @brief Updates the GNSS boot status.
     * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
     */
    void updateBootGPS(const bool* value);

    /**
     * @brief Updates the GNSS acquisition progress displayed during boot.
     * @param progress Acquisition percentage, clamped to the range 0–100.
     */
    void updateBootGPSProgress(uint8_t progress);

    /**
     * @brief Handles a touch event while the boot screen is active.
     * @param x Horizontal touch coordinate in pixels.
     * @param y Vertical touch coordinate in pixels.
     * @return `true` when the GNSS-search button contains the touch coordinate.
     */
    bool handleBootTouch(int x, int y);
}
