/*
 * src/screens/main/title.cpp
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

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <esp_timer.h>

#include "services/battery.h"
#include "screens/main/title.h"
#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/mockup/header.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

namespace battery = services::battery;
namespace title   = screens::main::title;
namespace header  = ui::mockup::header;
namespace mockup  = ui::settings::mockup;
namespace theme   = ui::settings::themes::defaults;

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

    void _drawText(ST7796S::MSP4021 &tft, const Field &field);
    void _drawLine(ST7796S::MSP4021 &tft, const Field &field);
    void _drawRect(ST7796S::MSP4021 &tft, const Field &field);

    void _setField(Field &field, const char* value);
    void _updateField(ST7796S::MSP4021 &tft, Field &field, const char* value);

    Field _callsign("");
    Field _date("");
    Field _time("");
    Field _battery("");

    void _drawText(ST7796S::MSP4021 &tft, const Field &field) {
        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(theme::WHITE);
        tft.textCenter(
            field.innerX, header::innerY(),
            field.innerW, header::innerHeight(),
            field.value
        );
    }

    void _drawLine(ST7796S::MSP4021 &tft, const Field &field) {
        int gap = mockup::GAP;
        tft.lineV(
            field.outerX,
            header::innerY() + gap,
            header::innerHeight() - (gap * 2),
            theme::BORDER
        );
    }

    void _drawRect(ST7796S::MSP4021 &tft, const Field &field) {
        tft.rectFill(
            field.outerX + 1, header::outerY() + 1,
            field.outerW - 2, header::outerHeight() - 2,
            theme::BLACK
        );
    }

    void _setField(Field &field, const char* value) {
        strncpy(field.value, value, VALUE_SIZE - 1);
        field.value[VALUE_SIZE - 1] = '\0';
    }

    void _updateField(ST7796S::MSP4021 &tft, Field &field, const char* value) {
        if (strcmp(field.value, value) == 0)
            { return; }
        _setField(field, value);
        _drawRect(tft, field);
        _drawText(tft, field);
    }
}

uint64_t title::uptimeSeconds() { return esp_timer_get_time() / 1000000ULL; }

void title::setCallsign(const char* value) { _setField(_callsign, value); }
void title::setDate    (const char* value) { _setField(_date,     value); }
void title::setTime    (const char* value) { _setField(_time,     value); }
void title::setBattery (const char* value) { _setField(_battery,  value); }

void title::updateCallsign(ST7796S::MSP4021 &tft, const char* value) { _updateField(tft, _callsign, value); }
void title::updateDate    (ST7796S::MSP4021 &tft, const char* value) { _updateField(tft, _date,     value); }
void title::updateTime    (ST7796S::MSP4021 &tft, const char* value) { _updateField(tft, _time,     value); }
void title::updateBattery (ST7796S::MSP4021 &tft, const char* value) { _updateField(tft, _battery,  value); }

void title::getUptime(char* buffer, size_t size) {
    uint64_t sec = uptimeSeconds();

    const uint64_t hours = sec / 3600;
    sec %= 3600;

    const uint64_t minutes = sec / 60;
    sec %= 60;

    snprintf(buffer, size, "%03llu : %02llu : %02llu", hours, minutes, sec);
}

void title::getBatteryLevel(char* buffer, size_t size) {
    if (battery::isPresent())
        { snprintf(buffer, size, "%u %%", battery::getPercent()); }
    else { snprintf(buffer, size, "N/A"); }
}

void title::draw(ST7796S::MSP4021 &tft) {
    header::draw(tft);

    const int gap     = mockup::GAP;
    const int headerW = header::outerWidth();

    _callsign.outerX = header::outerX();
    _callsign.outerW = (headerW * 3) / 8;
    _callsign.innerX = _callsign.outerX + (gap * 2);
    _callsign.innerW = _callsign.outerW - (gap * 4);
    _drawText(tft, _callsign);
    
    _date.outerX = _callsign.outerX + _callsign.outerW;
    _date.outerW = headerW / 4;
    _date.innerX = _date.outerX + (gap * 2);
    _date.innerW = _date.outerW - (gap * 4);
    _drawLine(tft, _date);
    _drawText(tft, _date);
    
    _time.outerX = _date.outerX + _date.outerW;
    _time.outerW = headerW / 4;
    _time.innerX = _time.outerX + (gap * 2);
    _time.innerW = _time.outerW - (gap * 4);
    _drawLine(tft, _time);
    _drawText(tft, _time);

    _battery.outerX = _time.outerX + _time.outerW;
    _battery.outerW = headerW - _callsign.outerW - _date.outerW - _time.outerW;
    _battery.innerX = _battery.outerX + (gap * 2);
    _battery.innerW = _battery.outerW - (gap * 4);
    _drawLine(tft, _battery);
    _drawText(tft, _battery);
}
