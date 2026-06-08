/*
 * screens/main/title.cpp
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
#include "screens/main/title.h"
#include "screens/mockup/header.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"

#include "ui/fonts/RobotoMono_Bold_16.h"

namespace screens::main::title {
    namespace {
        constexpr size_t VALUE_SIZE = 32;
        struct Field {
            int outerX = 0;
            int outerW = 0;
            int innerX = 0;
            int innerW = 0;
            char value[VALUE_SIZE] = {};
            Field(const char* initial) {
                strncpy(value, initial, VALUE_SIZE - 1);
                value[VALUE_SIZE - 1] = '\0';
            }
        };

        Field callsign  ("");
        Field date      ("");
        Field time      ("");
        Field battery   ("");

        void drawText(ST7796S::MSP4021 &tft, const Field &field);
        void drawLine(ST7796S::MSP4021 &tft, const Field &field);
        void drawRect(ST7796S::MSP4021 &tft, const Field &field);
        void setField(Field &field, const char* value);
        void updateField(ST7796S::MSP4021 &tft, Field &field, const char* value);

        void drawText(ST7796S::MSP4021 &tft, const Field &field) {
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textCenter(
                field.innerX, screens::mockup::header::innerY(),
                field.innerW, screens::mockup::header::innerHeight(),
                field.value
            );
        }

        void drawLine(ST7796S::MSP4021 &tft, const Field &field) {
            int gap = ui::settings::mockup::GAP;
            tft.lineV(
                field.outerX,
                screens::mockup::header::innerY() + gap,
                screens::mockup::header::innerHeight() - (gap * 2),
                ui::settings::themes::defaults::BORDER
            );
        }

        void drawRect(ST7796S::MSP4021 &tft, const Field &field) {
            tft.rectFill(
                field.outerX + 1, screens::mockup::header::outerY() + 1,
                field.outerW - 2, screens::mockup::header::outerHeight() - 2,
                ui::settings::themes::defaults::BLACK
            );
        }
    
        void setField(Field &field, const char* value) {
            strncpy(field.value, value, VALUE_SIZE - 1);
            field.value[VALUE_SIZE - 1] = '\0';
        }

        void updateField(ST7796S::MSP4021 &tft, Field &field, const char* value) {
            if (strcmp(field.value, value) == 0)
                { return; }
            setField(field, value);
            drawRect(tft, field);
            drawText(tft, field);
        }
    }

    void setCallsign(const char* value) { setField(callsign, value); }
    void setDate    (const char* value) { setField(date,     value); }
    void setTime    (const char* value) { setField(time,     value); }
    void setBattery (const char* value) { setField(battery,  value); }

    void updateDate     (ST7796S::MSP4021 &tft, const char* value) { updateField(tft, date,    value); }
    void updateTime     (ST7796S::MSP4021 &tft, const char* value) { updateField(tft, time,    value); }
    void updateBattery  (ST7796S::MSP4021 &tft, const char* value) { updateField(tft, battery, value); }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::header::draw(tft);
        const int gap       = ui::settings::mockup::GAP;
        const int headerW   = screens::mockup::header::outerWidth();

        callsign.outerX = screens::mockup::header::outerX();
        callsign.outerW = (headerW * 3) / 8;
        callsign.innerX = callsign.outerX + (gap * 2);
        callsign.innerW = callsign.outerW - (gap * 4);
        drawText(tft, callsign);
        
        date.outerX = callsign.outerX + callsign.outerW;
        date.outerW = headerW / 4;
        date.innerX = date.outerX + (gap * 2);
        date.innerW = date.outerW - (gap * 4);
        drawLine(tft, date);
        drawText(tft, date);
        
        time.outerX = date.outerX + date.outerW;
        time.outerW = headerW / 4;
        time.innerX = time.outerX + (gap * 2);
        time.innerW = time.outerW - (gap * 4);
        drawLine(tft, time);
        drawText(tft, time);

        battery.outerX = time.outerX + time.outerW;
        battery.outerW = headerW - callsign.outerW - date.outerW  - time.outerW;
        battery.innerX = battery.outerX + (gap * 2);
        battery.innerW = battery.outerW - (gap * 4);
        drawLine(tft, battery);
        drawText(tft, battery);
    }
}
