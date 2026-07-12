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
    uint8_t _batteryPin  = 0;
    uint8_t _percent     = 0;
    float _voltage       = 0.0f;
    bool _critical       = false;
    bool _present        = false;
    bool _low            = false;

    constexpr float BATTERY_PRESENT             = 2.50f;
    constexpr float BATTERY_LOW_PERCENT         = 20.0f;
    constexpr float BATTERY_CRITICAL_PERCENT    = 10.0f;

    constexpr float ADC_VREF    = 3.3f;
    constexpr int ADC_MAX       = 4095;
}

float   battery::getVoltage() { return _voltage; }
uint8_t battery::getPercent() { return _percent; }
bool    battery::isPresent () { return _present; }
bool    battery::isLow     () { return _low; }
bool    battery::isCritical() { return _critical; }

bool battery::begin(uint8_t pin) {
    _batteryPin = pin;
    pinMode(_batteryPin, INPUT);

    analogReadResolution(12);
    #ifdef ESP32
        analogSetPinAttenuation(_batteryPin, ADC_11db);
    #endif
    return update();
}

bool battery::update() {
    const float min     = settings::getBatteryMinimal();
    const float max     = settings::getBatteryMaximal();

    const float range   = max - min;
    if (range <= 0.0f) {
        _voltage        = 0.0f;
        _percent        = 0;
        _present        = false;
        _low            = false;
        _critical       = false;
        return true;
    }

    const float ratioHigh   = settings::getBatteryRatioHigh() / 100.0f;
    const float ratioLow    = 1.0f - ratioHigh;
    if (ratioLow <= 0.0f || ratioHigh <= 0.0f) {
        _voltage            = 0.0f;
        _percent            = 0;
        _present            = false;
        _low                = false;
        _critical           = false;
        return true;
    }

    const float lowVoltage      = min + (range * BATTERY_LOW_PERCENT / 100.0f);
    const float criticalVoltage = min + (range * BATTERY_CRITICAL_PERCENT / 100.0f);

    _voltage     = (static_cast<float>(analogRead(_batteryPin)) / ADC_MAX) * ADC_VREF / ratioLow;
    _present     = _voltage > BATTERY_PRESENT;
    _low         = _present && _voltage <= lowVoltage;
    _critical    = _present && _voltage <= criticalVoltage;

    if      (!_present)       { _percent = 0; }
    else if (_voltage <= min) { _percent = 0; }
    else if (_voltage >= max) { _percent = 100; }
    else { _percent = static_cast<uint8_t>(((_voltage - min) * 100.0f) / range); }
    return true;
}
