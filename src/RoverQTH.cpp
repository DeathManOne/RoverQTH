/*
 * RoverQTH.cpp
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

#include "RoverQTH.h"
#include <Arduino.h>
#include <SPI.h>
#include "core/boot.h"
#include "core/power.h"
#include "core/screenManager.h"
#include "core/state.h"
#include "display/manager.h"
#include "services/battery.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/nvs.h"
#include "services/settings.h"
#include "services/touch.h"
#include "services/update.h"

namespace RoverQTH {
    namespace {
        SPIClass sdSPI(HSPI);
        HardwareSerial gpsUART(1);

        uint32_t nextScreenRefresh  = 0;
        uint32_t nextBatteryRefresh = 0;

        TaskHandle_t gpsTaskHandle  = nullptr;

        constexpr uint32_t GPS_UPDATE_TIMEOUT_MS = 900;
        constexpr uint32_t GPS_TASK_PERIOD_MS    = 1000;
        constexpr uint32_t BATTERY_PERIOD_MS     = 5000;

        void gpsTask(void*) {
            while (true) {
                const uint32_t startMs = millis();
                services::gps::update(GPS_UPDATE_TIMEOUT_MS);

                const uint32_t elapsedMs = millis() - startMs;
                if (elapsedMs < GPS_TASK_PERIOD_MS)
                    { vTaskDelay(pdMS_TO_TICKS(GPS_TASK_PERIOD_MS - elapsedMs)); }
            }
        }
    }

    void setup() {
        services::nvs::begin();
        services::settings::begin();
        services::battery::begin(BATT_PIN);
        if (core::power::shouldShutdown())
            { core::power::shutdown(); }

        display::begin(
            TFT_CLK,        TFT_MISO,       TFT_MOSI,
            TFT_TOUCH_CS,
            TFT_SCREEN_CS,  TFT_SCREEN_DC,  TFT_SCREEN_RST,
            TFT_WIDTH,      TFT_HEIGHT,     TFT_ROTATION
        );

        services::touch::begin();
        services::update::begin();
        services::navigation::begin();

        core::state::begin();
        core::boot::run(gpsUART, sdSPI);
        core::screenManager::begin();

        nextScreenRefresh   = millis() + GPS_TASK_PERIOD_MS;
        nextBatteryRefresh  = millis() + BATTERY_PERIOD_MS;
        xTaskCreatePinnedToCore(gpsTask, "GNSS", 8192, nullptr, 1, &gpsTaskHandle, 0);
    }

    void loop() {
        const uint32_t now = millis();

        if (now >= nextBatteryRefresh) {
            services::battery::update();
            if (core::power::shouldShutdown())
                { core::power::shutdown(); }
            nextBatteryRefresh = now + BATTERY_PERIOD_MS;
        }

        core::screenManager::handleTouch();

        if (now < nextScreenRefresh)
            { return; }
        uint32_t nextRefreshIn = GPS_TASK_PERIOD_MS;
        core::screenManager::update(nextRefreshIn);
        nextScreenRefresh = now + nextRefreshIn;
    }
}