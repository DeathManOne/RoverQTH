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

    /** @brief Refreshes the active screen and returns its next refresh delay. */
    void update(uint32_t &nextRefreshIn);
    /** @brief Dispatches one pending touch event to the active screen. */
    void handleTouch();

    /** @brief Returns the current state of a persistent navigation button. */
    ButtonState buttonState(Button button);
    /** @brief Updates and redraws a persistent navigation button when required. */
    void setButtonState(Button button, ButtonState state);

    /** @brief Reads and maps the current touch coordinates. */
    bool readTouch(int &x, int &y);

    /** @brief Clears the boot-screen drawing area. */
    void clearBoot();
    /** @brief Draws the complete boot screen. */
    void drawBoot();
    /** @brief Draws the boot logo. */
    void drawBootLogo();
    /** @brief Updates the Wi-Fi boot status; null denotes an in-progress state. */
    void updateBootWifi(const bool* value);
    /** @brief Updates the SD-card boot status; null denotes an in-progress state. */
    void updateBootSD(const bool* value);
    /** @brief Updates the GNSS boot status; null denotes an in-progress state. */
    void updateBootGPS(const bool* value);
    /** @brief Updates the GNSS acquisition progress displayed during boot. */
    void updateBootGPSProgress(uint8_t progress);
    /** @brief Handles a touch event while the boot screen is active. */
    bool handleBootTouch(int x, int y);
}
