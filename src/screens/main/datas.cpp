/*
 * src/screens/main/datas.cpp
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

#include <cstddef>
#include <cstring>

#include "screens/main/datas.h"
#include "ui/fonts/RobotoMono_Medium_18.h"
#include "ui/fonts/RobotoMono_Regular_14.h"
#include "ui/mockup/right.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

namespace datas = screens::main::datas;
namespace right = ui::mockup::right;
namespace mockup = ui::settings::mockup;
namespace theme = ui::settings::themes::defaults;

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

        Field(const int rowIndex, const char* name, uint16_t textColor)
        : row(rowIndex), color(textColor) {
            strncpy(label, name, VALUE_SIZE - 1);
            label[VALUE_SIZE - 1] = '\0';

            strncpy(value, "", VALUE_SIZE - 1);
            value[VALUE_SIZE - 1] = '\0';
        }
    };

    void _clearRow(ST7796S::MSP4021 &tft, const Field &field);
    void _drawRow (ST7796S::MSP4021 &tft, const Field &field);

    void _updateRow(ST7796S::MSP4021 &tft, Field &field, const char* value);
    void _setField(Field &field, const char* value);

    Field _latitude (0, "LAT",    theme::GREEN);
    Field _longitude(1, "LON",    theme::GREEN);
    Field _speed    (2, "SPEED",  theme::NAV);
    Field _heading  (3, "HDG",    theme::NAV);
    Field _asl      (4, "ASL",    theme::WHITE);
    Field _update   (5, "UPDATE", theme::NAV);

    void _clearRow(ST7796S::MSP4021 &tft, const Field &field) {
        const int gap = mockup::GAP;
        tft.rectFill(
            field.outerX + gap,         field.outerY + gap,
            field.outerW - (gap * 2),   field.outerH - (gap * 2),
            theme::BLACK
        );
    }

    void _drawRow(ST7796S::MSP4021 &tft, const Field &field) {
        tft.setFont(ST7796S::RobotoMono_Regular_14);
        tft.setTextColor(theme::WHITE);
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

    void _setField(Field &field, const char* value) {
        strncpy(field.value, value, VALUE_SIZE - 1);
        field.value[VALUE_SIZE - 1] = '\0';
    }

    void _updateRow(ST7796S::MSP4021 &tft, Field &field, const char* value) {
        if (strcmp(field.value, value) == 0)
            { return; }
        _setField(field, value);
        _clearRow(tft, field);
        _drawRow(tft, field);
    }
}

void datas::setLatitude (const char* value) { _setField(_latitude,  value); }
void datas::setLongitude(const char* value) { _setField(_longitude, value); }
void datas::setSpeed    (const char* value) { _setField(_speed,     value); }
void datas::setHeading  (const char* value) { _setField(_heading,   value); }
void datas::setASL      (const char* value) { _setField(_asl,       value); }
void datas::setUpdate   (const char* value) { _setField(_update,    value); }

void datas::updateLatitude (ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _latitude,  value); }
void datas::updateLongitude(ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _longitude, value); }
void datas::updateSpeed    (ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _speed,     value); }
void datas::updateHeading  (ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _heading,   value); }
void datas::updateASL      (ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _asl,       value); }
void datas::updateUpdate   (ST7796S::MSP4021 &tft, const char* value) { _updateRow(tft, _update,    value); }

void datas::draw(ST7796S::MSP4021 &tft) {
    right::draw(tft);

    Field* fields[]      = {&_latitude, &_longitude, &_speed, &_heading, &_asl, &_update};
    const int fieldCount = sizeof(fields) / sizeof(fields[0]);
    const int rowH       = right::outerHeight() / fieldCount;
    const int gap        = mockup::GAP;

    for (Field* field : fields) {
        field->outerX = right::outerX();
        field->outerY = right::outerY() + (rowH * field->row);
        field->outerW = right::outerWidth();
        field->outerH = rowH;

        field->innerX = field->outerX + (gap * 4);
        field->innerY = field->outerY + (gap / 2);
        field->innerW = field->outerW - (gap * 8);
        field->innerH = field->outerH - (gap);

        if (field->row > 0) {
            tft.lineH(
                field->innerX, field->outerY,
                field->innerW, theme::BORDER
            );
        }
        _drawRow(tft, *field);
    }
}
