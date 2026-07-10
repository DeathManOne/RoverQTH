/*
 * screens/menu/general.h
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
#include "services/settings.h"

namespace screens::menu {
    class General final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
            void update(ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class Action {NONE, CALLSIGN, SUFFIX, UNITS, THEME};

            char callsignValue[32] = "";

            Field<Action> callsignField = makeField("Callsign", Action::CALLSIGN, ui::settings::themes::defaults::GREEN);
            Field<Action> suffixField   = makeField("Suffix",   Action::SUFFIX,   ui::settings::themes::defaults::GREEN);
            Field<Action> unitsField    = makeField("Units",    Action::UNITS,    ui::settings::themes::defaults::GREEN);
            Field<Action> themeField    = makeField("Theme",    Action::THEME,    ui::settings::themes::defaults::WHITE);

            Field<Action>* fields[4] = {
                &callsignField,
                &suffixField,
                &unitsField,
                &themeField
            };

            void actionCallsign(ST7796S::MSP4021 &tft);
            static services::settings::CallsignSuffix nextSuffix(services::settings::CallsignSuffix suffix);
            static const char* suffixToText(services::settings::CallsignSuffix suffix);

            void actionSuffix(ST7796S::MSP4021 &tft, Field<Action> &field);
            static services::settings::Units nextUnits(services::settings::Units units);
            static const char* unitsToText(services::settings::Units units);

            void actionUnits(ST7796S::MSP4021 &tft, Field<Action> &field);
            static services::settings::Theme nextTheme(services::settings::Theme theme);
            static const char* themeToText(services::settings::Theme theme);

            void actionTheme(ST7796S::MSP4021 &tft, Field<Action> &field);
    };
}
