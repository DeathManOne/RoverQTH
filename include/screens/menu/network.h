/*
 * screens/menu/network.h
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

#include "screens/menu/page.h"

namespace screens::menu {
    class Network final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class Action {NONE, WIFI_SSID, WIFI_PASS, BT_NAME, BT_PASS};

            char wifiSsidValue[32]  = "";
            char wifiPassValue[32]  = "";
            char btNameValue[32]    = "";
            char btPassValue[32]    = "";

            Field<Action> wifiSsidField = makeField("WiFi SSID",      Action::WIFI_SSID, ui::settings::themes::defaults::WHITE);
            Field<Action> wifiPassField = makeField("WiFi password",  Action::WIFI_PASS, ui::settings::themes::defaults::WHITE);
            Field<Action> btNameField   = makeField("Bluetooth name", Action::BT_NAME,   ui::settings::themes::defaults::WHITE);
            Field<Action> btPassField   = makeField("Bluetooth pin",  Action::BT_PASS,   ui::settings::themes::defaults::WHITE);

            Field<Action>* fields[4] = {
                &wifiSsidField,
                &wifiPassField,
                &btNameField,
                &btPassField
            };
    };
}