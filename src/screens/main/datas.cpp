/*
 * screens/main/datas.cpp
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
#include <cstddef>
#include "screens/main/datas.h"
#include "screens/mockup/right.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"

#include "ui/fonts/RobotoMono_Medium_18.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace screens::main::datas {
    namespace {
        constexpr size_t VALUE_SIZE = 32;
        struct Field {
            int outerX = 0;
            int outerY = 0;
            int outerW = 0;
            int outerH = 0;
            int innerX = 0;
            int innerY = 0;
            int innerW = 0;
            int innerH = 0;
            int row = 0;
            uint16_t color;
            char label[VALUE_SIZE]  = {};
            char value[VALUE_SIZE]  = {};
            Field(const int rowIndex, const char* name, const char* initial, uint16_t textColor)
            : row(rowIndex), color(textColor) {
                strncpy(label, name, VALUE_SIZE - 1);
                label[VALUE_SIZE - 1] = '\0';

                strncpy(value, initial, VALUE_SIZE - 1);
                value[VALUE_SIZE - 1] = '\0';
            }
        };

        Field latitude  (0, "LAT",    "",            ui::settings::themes::defaults::GREEN);
        Field longitude (1, "LON",    "",            ui::settings::themes::defaults::GREEN);
        Field speed     (2, "SPEED",  "",            ui::settings::themes::defaults::NAV);
        Field heading   (3, "HDG",    "",            ui::settings::themes::defaults::NAV);
        Field asl       (4, "ASL",    "",            ui::settings::themes::defaults::WHITE);
        Field update    (5, "UPDATE", "",   ui::settings::themes::defaults::NAV);

        void clearRow(ST7796S::MSP4021 &tft, const Field &field);
        void drawRow(ST7796S::MSP4021 &tft, const Field &field);
        void updateRow(ST7796S::MSP4021 &tft, Field &field, const char* value);
        void setField(Field &field, const char* value);

        void clearRow(ST7796S::MSP4021 &tft, const Field &field) {
            const int gap = ui::settings::mockup::GAP;
            tft.rectFill(
                field.outerX + gap,         field.outerY + gap,
                field.outerW - (gap * 2),   field.outerH - (gap * 2),
                ui::settings::themes::defaults::BLACK
            );
        }

        void drawRow(ST7796S::MSP4021 &tft, const Field &field) {
            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textTopLeft(
                field.innerX, field.innerY,
                field.innerW, field.innerH,
                field.label
            );

            tft.setFont(ST7796S::RobotoMono_Medium_18);
            tft.setTextColor(field.color);
            tft.textBottomRight(
                field.innerX, field.innerY,
                field.innerW, field.innerH,
                field.value
            );
        }

        void setField(Field &field, const char* value) {
            strncpy(field.value, value, VALUE_SIZE - 1);
            field.value[VALUE_SIZE - 1] = '\0';
        }

        void updateRow(ST7796S::MSP4021 &tft, Field &field, const char* value) {
            if (strcmp(field.value, value) == 0)
                { return; }
            setField(field, value);
            clearRow(tft, field);
            drawRow(tft, field);
        }
    }

    void setLatitude    (const char* value) { setField(latitude,    value); }
    void setLongitude   (const char* value) { setField(longitude,   value); }
    void setSpeed       (const char* value) { setField(speed,       value); }
    void setHeading     (const char* value) { setField(heading,     value); }
    void setASL         (const char* value) { setField(asl,         value); }
    void setUpdate      (const char* value) { setField(update,      value); }

    void updateLatitude   (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, latitude,     value); }
    void updateLongitude  (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, longitude,    value); }
    void updateSpeed      (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, speed,        value); }
    void updateHeading    (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, heading,      value); }
    void updateASL        (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, asl,          value); }
    void updateUpdate     (ST7796S::MSP4021 &tft, const char* value) { updateRow(tft, update,       value); }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::right::draw(tft);

        Field* fields[]         = {&latitude, &longitude, &speed, &heading, &asl, &update};
        const int fieldCount    = sizeof(fields) / sizeof(fields[0]);
        const int rowH          = screens::mockup::right::outerHeight() / fieldCount;
        const int gap           = ui::settings::mockup::GAP;

        for (auto field : fields) {
            field->outerX = screens::mockup::right::outerX();
            field->outerY = screens::mockup::right::outerY() + (rowH * field->row);
            field->outerW = screens::mockup::right::outerWidth();
            field->outerH = rowH;

            field->innerX = field->outerX + (gap * 4);
            field->innerY = field->outerY + (gap / 2);
            field->innerW = field->outerW - (gap * 8);
            field->innerH = field->outerH - (gap);

            if (field->row > 0) {
                tft.lineH(
                    field->innerX, field->outerY,
                    field->innerW, ui::settings::themes::defaults::BORDER
                );
            }
            drawRow(tft, *field);
        }
    }
}
