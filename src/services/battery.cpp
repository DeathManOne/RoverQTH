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
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr float BATTERY_PRESENT          = 2.50f;
    constexpr float BATTERY_CRITICAL_PERCENT = 10.0f;
    constexpr float MILLIVOLTS_PER_VOLT      = 1000.0f;
    constexpr uint8_t ADC_SAMPLE_COUNT       = 8U;

    uint8_t _batteryPin = 0;
    uint8_t _percent    = 0;
    bool _critical      = false;
    bool _present       = false;

    void _resetState() {
        portENTER_CRITICAL(&_lock);
        _percent  = 0;
        _present  = false;
        _critical = false;
        portEXIT_CRITICAL(&_lock);
    }

    uint32_t _readAverageMillivolts() {
        uint32_t total = 0;
        for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; ++sample)
            { total += analogReadMilliVolts(_batteryPin); }
        return total / ADC_SAMPLE_COUNT;
    }
}

uint8_t battery::getPercent() {
    portENTER_CRITICAL(&_lock);
    const uint8_t value = _percent;
    portEXIT_CRITICAL(&_lock);
    return value;
}

bool battery::isPresent () {
    portENTER_CRITICAL(&_lock);
    const bool value = _present;
    portEXIT_CRITICAL(&_lock);
    return value;
}

bool battery::isCritical() {
    portENTER_CRITICAL(&_lock);
    const bool value = _critical;
    portEXIT_CRITICAL(&_lock);
    return value;
}

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
    const settings::Battery configuration = settings::battery();

    const float min   = configuration.minimal;
    const float max   = configuration.maximal;
    const float range = max - min;

    if (range <= 0.0f) {
        _resetState();
        return;
    }

    const float ratioHigh = configuration.ratioHigh / 100.0f;
    const float ratioLow  = 1.0f - ratioHigh;
    if (ratioLow <= 0.0f || ratioHigh <= 0.0f) {
        _resetState();
        return;
    }

    const float criticalVoltage = min + (range * BATTERY_CRITICAL_PERCENT / 100.0f);
    const uint32_t millivolts   = _readAverageMillivolts();
    const float voltage         = (static_cast<float>(millivolts) / MILLIVOLTS_PER_VOLT) / ratioLow;
    const bool present          = voltage > BATTERY_PRESENT;
    const bool critical         = present && voltage <= criticalVoltage;

    uint8_t percent     = 0;
    if      (voltage <= min || !present) { percent = 0; }
    else if (voltage >= max)             { percent = 100; }
    else { percent = static_cast<uint8_t>(((voltage - min) * 100.0f) / range); }

    portENTER_CRITICAL(&_lock);
    _present  = present;
    _critical = critical;
    _percent  = percent;
    portEXIT_CRITICAL(&_lock);
}
