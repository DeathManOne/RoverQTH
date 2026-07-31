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

#include "screens/menu/battery.h"
#include "services/settings.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "utilities/format.h"

using screens::menu::Battery;
namespace settings = services::settings;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;
namespace format   = utilities::format;

float Battery::_nextVoltage(float voltage) {
    voltage += 0.10f;
    if (voltage > 4.30f)
        { return 2.50f; }
    return voltage;
}

float Battery::_nextMinimal(const float minimal, const float maximal) {
    float value = minimal;
    for (uint8_t attempt = 0; attempt < 19U; ++attempt) {
        value = _nextVoltage(value);
        if (value < maximal)
            { return value; }
    }
    return minimal;
}

float Battery::_nextMaximal(const float maximal, const float minimal) {
    float value = maximal;
    for (uint8_t attempt = 0; attempt < 19U; ++attempt) {
        value = _nextVoltage(value);
        if (value > minimal)
            { return value; }
    }
    return maximal;
}

void Battery::_actionMinimal(ST7796S::MSP4021 &tft, Field<_Action> &field) {
   const settings::Battery configuration = settings::battery();
   const float minimal = _nextMinimal(configuration.minimal, configuration.maximal);

    if (minimal == configuration.minimal)      { return; }
    if (!settings::setBatteryMinimal(minimal)) { return; }

    format::voltage(minimal, _minimalValue, sizeof(_minimalValue));
    field.value = _minimalValue;
    _updateField(tft, field);
}

void Battery::_actionMaximal(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const settings::Battery configuration = settings::battery();
    const float maximal = _nextMaximal(configuration.maximal, configuration.minimal);

    if (maximal == configuration.maximal)      { return; }
    if (!settings::setBatteryMaximal(maximal)) { return; }

    format::voltage(maximal, _maximalValue, sizeof(_maximalValue));
    field.value = _maximalValue;
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

void Battery::_actionRatio(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const settings::Battery configuration = settings::battery();
    uint8_t value                         = configuration.ratioHigh;

    if (field.action == _Action::RATIO_HIGH)
        { value = _nextRatioHigh(value); }
    else { value = _previousRatioHigh(value); }

    if (!settings::setBatteryRatioHigh(value)) { return; }
    format::percentage(value,        _ratioHighValue, sizeof(_ratioHighValue));
    format::percentage(100U - value, _ratioLowValue,  sizeof(_ratioLowValue));

    _ratioHighField.value = _ratioHighValue;
    _ratioLowField.value  = _ratioLowValue;

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

    const settings::Battery configuration = settings::battery();

    const float minimal     = configuration.minimal;
    const float maximal     = configuration.maximal;
    const uint8_t ratioHigh = configuration.ratioHigh;
    const uint8_t ratioLow  = 100U - ratioHigh;

    format::voltage   (minimal,   _minimalValue,   sizeof(_minimalValue));
    format::voltage   (maximal,   _maximalValue,   sizeof(_maximalValue));
    format::percentage(ratioHigh, _ratioHighValue, sizeof(_ratioHighValue));
    format::percentage(ratioLow,  _ratioLowValue,  sizeof(_ratioLowValue));

    _minimalField.value   = _minimalValue;
    _maximalField.value   = _maximalValue;
    _ratioHighField.value = _ratioHighValue;
    _ratioLowField.value  = _ratioLowValue;

    _drawTitle(tft, x, y, w, rowH, gap, "battery");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

bool Battery::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::MINIMAL:
                _actionMinimal(tft, *field);
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
