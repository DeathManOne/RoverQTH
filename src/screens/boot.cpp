/*
 * src/screens/boot.cpp
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
#include <cstddef>
#include <DejaVuSans_Bold_18.h>

#include "screens/boot.h"
#include "ui/images/logo.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"
#include "ui/widgets/buttons.h"

namespace boot    = screens::boot;
namespace mockup  = ui::settings::mockup;
namespace theme   = ui::settings::themes::defaults;
namespace buttons = ui::widgets::buttons;

namespace {
    uint8_t _gpsProgress = 0;
    struct Field {
        int outerX  = 0;
        int outerY  = 0;
        int outerW  = 0;
        int outerH  = 0;
        int innerX  = 0;
        int innerY  = 0;
        int innerW  = 0;
        int innerH  = 0;
        int row     = 0;

        const char* label = nullptr;
        bool* state       = nullptr;

        Field(int rowIndex, const char* name)
        : row(rowIndex), label(name) {}
    };

    Field _wifi(0, "WiFi");
    Field _sd  (1, "SD");
    Field _gps (2, "GPS");

    void _clearModule     (ST7796S::MSP4021 &tft, const Field &field);
    void _drawModuleStatus(ST7796S::MSP4021 &tft, Field &field);

    void _updateField(ST7796S::MSP4021 &tft, Field &field, bool* state);

    void _drawGPSSearch  (ST7796S::MSP4021 &tft);
    void _drawGPSAction  (ST7796S::MSP4021 &tft);
    void _drawGPSProgress(ST7796S::MSP4021 &tft, uint8_t value);

    void _clearModule(ST7796S::MSP4021 &tft, const Field &field) {
        tft.rectFill(
            field.outerX + 1, field.outerY + 1,
            field.outerW - 2, field.outerH - 2,
            theme::BLACK
        );
    }
    
    void _drawModuleStatus(ST7796S::MSP4021 &tft, Field &field) {
        _clearModule(tft, field);

        const int gap         = mockup::GAP;
        const int labelH      = tft.textHeight(field.label);
        const int innerValueX = field.innerX + (gap * 4);
        const int innerValueY = field.innerY + gap + labelH;
        
        tft.setFont(ST7796S::DejaVuSans_Bold_18);
        tft.setTextColor(theme::WHITE);
        tft.text(field.innerX, field.innerY, field.label);

        uint16_t color;
        const char* status = nullptr;
        if (!field.state) {
            status  = "loading";
            color   = theme::YELLOW;
        } else if (*field.state) {
            status  = "loaded";
            color   = theme::GREEN;
        } else {
            status  = "failed";
            color   = theme::RED;
        }

        tft.setTextColor(color);
        tft.text(innerValueX, innerValueY, status);
    }

    void _updateField(ST7796S::MSP4021 &tft, Field &field, bool* state) {
        if (field.state == state)
            { return; }
        field.state = state;
        _drawModuleStatus(tft, field);
    }

    void _drawGPSSearch(ST7796S::MSP4021 &tft) {
        const buttons::ButtonArea &btn = buttons::bootSearchGPS;

        tft.setFont(ST7796S::DejaVuSans_Bold_18);
        tft.setTextColor(theme::WHITE);
        tft.rectFill(
            btn.x,      btn.y,
            btn.width,  btn.height,
            theme::GREY
        );
        tft.rect(
            btn.x,      btn.y,
            btn.width,  btn.height,
            theme::WHITE
        );
        tft.textCenter(
            btn.x,      btn.y,
            btn.width,  btn.height,
            "Search GPS"
        );
    }

    void _drawGPSProgress(ST7796S::MSP4021 &tft, uint8_t value) {
        const buttons::ButtonArea &bar = buttons::bootSearchGPS;
        constexpr size_t TEXT_SIZE = 8;

        if (value > 100) { value = 100; }
        tft.progressBar(
            bar.x,      bar.y,
            bar.width,  bar.height,
            value,      theme::CYAN
        );

        char text[TEXT_SIZE];
        snprintf(text, TEXT_SIZE, "%u %%", value);

        tft.setFont(ST7796S::DejaVuSans_Bold_18);
        tft.setTextColor(theme::WHITE);
        tft.textCenter(
            bar.x,      bar.y,
            bar.width,  bar.height,
            text
        );
    }

    void _drawGPSAction(ST7796S::MSP4021 &tft) {
        const buttons::ButtonArea &area = buttons::bootSearchGPS;
        tft.rectFill(
            area.x,     area.y,
            area.width, area.height,
            theme::BLACK
        );

        if (_gps.state == nullptr) { return; }
        if (!(*_gps.state)) {
            _drawGPSSearch(tft);
            return;
        }
        _drawGPSProgress(tft, _gpsProgress);
    }
}

void boot::clear(ST7796S::MSP4021 &tft) { tft.fillScreen(theme::BLACK); }

void boot::draw(ST7796S::MSP4021 &tft) {
    const int gap          = mockup::GAP;
    const int imgGapX      = 10;
    const int imgW         = ST7796S::logo_width;

    const int outerX       = imgW + (imgGapX * 2);
    const int outerW       = tft.width() - outerX - (gap * 2);

    const int btnH         = ui::settings::mockup::BUTTON_HEIGHT;
    const int btnY         = tft.height() - btnH - (gap * 2);
    buttons::bootSearchGPS = buttons::makeArea(outerX, btnY, outerW, btnH);

    Field* fields[] = {&_wifi, &_sd, &_gps};
    constexpr int FIELD_COUNT = sizeof(fields) / sizeof(fields[0]);

    const int screenY   = 0;
    const int screenH   = btnY - gap - screenY;
    const int rowH      = screenH / FIELD_COUNT;

    for (auto field : fields) {
        field->outerX = outerX;
        field->outerY = screenY + (rowH * field->row);
        field->outerW = outerW;
        field->outerH = rowH;

        field->innerX = field->outerX + gap;
        field->innerY = field->outerY + gap;
        field->innerW = field->outerW - (gap * 2);
        field->innerH = field->outerH - (gap * 2);

        _drawModuleStatus(tft, *field);
    }
    _drawGPSAction(tft);
}

void boot::updateWifi(ST7796S::MSP4021 &tft, bool* value) { _updateField(tft, _wifi, value); }
void boot::updateSD  (ST7796S::MSP4021 &tft, bool* value) { _updateField(tft, _sd,   value); }
void boot::updateGPS (ST7796S::MSP4021 &tft, bool* value) {
    _updateField(tft, _gps, value);
    _drawGPSAction(tft);
}

void boot::updateGPSProgress(ST7796S::MSP4021 &tft, uint8_t progress) {
    if (_gpsProgress == progress)
        { return; }
    _gpsProgress = progress;
    _drawGPSAction(tft);
}

