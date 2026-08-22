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
    class Boot final {
        public:
            void clear   (ST7796S::MSP4021 &tft);
            void draw    (ST7796S::MSP4021 &tft);
            void drawLogo(ST7796S::MSP4021 &tft);

            void updateWifi(ST7796S::MSP4021 &tft, bool* value);
            void updateSD  (ST7796S::MSP4021 &tft, bool* value);
            void updateGPS (ST7796S::MSP4021 &tft, bool* value);
            void updateGPSProgress(ST7796S::MSP4021 &tft, uint8_t progress);
            bool handleTouch(int x, int y) const;

        private:
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
                bool* state       = nullptr;

                Field(int rowIndex, const char* name)
                : row(rowIndex), label(name) {}
            };

            uint8_t _gpsProgress = 0;
            Field _wifi{0, "WiFi"};
            Field _sd  {1, "SD"};
            Field _gps {2, "GPS"};

            void _clearModule     (ST7796S::MSP4021 &tft, const Field &field);
            void _drawModuleStatus(ST7796S::MSP4021 &tft, Field &field);
            void _updateField     (ST7796S::MSP4021 &tft, Field &field, bool* state);
            void _drawGPSSearch   (ST7796S::MSP4021 &tft);
            void _drawGPSAction   (ST7796S::MSP4021 &tft);
            void _drawGPSProgress (ST7796S::MSP4021 &tft, uint8_t value);
    };
}
