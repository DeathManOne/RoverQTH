/*
 * src/RoverQTH.cpp
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

#include <Arduino.h>
#include <SPI.h>

#include "core/boot.h"
#include "core/power.h"
#include "core/screenManager.h"
#include "core/state.h"
#include "display/manager.h"
#include "RoverQTH.h"
#include "services/battery.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/settings.h"
#include "services/storage.h"
#include "services/update.h"
#include "ui/settings/gps.h"

namespace boot        = core::boot;
namespace power       = core::power;
namespace manager     = core::screenManager;
namespace state       = core::state;
namespace app         = RoverQTH;
namespace battery     = services::battery;
namespace gps         = services::gps;
namespace navigation  = services::navigation;
namespace settings    = services::settings;
namespace storage     = services::storage;
namespace update      = services::update;
namespace gpsSettings = ui::settings::gps;

namespace {
    SPIClass _sdSPI(HSPI);
    HardwareSerial _gpsUART(1);

    uint32_t _nextScreenRefresh  = 0;
    uint32_t _nextBatteryRefresh = 0;
    TaskHandle_t _gpsTaskHandle  = nullptr;

    constexpr uint32_t GPS_SAMPLE_PERIOD_MS  = 1000 / gpsSettings::NAVIGATION_RATE_HZ;
    constexpr uint32_t GPS_UPDATE_TIMEOUT_MS = GPS_SAMPLE_PERIOD_MS + 100;
    constexpr uint32_t SCREEN_REFRESH_MS     = 1000;
    constexpr uint32_t BATTERY_PERIOD_MS     = 5000;

    void _gpsTask(void*) {
        while (true) {
            const uint32_t startMs = millis();
            navigation::updateGPSFix(gps::update(GPS_UPDATE_TIMEOUT_MS));

            const uint32_t elapsedMs = millis() - startMs;
            if (elapsedMs < GPS_SAMPLE_PERIOD_MS)
                { vTaskDelay(pdMS_TO_TICKS(GPS_SAMPLE_PERIOD_MS - elapsedMs)); }
        }
    }
}

void app::setup() {
    storage::appendLogRecord("SYSTEM_START");

    if (!settings::begin()) { storage::appendErrorRecord("NVS_INIT_FAILED"); }

    battery::begin(BATT_PIN);
    if (power::shouldShutdown())
        { power::shutdown(); }

    display::begin(
        TFT_CLK,        TFT_MISO,       TFT_MOSI,
        TFT_TOUCH_CS,   TFT_SCREEN_CS,  TFT_SCREEN_DC,
        TFT_SCREEN_RST, TFT_WIDTH,      TFT_HEIGHT
    );

    update::begin();
    navigation::begin();

    state::begin();
    boot::run(_gpsUART, _sdSPI);
    manager::begin();

    _nextScreenRefresh   = millis() + SCREEN_REFRESH_MS;
    _nextBatteryRefresh  = millis() + BATTERY_PERIOD_MS;

    const BaseType_t gpsTaskResult = xTaskCreatePinnedToCore(_gpsTask, "GNSS", 8192, nullptr, 1, &_gpsTaskHandle, 0);
    if (gpsTaskResult != pdPASS) { storage::appendErrorRecord("GPS_TASK_CREATE_FAILED"); }
    else { storage::appendLogRecord("SYSTEM_READY"); }

}

void app::loop() {
    const uint32_t now = millis();
    if (now >= _nextBatteryRefresh) {
        battery::update();
        if (power::shouldShutdown())
            { power::shutdown(); }
        _nextBatteryRefresh = now + BATTERY_PERIOD_MS;
    }
    manager::handleTouch();

    if (now < _nextScreenRefresh) { return; }
    uint32_t nextRefreshIn = SCREEN_REFRESH_MS;

    manager::update(nextRefreshIn);
    _nextScreenRefresh = now + nextRefreshIn;
}
