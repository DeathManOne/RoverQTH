/*
 * screens/menu/battery.h
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
    class Battery final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;

        private:
            enum class Action {NONE, CAPACITY, MINIMAL, NOMINAL, MAXIMAL, RATIO_HIGH, RATIO_LOW};

            char capacityValue[16]  = "";
            char minimalValue[16]   = "";
            char nominalValue[16]   = "";
            char maximalValue[16]   = "";
            char ratioHighValue[16] = "";
            char ratioLowValue[16]  = "";

            Field<Action> capacityField  = makeField("Capacity",   Action::CAPACITY,   ui::settings::themes::defaults::GREEN);
            Field<Action> minimalField   = makeField("Minimal",    Action::MINIMAL,    ui::settings::themes::defaults::GREEN);
            Field<Action> nominalField   = makeField("Nominal",    Action::NOMINAL,    ui::settings::themes::defaults::GREEN);
            Field<Action> maximalField   = makeField("Maximal",    Action::MAXIMAL,    ui::settings::themes::defaults::GREEN);
            Field<Action> ratioHighField = makeField("Ratio high", Action::RATIO_HIGH, ui::settings::themes::defaults::GREEN);
            Field<Action> ratioLowField  = makeField("Ratio low",  Action::RATIO_LOW,  ui::settings::themes::defaults::GREEN);

            Field<Action>* fields[6] = {
                &capacityField,
                &minimalField,
                &nominalField,
                &maximalField,
                &ratioHighField,
                &ratioLowField
            };

            static uint32_t nextCapacity(uint32_t value);
            const char* capacityToText(uint32_t value);
            void actionCapacity(ST7796S::MSP4021 &tft, Field<Action> &field);

            static float nextVoltage(float voltage);
            static const char* voltageToText(float voltage, char* buffer, size_t size);
            void actionMinimal(ST7796S::MSP4021 &tft, Field<Action> &field);
            void actionNominal(ST7796S::MSP4021 &tft, Field<Action> &field);
            void actionMaximal(ST7796S::MSP4021 &tft, Field<Action> &field);

            static uint8_t nextRatioHigh(uint8_t value);
            static uint8_t previousRatioHigh(uint8_t value);
            static const char* ratioToText(uint8_t value, char* buffer, size_t size);
            void actionRatio(ST7796S::MSP4021 &tft, Field<Action> &field);
    };
}