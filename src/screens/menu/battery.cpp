/*
 * src/screens/menu/battery.cpp
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
#include "services/settings.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::Battery;
namespace settings = services::settings;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

uint32_t Battery::_nextCapacity(uint32_t value) {
    value += 500;
    if (value > 10000)
        { return 500; }
    return value;
}

const char* Battery::_capacityToText(uint32_t value) {
    std::snprintf(
        _capacityValue,  sizeof(_capacityValue),
        "%lu mAh",      static_cast<unsigned long>(value)
    );
    return _capacityValue;
}

void Battery::_actionCapacity(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const uint32_t capacity = _nextCapacity(settings::getBatteryCapacity());
    settings::setBatteryCapacity(capacity);
    field.value = _capacityToText(capacity);
    _updateField(tft, field);
}

float Battery::_nextVoltage(float voltage) {
    voltage += 0.10f;
    if (voltage > 4.30f)
        { return 2.50f; }
    return voltage;
}

const char* Battery::_voltageToText(float voltage, char* buffer, size_t size) {
    std::snprintf(buffer, size, "%.2f V", voltage);
    return buffer;
}

void Battery::_actionMinimal(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const float minimal = _nextVoltage(settings::getBatteryMinimal());
    settings::setBatteryMinimal(minimal);
    field.value = _voltageToText(minimal, _minimalValue, sizeof(_minimalValue));
    _updateField(tft, field);
}

void Battery::_actionNominal(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const float nominal = _nextVoltage(settings::getBatteryNominal());
    settings::setBatteryNominal(nominal);
    field.value = _voltageToText(nominal, _nominalValue, sizeof(_nominalValue));
    _updateField(tft, field);
}

void Battery::_actionMaximal(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const float maximal = _nextVoltage(settings::getBatteryMaximal());
    settings::setBatteryMaximal(maximal);
    field.value = _voltageToText(maximal, _maximalValue, sizeof(_maximalValue));
    _updateField(tft, field);
}

uint8_t Battery::_nextRatioHigh(uint8_t value) {
    if (value >= 99)
        { return 1; }
    return value + 1;
}

uint8_t Battery::_previousRatioHigh(uint8_t value) {
    if (value <= 1)
        { return 99; }
    return value - 1;
}

const char* Battery::_ratioToText(uint8_t value, char* buffer, size_t size) {
    std::snprintf(buffer, size, "%u %%", value);
    return buffer;
}

void Battery::_actionRatio(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    uint8_t value = settings::getBatteryRatioHigh();

    if (field.action == _Action::RATIO_HIGH)
        { value = _nextRatioHigh(value); }
    else { value = _previousRatioHigh(value); }

    settings::setBatteryRatioHigh(value);
    _ratioHighField.value = _ratioToText(value,       _ratioHighValue, sizeof(_ratioHighValue));
    _ratioLowField.value  = _ratioToText(100 - value, _ratioLowValue,  sizeof(_ratioLowValue));
    _updateField(tft, _ratioHighField);
    _updateField(tft, _ratioLowField);
}

void Battery::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()       + (gap * 2);
    const int y     = grid::innerY()       + (gap * 2);
    const int w     = grid::innerWidth()   - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    const uint32_t capacity = settings::getBatteryCapacity();
    const float minimal     = settings::getBatteryMinimal();
    const float nominal     = settings::getBatteryNominal();
    const float maximal     = settings::getBatteryMaximal();
    const uint8_t ratioHigh = settings::getBatteryRatioHigh();

    _capacityField.value  = _capacityToText(capacity);
    _minimalField.value   = _voltageToText(minimal,       _minimalValue,   sizeof(_minimalValue));
    _nominalField.value   = _voltageToText(nominal,       _nominalValue,   sizeof(_nominalValue));
    _maximalField.value   = _voltageToText(maximal,       _maximalValue,   sizeof(_maximalValue));
    _ratioHighField.value = _ratioToText(ratioHigh,       _ratioHighValue, sizeof(_ratioHighValue));
    _ratioLowField.value  = _ratioToText(100 - ratioHigh, _ratioLowValue,  sizeof(_ratioLowValue));

    _drawTitle(tft, x, y, w, rowH, gap, "battery");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

bool Battery::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::CAPACITY:
                _actionCapacity(tft, *field);
                return true;
            case _Action::MINIMAL:
                _actionMinimal(tft, *field);
                return true;
            case _Action::NOMINAL:
                _actionNominal(tft, *field);
                return true;
            case _Action::MAXIMAL:
                _actionMaximal(tft, *field);
                return true;
            case _Action::RATIO_HIGH:
            case _Action::RATIO_LOW:
                _actionRatio(tft, *field);
                return true;
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}
