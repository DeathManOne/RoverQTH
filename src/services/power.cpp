/*
 * src/services/power.cpp
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
#include <driver/rtc_io.h>
#include <esp_sleep.h>

#include "display/manager.h"
#include "services/power.h"
#include "services/storage.h"
#include "services/update.h"

namespace power   = services::power;
namespace storage = services::storage;
namespace update  = services::update;

namespace {
    constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
    uint8_t _buttonPin                    = 0;
    uint32_t _lastButtonChange            = 0;
    gpio_num_t _wakeupGpio                = GPIO_NUM_NC;

    bool _initialized     = false;
    bool _rawPressed      = false;
    bool _stablePressed   = false;
    bool _buttonArmed     = false;
    bool _shutdownPending = false;

    power::ShutdownReason _shutdownReason = power::ShutdownReason::BUTTON;

    const char* _shutdownLog(const power::ShutdownReason reason) {
        switch (reason) {
            case power::ShutdownReason::BATTERY_CRITICAL: return "SYSTEM_SHUTDOWN_BATTERY_CRITICAL";
            case power::ShutdownReason::BUTTON:
            default:                                      return "SYSTEM_SHUTDOWN_BUTTON";
        }
    }

    void _tryShutdown() {
        if (!_initialized || !_shutdownPending) { return; }
        if (update::isBusy())                   { return; }
        if (digitalRead(_buttonPin) == LOW)     { return; }

        rtc_gpio_pullup_en(_wakeupGpio);
        rtc_gpio_pulldown_dis(_wakeupGpio);

        const esp_err_t wakeupResult = esp_sleep_enable_ext0_wakeup(_wakeupGpio, 0);
        if (wakeupResult != ESP_OK) {
            storage::appendErrorRecord("POWER_WAKEUP_CONFIGURATION_FAILED");
            _shutdownPending = false;

            rtc_gpio_deinit(_wakeupGpio);
            pinMode(_buttonPin, INPUT_PULLUP);
            return;
        }

        storage::appendLogRecord(_shutdownLog(_shutdownReason));
        display::shutdown();
        delay(20);
        esp_deep_sleep_start();
    }
}

bool power::begin(const uint8_t buttonPin) {
    const int gpio = digitalPinToGPIONumber(buttonPin);
    if (gpio < 0) { return false; }

    _buttonPin  = buttonPin;
    _wakeupGpio = static_cast<gpio_num_t>(gpio);

    if (!esp_sleep_is_valid_wakeup_gpio(_wakeupGpio)) {
        _wakeupGpio = GPIO_NUM_NC;
        return false;
    }

    rtc_gpio_deinit(_wakeupGpio);
    pinMode(_buttonPin, INPUT_PULLUP);

    _rawPressed       = digitalRead(_buttonPin) == LOW;
    _stablePressed    = _rawPressed;
    _buttonArmed      = !_stablePressed;
    _lastButtonChange = millis();
    _shutdownPending  = false;
    _initialized      = true;
    return true;
}

void power::update() {
    if (!_initialized) { return; }

    const bool pressed = digitalRead(_buttonPin) == LOW;
    const uint32_t now = millis();

    if (pressed != _rawPressed) {
        _rawPressed      = pressed;
        _lastButtonChange = now;
    }

    if (_rawPressed != _stablePressed && now - _lastButtonChange >= BUTTON_DEBOUNCE_MS) {
        _stablePressed = _rawPressed;
        if (_stablePressed) {
            if (_buttonArmed)
                { shutdown(ShutdownReason::BUTTON); }
        } else { _buttonArmed = true; }
    }
    _tryShutdown();
}

void power::shutdown(const ShutdownReason reason) {
    if (!_initialized) { return; }

    if (_shutdownPending && _shutdownReason == ShutdownReason::BATTERY_CRITICAL && reason == ShutdownReason::BUTTON) {
        _tryShutdown();
        return;
    }

    _shutdownReason  = reason;
    _shutdownPending = true;
    _tryShutdown();
}
