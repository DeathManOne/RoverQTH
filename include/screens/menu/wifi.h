/*
 * include/screens/menu/wifi.h
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

#pragma once

#include <MSP4021.h>
#include "screens/menu/page.h"
#include "services/settings.h"

namespace screens::menu {
    class Wifi final : public Page {
        public:
            void draw       (ST7796S::MSP4021 &tft) override;
            void update     (ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class _Action {NONE, BOOT_MODE, SSID, PASS, LOGIN};
            _Action _editingAction = _Action::NONE;

            char _wifiStatusValue[32] = "";
            char _wifiSsidValue[64]   = "";
            char _wifiPassValue[64]   = "";
            char _wifiLoginValue[32]  = "";

            Field<_Action> _wifiStatusField   = _makeField("Status",          _Action::NONE,      ui::settings::themes::defaults::WHITE);
            Field<_Action> _wifiSsidField     = _makeField("SSID",            _Action::SSID,      ui::settings::themes::defaults::GREEN);
            Field<_Action> _wifiPassField     = _makeField("Password",        _Action::PASS,      ui::settings::themes::defaults::GREEN);
            Field<_Action> _wifiBootModeField = _makeField("Connect at boot", _Action::BOOT_MODE, ui::settings::themes::defaults::GREEN);
            Field<_Action> _wifiLoginField    = _makeField("Login",           _Action::LOGIN,     ui::settings::themes::defaults::GREEN);

            Field<_Action>* _fields[5] = {
                &_wifiStatusField,
                &_wifiSsidField,
                &_wifiPassField,
                &_wifiBootModeField,
                &_wifiLoginField
            };

            static const char* _bootModeToText(services::settings::WifiBootMode mode);
            void _prepareConnectionFields();
            void _updateConnectionFields(ST7796S::MSP4021 &tft);
            void _actionSSID            (ST7796S::MSP4021 &tft);
            void _actionPassword        (ST7796S::MSP4021 &tft);
            void _actionBootMode        (ST7796S::MSP4021 &tft);
            void _actionLogin           (ST7796S::MSP4021 &tft);
    };
}