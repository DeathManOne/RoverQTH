/*
 * include/screens/main.h
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

#include "screens/screen.h"
#include "services/settings.h"

namespace screens {
    class Main final : public Screen {
        private:
            services::settings::OtaSelection _otaSelection {};

            void _preloadGPS();
            void _preloadOTA();
            void _preloadMARK();
            void _updateOTA(ST7796S::MSP4021 &tft);

        public:
            void preload() override;
            void draw(ST7796S::MSP4021 &tft) override;

            void update(
                ST7796S::MSP4021 &tft,
                uint32_t &nextRefreshIn
            ) override;

            bool handleTouch(
                ST7796S::MSP4021 &tft,
                int x,
                int y
            ) override;

            void updateMARK(ST7796S::MSP4021 &tft);
    };
}
