/*
 * include/screens/boot.h
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
    /** @brief Draws and updates the application boot-status screen. */
    class Boot final {
        public:
            /**
             * @brief Clears the boot-screen area.
             * @param tft Display instance whose screen is cleared.
             */
            void clear(ST7796S::MSP4021 &tft);

            /**
             * @brief Draws the complete boot screen.
             * @param tft Display instance used to draw the screen.
             */
            void draw(ST7796S::MSP4021 &tft);

            /**
             * @brief Draws the application logo on the boot screen.
             * @param tft Display instance used to draw the logo.
             */
            void drawLogo(ST7796S::MSP4021 &tft);

            /**
             * @brief Updates the Wi-Fi initialization state.
             * @param tft Display instance used to redraw the Wi-Fi state.
             * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
             */
            void updateWifi(ST7796S::MSP4021 &tft, const bool* value);

            /**
             * @brief Updates the SD-card initialization state.
             * @param tft Display instance used to redraw the SD-card state.
             * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
             */
            void updateSD(ST7796S::MSP4021 &tft, const bool* value);

            /**
             * @brief Updates the GNSS initialization state.
             * @param tft Display instance used to redraw the GNSS state.
             * @param value State to display: `nullptr` while loading, `true` when loaded, or `false` on failure.
             */
            void updateGPS(ST7796S::MSP4021 &tft, const bool* value);

            /**
             * @brief Updates the GNSS acquisition percentage.
             * @param tft Display instance used to redraw the progress indicator.
             * @param progress Acquisition percentage, clamped to the range 0–100.
             */
            void updateGPSProgress(ST7796S::MSP4021 &tft, uint8_t progress);

            /**
             * @brief Handles the GNSS-search action on the boot screen.
             * @param x Horizontal touch coordinate in pixels.
             * @param y Vertical touch coordinate in pixels.
             * @return `true` when the GNSS-search button contains the touch coordinate.
             */
            bool handleTouch(int x, int y) const;
        private:
            enum class ModuleState : uint8_t {LOADING, LOADED, FAILED};

            struct Field {
                int outerX = 0;
                int outerY = 0;
                int outerW = 0;
                int outerH = 0;
                int innerX = 0;
                int innerY = 0;
                int innerW = 0;
                int innerH = 0;
                int row    = 0;

                const char* label = nullptr;
                ModuleState state = ModuleState::LOADING;

                Field(int rowIndex, const char* name) : row(rowIndex), label(name) {}
            };

            uint8_t _gpsProgress = 0;

            Field _wifi{0, "WiFi"};
            Field _sd  {1, "SD"};
            Field _gps {2, "GPS"};

            void _clearModule(ST7796S::MSP4021 &tft, const Field &field);
            void _drawModuleStatus(ST7796S::MSP4021 &tft, const Field &field);
            void _updateField(ST7796S::MSP4021 &tft, Field &field, const bool* state);
            void _drawGPSSearch(ST7796S::MSP4021 &tft);
            void _drawGPSAction(ST7796S::MSP4021 &tft);
            void _drawGPSProgress(ST7796S::MSP4021 &tft, uint8_t value);
    };
}
