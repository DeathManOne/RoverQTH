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

#include <esp_heap_caps.h>
#include <esp_system.h>
#include <rom/rtc.h>

#include "core/boot.h"
#include "core/screenManager.h"
#include "core/state.h"
#include "display/manager.h"
#include "RoverQTH.h"
#include "services/battery.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/pota.h"
#include "services/power.h"
#include "services/qth.h"
#include "services/settings.h"
#include "services/sota.h"
#include "services/storage.h"
#include "services/update.h"
#include "services/wifi.h"
#include "ui/settings/gps.h"

namespace boot        = core::boot;
namespace manager     = core::screenManager;
namespace state       = core::state;
namespace app         = RoverQTH;
namespace battery     = services::battery;
namespace gps         = services::gps;
namespace navigation  = services::navigation;
namespace pota        = services::pota;
namespace power       = services::power;
namespace qth         = services::qth;
namespace settings    = services::settings;
namespace sota        = services::sota;
namespace storage     = services::storage;
namespace update      = services::update;
namespace wifi        = services::wifi;
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
            gps::update(GPS_UPDATE_TIMEOUT_MS);

            gps::Snapshot snapshot {};
            if (gps::getSnapshot(snapshot)) {
                const navigation::Coordinate coordinate { snapshot.latitude, snapshot.longitude, snapshot.altitude};
                navigation::updateGPSFix(coordinate, snapshot.fixValid && snapshot.positionValid);
            } else { navigation::updateGPSFix({}, false); }

            const uint32_t elapsedMs = millis() - startMs;
            if (elapsedMs < GPS_SAMPLE_PERIOD_MS)
                { vTaskDelay(pdMS_TO_TICKS(GPS_SAMPLE_PERIOD_MS - elapsedMs)); }
        }
    }
}

void app::setup() {
    storage::appendLogRecord("SYSTEM_START");

    char versionRecord[48];
    const int versionWritten = snprintf(
        versionRecord, sizeof(versionRecord),
        "FIRMWARE_VERSION version=%s",
        PROJECT_VERSION
    );

    if (versionWritten > 0 &&
        static_cast<size_t>(versionWritten) < sizeof(versionRecord)
    ) { storage::appendLogRecord(versionRecord); }

    char resetRecord[64];
    const int written = snprintf(
        resetRecord, sizeof(resetRecord),
        "RESET_REASON esp=%d cpu0=%d cpu1=%d",
        static_cast<int>(esp_reset_reason()),
        static_cast<int>(rtc_get_reset_reason(0)),
        static_cast<int>(rtc_get_reset_reason(1))
    );

    if (written > 0 &&
        static_cast<size_t>(written) < sizeof(resetRecord)
    ) { storage::appendLogRecord(resetRecord); }

    settings::begin();
    power::begin(BTN_PIN);

    battery::begin(BATT_PIN);
    if (battery::isCritical()) { power::shutdown(power::ShutdownReason::BATTERY_CRITICAL); }

    display::begin(
        TFT_CLK,        TFT_MISO,       TFT_MOSI,
        TFT_TOUCH_CS,   TFT_SCREEN_CS,  TFT_SCREEN_DC,
        TFT_SCREEN_RST, TFT_WIDTH,      TFT_HEIGHT
    );

    navigation::begin();
    wifi::begin();

    state::begin();
    boot::run(_gpsUART, _sdSPI);
    update::begin();

    sota::begin();
    pota::begin();

    if (sota::snapshot().status != sota::Status::UNAVAILABLE ||
        pota::snapshot().status != pota::Status::UNAVAILABLE
    ) { state::setButtonState(state::Button::SOTA, state::ButtonState::READY); }

    manager::begin();

    _nextScreenRefresh   = millis() + SCREEN_REFRESH_MS;
    _nextBatteryRefresh  = millis() + BATTERY_PERIOD_MS;

    const BaseType_t gpsTaskResult = xTaskCreatePinnedToCore(
        _gpsTask, "GNSS", 8192, nullptr, 1, &_gpsTaskHandle, 0
    );

    if (gpsTaskResult != pdPASS) { storage::appendErrorRecord("GPS_TASK_CREATE_FAILED"); }
    else {
        char memoryRecord[96];
        const int memoryWritten = snprintf(
            memoryRecord, sizeof(memoryRecord),
            "MEMORY_READY free=%lu minimum=%lu largest=%lu",
            static_cast<unsigned long>(heap_caps_get_free_size(MALLOC_CAP_8BIT)),
            static_cast<unsigned long>(heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)),
            static_cast<unsigned long>(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))
        );

        if (memoryWritten > 0 &&
            static_cast<size_t>(memoryWritten) < sizeof(memoryRecord)
        ) { storage::appendLogRecord(memoryRecord); }

        char bootRecord[48];
        const int bootWritten = snprintf(
            bootRecord, sizeof(bootRecord),
            "SYSTEM_BOOT_DURATION duration_ms=%lu",
            static_cast<unsigned long>(millis())
        );

        if (bootWritten > 0 &&
            static_cast<size_t>(bootWritten) < sizeof(bootRecord)
        ) { storage::appendLogRecord(bootRecord); }
        storage::appendLogRecord("SYSTEM_READY");
    }
}

void app::loop() {
    power::update();
    wifi::update();

    navigation::TracePoint pendingPoint {};
    while (navigation::peekPendingTracePoint(pendingPoint)) {
        qth::TracePoint tracePoint {};
        tracePoint.utc       = pendingPoint.utc;
        tracePoint.latitude  = pendingPoint.coordinate.latitude;
        tracePoint.longitude = pendingPoint.coordinate.longitude;
        tracePoint.altitude  = pendingPoint.coordinate.altitude;

        if (!qth::appendTracePoint(tracePoint)) {
            storage::appendErrorRecord("QTH_TRACE_APPEND_FAILED");
            break;
        }

        if (!navigation::discardPendingTracePoint()) {
            storage::appendErrorRecord("QTH_TRACE_QUEUE_FAILED");
            break;
        }
    }

    const uint32_t now = millis();
    if (static_cast<int32_t>(now - _nextBatteryRefresh) >= 0) {
        battery::update();
        //if (battery::isCritical())
            //{ power::shutdown(power::ShutdownReason::BATTERY_CRITICAL); }
        _nextBatteryRefresh = now + BATTERY_PERIOD_MS;
    }

    manager::handleTouch();

    if (static_cast<int32_t>(now - _nextScreenRefresh) < 0) { return; }
    uint32_t nextRefreshIn = SCREEN_REFRESH_MS;

    manager::update(nextRefreshIn);
    _nextScreenRefresh = now + nextRefreshIn;
}
