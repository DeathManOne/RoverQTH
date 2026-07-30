/*
 * src/services/battery.cpp
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

namespace battery  = services::battery;
namespace settings = services::settings;

namespace {
    constexpr float BATTERY_PRESENT             = 2.50f;
    constexpr float BATTERY_LOW_PERCENT         = 20.0f;
    constexpr float BATTERY_CRITICAL_PERCENT    = 10.0f;

    constexpr float ADC_VREF              = 3.3f;
    constexpr int ADC_MAX                 = 4095;
    constexpr uint8_t ADC_SAMPLE_COUNT    = 8U;

    uint8_t _batteryPin  = 0;
    uint8_t _percent     = 0;
    float _voltage       = 0.0f;
    bool _critical       = false;
    bool _present        = false;
    bool _low            = false;

    void _resetState() {
        _voltage  = 0.0f;
        _percent  = 0;
        _present  = false;
        _low      = false;
        _critical = false;
    }

    uint16_t _readAverageADC() {
        uint32_t total = 0;
        for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; ++sample)
            { total += static_cast<uint32_t>(analogRead(_batteryPin)); }
        return static_cast<uint16_t>(total / ADC_SAMPLE_COUNT);
    }
}

uint8_t battery::getPercent() { return _percent; }
bool    battery::isPresent () { return _present; }
bool    battery::isCritical() { return _critical; }

void battery::begin(uint8_t pin) {
    _batteryPin = pin;
    pinMode(_batteryPin, INPUT);

    analogReadResolution(12);
    #ifdef ESP32
        analogSetPinAttenuation(_batteryPin, ADC_11db);
    #endif
    update();
}

void battery::update() {
    const float min     = settings::getBatteryMinimal();
    const float max     = settings::getBatteryMaximal();
    const float range   = max - min;

    if (range <= 0.0f) {
        _resetState();
        return;
    }

    const float ratioHigh   = settings::getBatteryRatioHigh() / 100.0f;
    const float ratioLow    = 1.0f - ratioHigh;
    if (ratioLow <= 0.0f || ratioHigh <= 0.0f) {
        _resetState();
        return;
    }

    const float lowVoltage      = min + (range * BATTERY_LOW_PERCENT / 100.0f);
    const float criticalVoltage = min + (range * BATTERY_CRITICAL_PERCENT / 100.0f);
    const uint16_t adcValue     = _readAverageADC();

    _voltage  = (static_cast<float>(adcValue) / static_cast<float>(ADC_MAX)) * ADC_VREF / ratioLow;
    _present  = _voltage > BATTERY_PRESENT;
    _low      = _present && _voltage <= lowVoltage;
    _critical = _present && _voltage <= criticalVoltage;

    if (!_present || _voltage <= min) {
        _percent = 0;
        return;
    }
    if (_voltage >= max) {
        _percent = 100;
        return;
    }
    _percent = static_cast<uint8_t>(((_voltage - min) * 100.0f) / range);
}
