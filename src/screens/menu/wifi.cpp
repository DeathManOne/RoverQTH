/*
 * src/screens/menu/wifi.cpp
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

#include <cstring>
#include "screens/menu/wifi.h"
#include "services/wifi.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "utilities/text.h"

using screens::menu::Wifi;
namespace settings = services::settings;
namespace wifi     = services::wifi;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;
namespace text     = utilities::text;

const char* Wifi::_bootModeToText(settings::WifiBootMode mode) {
    switch (mode) {
        case settings::WifiBootMode::ALWAYS:
            return "Always";
        case settings::WifiBootMode::LAST_STATE:
            return "Last state";
        case settings::WifiBootMode::NEVER:
        default:
            return "Never";
    }
}

void Wifi::_prepareConnectionFields() {
    if (wifi::isConnected()) {
        text::copy(_wifiStatusValue, sizeof(_wifiStatusValue), "Connected");
        text::copy(_wifiLoginValue,  sizeof(_wifiLoginValue),  "Logout");
    } else if (wifi::isConnecting()) {
        text::copy(_wifiStatusValue, sizeof(_wifiStatusValue), "Connecting...");
        text::copy(_wifiLoginValue,  sizeof(_wifiLoginValue),  "Cancel");
    } else {
        text::copy(_wifiStatusValue, sizeof(_wifiStatusValue), "Not connected");
        text::copy(_wifiLoginValue,  sizeof(_wifiLoginValue),  "Login");
    }

    _wifiStatusField.value = _wifiStatusValue;
    _wifiLoginField.value  = _wifiLoginValue;
}

void Wifi::_actionSSID(ST7796S::MSP4021& tft) {
    settings::Wifi configuration = settings::wifi();

    if (configuration.ssid[0] != '\0')
        { tft.KSetText(configuration.ssid); }
    std::memset(configuration.password, 0, sizeof(configuration.password));

    _editingAction = _Action::SSID;
    tft.KDraw("SSID");
    _mode          = Mode::KEYBOARD;
}

void Wifi::_actionPassword(ST7796S::MSP4021 &tft) {
    tft.KSetText("");
    _editingAction = _Action::PASS;

    tft.KDraw("Password");
    _mode = Mode::KEYBOARD;
}

void Wifi::_actionBootMode(ST7796S::MSP4021 &tft) {
    settings::Wifi configuration = settings::wifi();
    settings::WifiBootMode nextMode;

    switch (configuration.bootMode) {
        case settings::WifiBootMode::NEVER:
            nextMode = settings::WifiBootMode::ALWAYS;
            break;
        case settings::WifiBootMode::ALWAYS:
            nextMode = settings::WifiBootMode::LAST_STATE;
            break;
        case settings::WifiBootMode::LAST_STATE:
        default:
            nextMode = settings::WifiBootMode::NEVER;
            break;
    }
    std::memset(configuration.password, 0, sizeof(configuration.password));

    if (!settings::setWifiBootMode(nextMode))
        { return; }
    _wifiBootModeField.value = _bootModeToText(nextMode);

    _updateField(tft, _wifiBootModeField);
}

void Wifi::_updateConnectionFields(ST7796S::MSP4021 &tft) {
    _prepareConnectionFields();
    _updateField(tft, _wifiStatusField);
    _updateField(tft, _wifiLoginField);
}

void Wifi::_actionLogin(ST7796S::MSP4021 &tft) {
    if (wifi::isConnected()) {
        wifi::disconnect();
        settings::setWifiLastEnabled(false);
        _updateConnectionFields(tft);
        return;
    }

    if (wifi::isConnecting()) {
        wifi::disconnect();
        settings::setWifiLastEnabled(false);
        _updateConnectionFields(tft);
        return;
    }

    settings::Wifi configuration = settings::wifi();
    if (configuration.ssid[0] == '\0') {
        std::memset(configuration.password, 0, sizeof(configuration.password));
        return;
    }

    const bool started = wifi::connect(configuration.ssid, configuration.password);
    std::memset(configuration.password, 0, sizeof(configuration.password));

    if (!started) { return; }
    settings::setWifiLastEnabled(true);
    _updateConnectionFields(tft);
}

void Wifi::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()     + (gap * 2);
    const int y     = grid::innerY()     + (gap * 2);
    const int w     = grid::innerWidth() - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    settings::Wifi configuration = settings::wifi();
    if (configuration.ssid[0] != '\0')
        { text::copy(_wifiSsidValue, sizeof(_wifiSsidValue), configuration.ssid); }
    else { text::copy(_wifiSsidValue, sizeof(_wifiSsidValue), "Empty"); }

    text::copy(_wifiPassValue, sizeof(_wifiPassValue), configuration.password[0] != '\0' ? "*****" : "Empty");
    _prepareConnectionFields();

    _wifiSsidField.value     = _wifiSsidValue;
    _wifiPassField.value     = _wifiPassValue;
    _wifiBootModeField.value = _bootModeToText(configuration.bootMode);
    std::memset(configuration.password, 0, sizeof(configuration.password));
    
    _drawTitle(tft, x, y, w, rowH, gap, "WiFi");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

void Wifi::update(ST7796S::MSP4021 &tft) {
    if (_mode == Mode::KEYBOARD) { return; }
    _updateConnectionFields(tft);
}

bool Wifi::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    if (_mode == Mode::KEYBOARD) {
        if (tft.KUpdate(x, y)) {
            const char* value = tft.KRead();
            switch (_editingAction) {
                case _Action::SSID:
                    settings::setWifiSSID(value);
                    break;
                case _Action::PASS:
                    settings::setWifiPassword(value);
                    break;
                case _Action::NONE:
                case _Action::BOOT_MODE:
                case _Action::LOGIN:
                default:
                    break;
            }
            _editingAction = _Action::NONE;
            _mode          = Mode::GRID;
            tft.setTextScale(1);
            return true;
        }
        return true;
    }

    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::SSID:
                _actionSSID(tft);
                return true;
            case _Action::PASS:
                _actionPassword(tft);
                return true;
            case _Action::BOOT_MODE:
                _actionBootMode(tft);
                return true;
            case _Action::LOGIN:
                _actionLogin(tft);
                return true;
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}
