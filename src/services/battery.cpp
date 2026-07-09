/*
 * core/battery.cpp
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
#include "services/battery.h"
#include "services/settings.h"

namespace services::battery {
    namespace {
        uint8_t batteryPin  = 0;
        uint8_t percent     = 0;
        float voltage       = 0.0f;

        bool present        = false;
        bool low            = false;
        bool critical       = false;

        constexpr float BATTERY_PRESENT             = 2.50f;
        constexpr float BATTERY_LOW_PERCENT         = 20.0f;
        constexpr float BATTERY_CRITICAL_PERCENT    = 10.0f;

        constexpr float ADC_VREF    = 3.3f;
        constexpr int ADC_MAX       = 4095;
    }

    float   getVoltage()    { return voltage; }
    uint8_t getPercent()    { return percent; }
    bool    isPresent()     { return present; }
    bool    isLow()         { return low; }
    bool    isCritical()    { return critical; }

    bool begin(uint8_t pin) {
        batteryPin = pin;
        pinMode(batteryPin, INPUT);

        analogReadResolution(12);
        #ifdef ESP32
            analogSetPinAttenuation(batteryPin, ADC_11db);
        #endif
        return update();
    }

    bool update() {
        const float min     = services::settings::getBatteryMinimal();
        const float max     = services::settings::getBatteryMaximal();

        const float range   = max - min;
        if (range <= 0.0f) {
            voltage     = 0.0f;
            percent     = 0;
            present     = false;
            low         = false;
            critical    = false;
            return true;
        }

        const float ratioHigh   = services::settings::getBatteryRatioHigh() / 100.0f;
        const float ratioLow    = 1.0f - ratioHigh;
        if (ratioLow <= 0.0f || ratioHigh <= 0.0f) {
            voltage     = 0.0f;
            percent     = 0;
            present     = false;
            low         = false;
            critical    = false;
            return true;
        }

        const float lowVoltage      = min + (range * BATTERY_LOW_PERCENT / 100.0f);
        const float criticalVoltage = min + (range * BATTERY_CRITICAL_PERCENT / 100.0f);

        voltage     = (static_cast<float>(analogRead(batteryPin)) / ADC_MAX) * ADC_VREF / ratioLow;
        present     = voltage > BATTERY_PRESENT;
        low         = present && voltage <= lowVoltage;
        critical    = present && voltage <= criticalVoltage;

        if      (!present)          { percent = 0; }
        else if (voltage <= min)    { percent = 0; }
        else if (voltage >= max)    { percent = 100; }
        else { percent = static_cast<uint8_t>(((voltage - min) * 100.0f) / range); }
        return true;
    }
}