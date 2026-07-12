/*
 * include/screens/menu/battery.h
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

namespace screens::menu {
    class Battery final : public Page {
        public:
            void draw       (ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class _Action {NONE, CAPACITY, MINIMAL, NOMINAL, MAXIMAL, RATIO_HIGH, RATIO_LOW};

            char _capacityValue[16]  = "";
            char _minimalValue[16]   = "";
            char _nominalValue[16]   = "";
            char _maximalValue[16]   = "";
            char _ratioHighValue[16] = "";
            char _ratioLowValue[16]  = "";

            Field<_Action> _capacityField  = _makeField("Capacity",   _Action::CAPACITY,   ui::settings::themes::defaults::GREEN);
            Field<_Action> _minimalField   = _makeField("Minimal",    _Action::MINIMAL,    ui::settings::themes::defaults::GREEN);
            Field<_Action> _nominalField   = _makeField("Nominal",    _Action::NOMINAL,    ui::settings::themes::defaults::GREEN);
            Field<_Action> _maximalField   = _makeField("Maximal",    _Action::MAXIMAL,    ui::settings::themes::defaults::GREEN);
            Field<_Action> _ratioHighField = _makeField("Ratio high", _Action::RATIO_HIGH, ui::settings::themes::defaults::GREEN);
            Field<_Action> _ratioLowField  = _makeField("Ratio low",  _Action::RATIO_LOW,  ui::settings::themes::defaults::GREEN);

            Field<_Action>* _fields[6] = {
                &_capacityField,
                &_minimalField,
                &_nominalField,
                &_maximalField,
                &_ratioHighField,
                &_ratioLowField
            };

            static uint32_t _nextCapacity(uint32_t value);
            const char* _capacityToText(uint32_t value);
            void _actionCapacity(ST7796S::MSP4021 &tft, Field<_Action> &field);

            static float _nextVoltage(float voltage);
            static const char* _voltageToText(float voltage, char* buffer, size_t size);
            void _actionMinimal(ST7796S::MSP4021 &tft, Field<_Action> &field);
            void _actionNominal(ST7796S::MSP4021 &tft, Field<_Action> &field);
            void _actionMaximal(ST7796S::MSP4021 &tft, Field<_Action> &field);

            static uint8_t _nextRatioHigh(uint8_t value);
            static uint8_t _previousRatioHigh(uint8_t value);
            static const char* _ratioToText(uint8_t value, char* buffer, size_t size);
            void _actionRatio(ST7796S::MSP4021 &tft, Field<_Action> &field);
    };
}