/*
 * screens/menu/general.cpp
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
#include "screens/mockup/grid.h"

namespace screens::menu {
    void General::actionCallsign(ST7796S::MSP4021 &tft) {
        char callsign[32] = "";
        if (services::settings::getCallsign(callsign, sizeof(callsign)))
            { tft.KSetText(callsign); }
        tft.KDraw("Callsign");
        mode = Mode::KEYBOARD;
    }

    services::settings::CallsignSuffix General::nextSuffix(services::settings::CallsignSuffix suffix) {
        switch (suffix) {
            case services::settings::CallsignSuffix::NONE:
                return services::settings::CallsignSuffix::P;
            case services::settings::CallsignSuffix::P:
                return services::settings::CallsignSuffix::M;
            case services::settings::CallsignSuffix::M:
                return services::settings::CallsignSuffix::MM;
            case services::settings::CallsignSuffix::MM:
                return services::settings::CallsignSuffix::AM;
            case services::settings::CallsignSuffix::AM:
            default:
                return services::settings::CallsignSuffix::NONE;
        }
    }

    const char* General::suffixToText(services::settings::CallsignSuffix suffix) {
        switch (suffix) {
            case services::settings::CallsignSuffix::P:
                return "/P";
            case services::settings::CallsignSuffix::M:
                return "/M";
            case services::settings::CallsignSuffix::MM:
                return "/MM";
            case services::settings::CallsignSuffix::AM:
                return "/AM";
            case services::settings::CallsignSuffix::NONE:
            default:
                return "None";
        }
    }

    void General::actionSuffix(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const services::settings::CallsignSuffix suffix = nextSuffix(services::settings::getCallsignSuffix());
        services::settings::setCallsignSuffix(suffix);
        field.value = suffixToText(suffix);
        updateField(tft, field);
    }

    services::settings::Units General::nextUnits(services::settings::Units units) {
        switch (units) {
            case services::settings::Units::METRIC:
                return services::settings::Units::IMPERIAL;
            case services::settings::Units::IMPERIAL:
            default:
                return services::settings::Units::METRIC;
        }
    }

    const char* General::unitsToText(services::settings::Units units) {
        switch (units) {
            case services::settings::Units::IMPERIAL:
                return "Imperial";
            case services::settings::Units::METRIC:
            default:
                return "Metric";
        }
    }

    void General::actionUnits(ST7796S::MSP4021 &tft, Field<Action> &field) {
        const services::settings::Units units = nextUnits(services::settings::getUnits());
        services::settings::setUnits(units);
        field.value = unitsToText(units);
        updateField(tft, field);
    }

    services::settings::Theme General::nextTheme(services::settings::Theme theme) {
        switch (theme) {
            case services::settings::Theme::DEFAULTS:
                return services::settings::Theme::NIGHT;
            case services::settings::Theme::NIGHT:
                return services::settings::Theme::HIGHS;
            case services::settings::Theme::HIGHS:
            default:
                return services::settings::Theme::DEFAULTS;
        }
    }

    const char* General::themeToText(services::settings::Theme theme) {
        switch (theme) {
            case services::settings::Theme::NIGHT:
                return "Night";
            case services::settings::Theme::HIGHS:
                return "High";
            case services::settings::Theme::DEFAULTS:
            default:
                return "Default";
        }
    }

    void General::actionTheme(ST7796S::MSP4021 &tft, Field<Action> &field) {
        return; // TODO: Feature temporarily disabled.
        const services::settings::Theme theme = nextTheme(services::settings::getTheme());
        services::settings::setTheme(theme);
        field.value = themeToText(theme);
        updateField(tft, field);
    }

    void General::draw(ST7796S::MSP4021 &tft) {
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

        if (!services::settings::getCallsign(callsignValue, sizeof(callsignValue)))
            { std::strcpy(callsignValue, "ERROR"); }

        const services::settings::CallsignSuffix suffix = services::settings::getCallsignSuffix();
        const services::settings::Units units           = services::settings::getUnits();
        const services::settings::Theme theme           = services::settings::getTheme();

        callsignField.value = callsignValue;
        suffixField.value   = suffixToText(suffix);
        unitsField.value    = unitsToText(units);
        themeField.value    = themeToText(theme);
        
        drawTitle(tft, x, y, w, rowH, gap, "general");
        for (Field<Action>* field : fields)
            { drawLine(tft, *field); }
    }

    void General::update(ST7796S::MSP4021 &tft) {
        char callsign[32] = "";
        if (!services::settings::getFullCallsign(callsign, sizeof(callsign)))
            { std::strcpy(callsign, "ERROR"); }
        screens::main::title::updateCallsign(tft, callsign);
    }

    bool General::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
        if (mode == Mode::KEYBOARD) {
            if (tft.KUpdate(x, y)) {
                const char* value = tft.KRead();
                services::settings::setCallsign(value);
                mode = Mode::GRID;
                tft.setTextScale(1);
                return true;
            }
            return true;
        }
        for (Field<Action>* field : fields) {
            if (!isPressed(*field, x, y))
                { continue; }
            switch (field->action) {
                case Action::CALLSIGN:
                    actionCallsign(tft);
                    return true;
                case Action::SUFFIX:
                    actionSuffix(tft, *field);
                    return true;
                case Action::UNITS:
                    actionUnits(tft, *field);
                    return true;
                case Action::THEME:
                    actionTheme(tft, *field);
                    return true;
                case Action::NONE:
                default:
                    return false;
            }
        }
        return false;
    }
}
