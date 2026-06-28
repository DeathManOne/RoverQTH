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
            COUNTRY,
            CALLSIGN,
            THEME,
            ROTATION,
            CALIBRATION
        };

        struct Field {
            Action action = Action::NONE;
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;
        };
        
        Field fields[5];

        void nextRow(int &rowY, int rowH) { rowY += rowH; }

        Field makeField(Action action, int x, int y, int w, int h) {
            Field field;
            field.action = action;
            field.x      = x;
            field.y      = y;
            field.w      = w;
            field.h      = h;
            return field;
        }

        bool isPressed(const Field &field, int tx, int ty) {
            return
                tx >= field.x && tx < field.x + field.w &&
                ty >= field.y && ty < field.y + field.h;
        }

        void drawLine(ST7796S::MSP4021 &tft, int x, int y, int w, int h, const char* label, const char* value, uint16_t valueColor = ui::settings::themes::defaults::WHITE) {
            const int gap = ui::settings::mockup::GAP;

            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textCenterLeft(x, y, w / 2, h, label);

            tft.setTextColor(valueColor);
            tft.textCenterRight(x + (w / 2), y, (w / 2) - gap, h, value);
        }
    }

    bool isEditing()    { return mode == Mode::KEYBOARD; }
    void reset()        { mode = Mode::GRID; }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX() + (gap * 2);
        const int y     = screens::mockup::grid::innerY() + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth() - (gap * 4);
        const int rowH  = 28;

        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(ui::settings::themes::defaults::GREEN);
        tft.textCenterLeft(x, y, w, rowH, "GENERAL");

        tft.lineH(
            x, y + rowH + gap,
            w, ui::settings::themes::defaults::BORDER
        );

        int rowY = y + rowH + (gap * 3);
        
        drawLine(tft, x, rowY, w, rowH, "Country", "France", ui::settings::themes::defaults::WHITE);
        nextRow(rowY, rowH);

        char callsign[32];
        if (!services::settings::getCallsign(callsign, sizeof(callsign)))
            { strcpy(callsign, "ERROR"); }
        fields[1] = makeField(Action::CALLSIGN, x, rowY, w, rowH);
        drawLine(tft, x, rowY, w, rowH, "Callsign", callsign, ui::settings::themes::defaults::GREEN);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Theme", "Normal", ui::settings::themes::defaults::WHITE);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Rotation", "Landscape", ui::settings::themes::defaults::WHITE);
        nextRow(rowY, rowH);

        drawLine(tft, x, rowY, w, rowH, "Touch", "Recalibrate", ui::settings::themes::defaults::WHITE);
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

        for (auto &field : fields) {
            if (!isPressed(field, x, y))
                { continue; }
            switch (field.action) {
                case Action::COUNTRY:
                    // TODO: change country, still do not know how
                    return true;
                case Action::CALLSIGN: {
                    char callsign[32];
                    if (services::settings::getCallsign(callsign, sizeof(callsign)))
                        { tft.KSetText(callsign); }
                    tft.KDraw("Callsign");
                    mode = Mode::KEYBOARD;
                    return true;
                }
                case Action::THEME:
                    // TODO: cycle theme
                    return true;
                case Action::ROTATION:
                    // TODO: toggle rotation
                    return true;
                case Action::CALIBRATION:
                    // TODO: recalibrate touch
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }
}