/*
 * screens/menu/battery.cpp
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

#include <cstdio>
#include "screens/menu/battery.h"
#include "screens/mockup/grid.h"
#include "services/settings.h"

namespace screens::menu {
    uint32_t Battery::nextCapacity(uint32_t value) {
        value += 500;
        if (value > 10000)
            { return 500; }
        return value;
    }

    const char* Battery::capacityToText(uint32_t value) {
        std::snprintf(
            capacityValue,  sizeof(capacityValue),
            "%lu mAh",      static_cast<unsigned long>(value)
        );
        return capacityValue;
    }

    void Battery::actionCapacity(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const uint32_t capacity = nextCapacity(services::settings::getBatteryCapacity());
        services::settings::setBatteryCapacity(capacity);
        field.value = capacityToText(capacity);
        updateField(tft, field);
    }

    float Battery::nextVoltage(float voltage) {
        voltage += 0.10f;
        if (voltage > 4.30f)
            { return 2.50f; }
        return voltage;
    }

    const char* Battery::voltageToText(float voltage, char* buffer, size_t size) {
        std::snprintf(buffer, size, "%.2f V", voltage);
        return buffer;
    }

    void Battery::actionMinimal(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const float minimal = nextVoltage(services::settings::getBatteryMinimal());
        services::settings::setBatteryMinimal(minimal);
        field.value = voltageToText(minimal, minimalValue, sizeof(minimalValue));
        updateField(tft, field);
    }

    void Battery::actionNominal(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const float nominal = nextVoltage(services::settings::getBatteryNominal());
        services::settings::setBatteryNominal(nominal);
        field.value = voltageToText(nominal, nominalValue, sizeof(nominalValue));
        updateField(tft, field);
    }

    void Battery::actionMaximal(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const float maximal = nextVoltage(services::settings::getBatteryMaximal());
        services::settings::setBatteryMaximal(maximal);
        field.value = voltageToText(maximal, maximalValue, sizeof(maximalValue));
        updateField(tft, field);
    }

    uint8_t Battery::nextRatioHigh(uint8_t value) {
        if (value >= 99)
            { return 1; }
        return value + 1;
    }

    uint8_t Battery::previousRatioHigh(uint8_t value) {
        if (value <= 1)
            { return 99; }
        return value - 1;
    }

    const char* Battery::ratioToText(uint8_t value, char* buffer, size_t size) {
        std::snprintf(buffer, size, "%u %%", value);
        return buffer;
    }

    void Battery::actionRatio(ST7796S::MSP4021 &tft, Field<Action> &field) {
        uint8_t value = services::settings::getBatteryRatioHigh();

        if (field.action == Action::RATIO_HIGH)
            { value = nextRatioHigh(value); }
        else { value = previousRatioHigh(value); }

        services::settings::setBatteryRatioHigh(value);
        ratioHighField.value = ratioToText(value,       ratioHighValue, sizeof(ratioHighValue));
        ratioLowField.value  = ratioToText(100 - value, ratioLowValue,  sizeof(ratioLowValue));
        updateField(tft, ratioHighField);
        updateField(tft, ratioLowField);
    }

    void Battery::draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field<Action>* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        const uint32_t capacity = services::settings::getBatteryCapacity();
        const float minimal     = services::settings::getBatteryMinimal();
        const float nominal     = services::settings::getBatteryNominal();
        const float maximal     = services::settings::getBatteryMaximal();
        const uint8_t ratioHigh = services::settings::getBatteryRatioHigh();

        capacityField.value  = capacityToText(capacity);
        minimalField.value   = voltageToText(minimal,       minimalValue,   sizeof(minimalValue));
        nominalField.value   = voltageToText(nominal,       nominalValue,   sizeof(nominalValue));
        maximalField.value   = voltageToText(maximal,       maximalValue,   sizeof(maximalValue));
        ratioHighField.value = ratioToText(ratioHigh,       ratioHighValue, sizeof(ratioHighValue));
        ratioLowField.value  = ratioToText(100 - ratioHigh, ratioLowValue,  sizeof(ratioLowValue));

        drawTitle(tft, x, y, w, rowH, gap, "battery");
        for (Field<Action>* field : fields)
            { drawLine(tft, *field); }
    }

    bool Battery::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
        for (Field<Action>* field : fields) {
            if (!isPressed(*field, x, y))
                { continue; }
            switch (field->action) {
                case Action::CAPACITY:
                    actionCapacity(tft, *field);
                    return true;
                case Action::MINIMAL:
                    actionMinimal(tft, *field);
                    return true;
                case Action::NOMINAL:
                    actionNominal(tft, *field);
                    return true;
                case Action::MAXIMAL:
                    actionMaximal(tft, *field);
                    return true;
                case Action::RATIO_HIGH:
                case Action::RATIO_LOW:
                    actionRatio(tft, *field);
                    return true;
                case Action::NONE:
                default:
                    return false;
            }
        }
        return false;
    }
}