/*
 * src/screens/menu/general.cpp
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

#include "screens/main/title.h"
#include "screens/menu/general.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::General;
namespace title    = screens::main::title;
namespace settings = services::settings;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

void General::_actionCallsign(ST7796S::MSP4021 &tft) {
    char callsign[32] = "";
    if (settings::getCallsign(callsign, sizeof(callsign)))
        { tft.KSetText(callsign); }
    tft.KDraw("Callsign");
    _mode = Mode::KEYBOARD;
}

settings::CallsignSuffix General::_nextSuffix(settings::CallsignSuffix suffix) {
    switch (suffix) {
        case settings::CallsignSuffix::NONE: return settings::CallsignSuffix::P;
        case settings::CallsignSuffix::P:    return settings::CallsignSuffix::M;
        case settings::CallsignSuffix::M:    return settings::CallsignSuffix::MM;
        case settings::CallsignSuffix::MM:   return settings::CallsignSuffix::AM;
        case settings::CallsignSuffix::AM:
        default:                             return settings::CallsignSuffix::NONE;
    }
}

const char* General::_suffixToText(settings::CallsignSuffix suffix) {
    switch (suffix) {
        case settings::CallsignSuffix::P:    return "/P";
        case settings::CallsignSuffix::M:    return "/M";
        case settings::CallsignSuffix::MM:   return "/MM";
        case settings::CallsignSuffix::AM:   return "/AM";
        case settings::CallsignSuffix::NONE:
        default:                             return "None";
    }
}

void General::_actionSuffix(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const settings::CallsignSuffix suffix = _nextSuffix(settings::getCallsignSuffix());
    if (!settings::setCallsignSuffix(suffix)) { return; }

    field.value = _suffixToText(suffix);
    _updateField(tft, field);
}

settings::Units General::_nextUnits(settings::Units units) {
    switch (units) {
        case settings::Units::METRIC:   return settings::Units::IMPERIAL;
        case settings::Units::IMPERIAL:
        default:                        return settings::Units::METRIC;
    }
}

const char* General::_unitsToText(settings::Units units) {
    switch (units) {
        case settings::Units::IMPERIAL: return "Imperial";
        case settings::Units::METRIC:
        default:                        return "Metric";
    }
}

void General::_actionUnits(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const settings::Units units = _nextUnits(settings::getUnits());
    if (!settings::setUnits(units)) { return; }

    field.value = _unitsToText(units);
    _updateField(tft, field);
}

settings::CoordinateFormat General::_nextCoordinateFormat(settings::CoordinateFormat format) {
    switch (format) {
        case settings::CoordinateFormat::DD:  return settings::CoordinateFormat::DDM;
        case settings::CoordinateFormat::DDM: return settings::CoordinateFormat::DMS;
        case settings::CoordinateFormat::DMS:
        default:                              return settings::CoordinateFormat::DD;
    }
}

const char* General::_coordinateFormatToText(settings::CoordinateFormat format) {
    switch (format) {
        case settings::CoordinateFormat::DD:  return "DD";
        case settings::CoordinateFormat::DMS: return "DMS";
        case settings::CoordinateFormat::DDM:
        default:                              return "DDM";
    }
}

void General::_actionCoordinateFormat(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    const settings::CoordinateFormat format = _nextCoordinateFormat(settings::getCoordinateFormat());
    if (!settings::setCoordinateFormat(format)) { return; }

    field.value = _coordinateFormatToText(format);
    _updateField(tft, field);
}

settings::Theme General::_nextTheme(settings::Theme theme) {
    switch (theme) {
        case settings::Theme::DEFAULTS: return settings::Theme::NIGHT;
        case settings::Theme::NIGHT:    return settings::Theme::HIGHS;
        case settings::Theme::HIGHS:
        default:                        return settings::Theme::DEFAULTS;
    }
}

const char* General::_themeToText(settings::Theme theme) {
    switch (theme) {
        case settings::Theme::NIGHT:    return "Night";
        case settings::Theme::HIGHS:    return "High";
        case settings::Theme::DEFAULTS:
        default:                        return "Default";
    }
}

void General::_actionTheme(ST7796S::MSP4021 &tft, Field<_Action> &field) {
    return; // TODO: Feature temporarily disabled.
    const settings::Theme theme = _nextTheme(settings::getTheme());
    if (!settings::setTheme(theme)) { return; }

    field.value = _themeToText(theme);
    _updateField(tft, field);
}

void General::draw(ST7796S::MSP4021 &tft) {
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

    if (!settings::getCallsign(_callsignValue, sizeof(_callsignValue)))
        { std::strcpy(_callsignValue, "ERROR"); }

    const settings::CallsignSuffix suffix             = settings::getCallsignSuffix();
    const settings::Units units                       = settings::getUnits();
    const settings::Theme theme                       = settings::getTheme();
    const settings::CoordinateFormat coordinateFormat = settings::getCoordinateFormat();

    _callsignField.value   = _callsignValue;
    _suffixField.value     = _suffixToText(suffix);
    _unitsField.value      = _unitsToText(units);
    _coordinateField.value = _coordinateFormatToText(coordinateFormat);
    _themeField.value      = _themeToText(theme);
    
    _drawTitle(tft, x, y, w, rowH, gap, "general");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

void General::update(ST7796S::MSP4021 &tft) {
    char callsign[32] = "";
    if (!settings::getFullCallsign(callsign, sizeof(callsign)))
        { std::strcpy(callsign, "ERROR"); }
    title::updateCallsign(tft, callsign);
}

bool General::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    if (_mode == Mode::KEYBOARD) {
        if (tft.KUpdate(x, y)) {
            const char* value = tft.KRead();
            settings::setCallsign(value);
            _mode = Mode::GRID;
            tft.setTextScale(1);
            return true;
        }
        return true;
    }
    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::CALLSIGN:
                _actionCallsign(tft);
                return true;
            case _Action::SUFFIX:
                _actionSuffix(tft, *field);
                return true;
            case _Action::UNITS:
                _actionUnits(tft, *field);
                return true;
            case _Action::COORDINATES:
                _actionCoordinateFormat(tft, *field);
                return true;
            case _Action::THEME:
                _actionTheme(tft, *field);
                return true;
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}
