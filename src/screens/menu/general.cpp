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
#include "display/manager.h"
#include "screens/main/title.h"
#include "screens/menu/general.h"
#include "screens/mockup/grid.h"
#include "services/settings.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace screens::menu::general {
    namespace {
        enum class Mode {GRID, KEYBOARD};
        Mode mode = Mode::GRID;

        enum class Action {
            NONE,
            CALLSIGN,
            SUFFIX,
            UNITS,
            THEME,
            ROTATION,
            CALIBRATION
        };

        struct Area {
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;
        };

        struct Field {
            Area area;
            const char* label   = nullptr;
            const char* value   = nullptr;
            Action action       = Action::NONE;
            uint16_t color      = ui::settings::themes::defaults::WHITE;
        };

        Field makeField(const char* label, Action action, uint16_t color);
        void makeFieldArea(Field &field, int x, int y, int w, int h);
        void updateField(ST7796S::MSP4021 &tft, const Field &field);
        bool isPressed(const Field &field, int tx, int ty);
        void drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap);
        void drawLine(ST7796S::MSP4021 &tft, const Field &field);
        void actionCallsign(ST7796S::MSP4021 &tft);
        services::settings::CallsignSuffix nextSuffix(services::settings::CallsignSuffix suffix);
        const char* suffixToText(services::settings::CallsignSuffix suffix);
        void actionSuffix(ST7796S::MSP4021 &tft, Field &field);
        services::settings::Units nextUnits(services::settings::Units units);
        const char* unitsToText(services::settings::Units units);
        void actionUnits(ST7796S::MSP4021 &tft, Field &field);
        services::settings::Theme nextTheme(services::settings::Theme theme);
        const char* themeToText(services::settings::Theme theme);
        void actionTheme(ST7796S::MSP4021 &tft, Field &field);
        services::settings::TFTRotation nextRotation(services::settings::TFTRotation rotation);
        const char* rotationToText(services::settings::TFTRotation rotation);
        void actionRotation(ST7796S::MSP4021 &tft, Field &field);
        void actionCalibration(ST7796S::MSP4021 &tft, Field &field);
        
        Field callsignField    = makeField("Callsign",    Action::CALLSIGN,       ui::settings::themes::defaults::GREEN);
        Field suffixField      = makeField("Suffix",      Action::SUFFIX,         ui::settings::themes::defaults::GREEN);
        Field unitsField       = makeField("Units",       Action::UNITS,          ui::settings::themes::defaults::WHITE);
        Field themeField       = makeField("Theme",       Action::THEME,          ui::settings::themes::defaults::WHITE);
        Field rotationField    = makeField("Rotation",    Action::ROTATION,       ui::settings::themes::defaults::WHITE);
        Field calibrationField = makeField("Calibration", Action::CALIBRATION,    ui::settings::themes::defaults::WHITE);

        Field* fields[] = {
            &callsignField,
            &suffixField,
            &unitsField,
            &themeField,
            &rotationField,
            &calibrationField
        };

        Field makeField(const char* label, Action action, uint16_t color) {
            Field field;
            field.label     = label;
            field.action    = action;
            field.color     = color;
            return field;
        }

        void makeFieldArea(Field &field, int x, int y, int w, int h) {
            Area area;
            area.x = x;
            area.y = y;
            area.w = w;
            area.h = h;
            field.area = area;
        }

        void updateField(ST7796S::MSP4021 &tft, const Field &field) {
            const int gap = ui::settings::mockup::GAP;

            tft.rectFill(
                field.area.x + (field.area.w / 2),  field.area.y,
                (field.area.w / 2) - gap,           field.area.h,
                ui::settings::themes::defaults::BLACK
            );

            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(field.color);
            tft.textCenterRight(
                field.area.x + (field.area.w / 2),  field.area.y,
                (field.area.w / 2) - gap,           field.area.h,
                field.value
            );
        }

        bool isPressed(const Field &field, int tx, int ty) {
            return
                tx >= field.area.x && tx < field.area.x + field.area.w &&
                ty >= field.area.y && ty < field.area.y + field.area.h;
        }

        void drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap) {
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(ui::settings::themes::defaults::GREEN);
            tft.textCenterLeft(x, y, w, h, "GENERAL");
            tft.lineH(x, y + h + gap, w, ui::settings::themes::defaults::BORDER);
        }

        void drawLine(ST7796S::MSP4021 &tft, const Field &field) {
            const int gap = ui::settings::mockup::GAP;

            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textCenterLeft(
                field.area.x,        field.area.y,
                field.area.w / 2,    field.area.h,
                field.label
            );

            tft.setTextColor(field.color);
            tft.textCenterRight(
                field.area.x + (field.area.w / 2),  field.area.y,
                (field.area.w / 2) - gap,           field.area.h,
                field.value
            );
        }

        void actionCallsign(ST7796S::MSP4021 &tft) {
            char callsign[32];
            if (services::settings::getCallsign(callsign, sizeof(callsign)))
                { tft.KSetText(callsign); }
            tft.KDraw("Callsign");
            mode = Mode::KEYBOARD;
        }

        services::settings::CallsignSuffix nextSuffix(services::settings::CallsignSuffix suffix) {
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

        const char* suffixToText(services::settings::CallsignSuffix suffix) {
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

        void actionSuffix(ST7796S::MSP4021 &tft, Field &field) {
            services::settings::CallsignSuffix suffix = nextSuffix(services::settings::getCallsignSuffix());
            services::settings::setCallsignSuffix(suffix);
            field.value = suffixToText(suffix);
            updateField(tft, field);
        }

        services::settings::Units nextUnits(services::settings::Units units) {
            switch (units) {
                case services::settings::Units::METRIC:
                    return services::settings::Units::IMPERIAL;
                case services::settings::Units::IMPERIAL:
                default:
                    return services::settings::Units::METRIC;
            }
        }

        const char* unitsToText(services::settings::Units units) {
            switch (units) {
                case services::settings::Units::IMPERIAL:
                    return "Imperial";
                case services::settings::Units::METRIC:
                default:
                    return "Metric";
            }
        }

        void actionUnits(ST7796S::MSP4021 &tft, Field &field) {
            return; // TODO: Feature temporarily disabled
            services::settings::Units units = nextUnits(services::settings::getUnits());
            services::settings::setUnits(units);
            field.value = unitsToText(units);
            updateField(tft, field);
        }

        services::settings::Theme nextTheme(services::settings::Theme theme) {
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

        const char* themeToText(services::settings::Theme theme) {
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

        void actionTheme(ST7796S::MSP4021 &tft, Field &field) {
            return; // TODO: Feature temporarily disabled
            services::settings::Theme theme = nextTheme(services::settings::getTheme());
            services::settings::setTheme(theme);
            field.value = themeToText(theme);
            updateField(tft, field);
        }

        services::settings::TFTRotation nextRotation(services::settings::TFTRotation rotation) {
            switch (rotation) {
                case services::settings::TFTRotation::NORMAL:
                    return services::settings::TFTRotation::REVERSED;
                case services::settings::TFTRotation::REVERSED:
                default:
                    return services::settings::TFTRotation::NORMAL;
            }
        }

        const char* rotationToText(services::settings::TFTRotation rotation) {
            switch (rotation) {
                case services::settings::TFTRotation::REVERSED:
                    return "Reversed";
                case services::settings::TFTRotation::NORMAL:
                default:
                    return "Normal";
            }
        }

        void actionRotation(ST7796S::MSP4021 &tft, Field &field) {
            return; // TODO: Feature temporarily disabled
            services::settings::TFTRotation rotation = nextRotation(services::settings::getTFTRotation());
            services::settings::setTFTRotation(rotation);
            field.value = rotationToText(rotation);
            tft.setRotation(static_cast<uint8_t>(services::settings::getTFTRotation()));
            // redraw all screen
        }

        void actionCalibration(ST7796S::MSP4021 &tft, Field &field) {
            return; // TODO: Feature temporarily disabled
            services::settings::resetTouchCalibration();
            display::TCalibrate();
        }
    }

    bool isEditing()    { return mode == Mode::KEYBOARD; }
    void reset()        { mode = Mode::GRID; }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        char callsign[32];
        if (!services::settings::getCallsign(callsign, sizeof(callsign)))
            { strcpy(callsign, "ERROR"); }
        const auto suffix   = services::settings::getCallsignSuffix();
        const auto units    = services::settings::getUnits();
        const auto theme    = services::settings::getTheme();
        const auto rotation = services::settings::getTFTRotation();

        callsignField.value     = callsign;
        suffixField.value       = suffixToText(suffix);
        unitsField.value        = unitsToText(units);
        themeField.value        = themeToText(theme);
        rotationField.value     = rotationToText(rotation);
        calibrationField.value  = "Recalibrate";

        drawTitle(tft, x, y, w, rowH, gap);
        for (Field* field : fields)
            { drawLine(tft, *field); }
    }

    void update(ST7796S::MSP4021 &tft) {
        char callsign[32];
        if (!services::settings::getFullCallsign(callsign, sizeof(callsign)))
            { strcpy(callsign, "ERROR"); }
        screens::main::title::updateCallsign(tft, callsign);
    }

    bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
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

        for (Field* field : fields) {
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
                case Action::ROTATION:
                    actionRotation(tft, *field);
                    return true;
                case Action::CALIBRATION:
                    actionCalibration(tft, *field);
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }
}