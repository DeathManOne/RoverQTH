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

namespace services::battery {
    namespace {
        uint8_t batteryPin  = 0;
        uint8_t percent     = 0;
        float voltage       = 0.0f;

        bool present        = false;
        bool low            = false;
        bool critical       = false;

        constexpr float ADC_VREF            = 3.3f;
        constexpr int ADC_MAX               = 4095;
        constexpr float DIVIDER_RATIO       = 2.0f;

        constexpr float BATTERY_MIN         = 3.30f;
        constexpr float BATTERY_MAX         = 4.20f;
        constexpr float BATTERY_PRESENT     = 2.50f;
        constexpr float BATTERY_LOW         = 3.50f;
        constexpr float BATTERY_CRITICAL    = 3.30f;
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
        const int raw = analogRead(batteryPin);

        voltage = ((static_cast<float>(raw) / ADC_MAX) * ADC_VREF * DIVIDER_RATIO);

        present     = voltage > BATTERY_PRESENT;
        low         = present && voltage <= BATTERY_LOW;
        critical    = present && voltage <= BATTERY_CRITICAL;

        if (!present) { percent = 0; }
        else if (voltage <= BATTERY_MIN) { percent = 0; }
        else if (voltage >= BATTERY_MAX) { percent = 100; }
        else { percent = static_cast<uint8_t>(((voltage - BATTERY_MIN) * 100.0f) / (BATTERY_MAX - BATTERY_MIN)); }
        return true;
    }
}