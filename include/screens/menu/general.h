/*
 * include/screens/menu/general.h
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
    class General final : public Page {
        public:
            void draw       (ST7796S::MSP4021 &tft) override;
            void update     (ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class _Action {NONE, CALLSIGN, SUFFIX, UNITS, THEME};

            char _callsignValue[32] = "";

            Field<_Action> _callsignField = _makeField("Callsign", _Action::CALLSIGN, ui::settings::themes::defaults::GREEN);
            Field<_Action> _suffixField   = _makeField("Suffix",   _Action::SUFFIX,   ui::settings::themes::defaults::GREEN);
            Field<_Action> _unitsField    = _makeField("Units",    _Action::UNITS,    ui::settings::themes::defaults::GREEN);
            Field<_Action> _themeField    = _makeField("Theme",    _Action::THEME,    ui::settings::themes::defaults::WHITE);

            Field<_Action>* _fields[4] = {
                &_callsignField,
                &_suffixField,
                &_unitsField,
                &_themeField
            };

            void _actionCallsign(ST7796S::MSP4021 &tft);
            static services::settings::CallsignSuffix _nextSuffix(services::settings::CallsignSuffix suffix);
            static const char* _suffixToText(services::settings::CallsignSuffix suffix);

            void _actionSuffix(ST7796S::MSP4021 &tft, Field<_Action> &field);
            static services::settings::Units _nextUnits(services::settings::Units units);
            static const char* _unitsToText(services::settings::Units units);

            void _actionUnits(ST7796S::MSP4021 &tft, Field<_Action> &field);
            static services::settings::Theme _nextTheme(services::settings::Theme theme);
            static const char* _themeToText(services::settings::Theme theme);

            void _actionTheme(ST7796S::MSP4021 &tft, Field<_Action> &field);
    };
}
