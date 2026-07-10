/*
 * screens/boot.cpp
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

namespace screens::boot {
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
            bool* state = nullptr;
            Field(int rowIndex, const char* name)
                : row(rowIndex), label(name) {}
        };

        Field wifi  (0, "WiFi");
        Field sd    (1, "SD");
        Field gps   (2, "GPS");

        void clearModule(ST7796S::MSP4021 &tft, const Field &field);
        void drawModuleStatus(ST7796S::MSP4021 &tft, Field &field);
        void updateField(ST7796S::MSP4021 &tft, Field &field, bool* state);
        void drawSearchGPS(ST7796S::MSP4021 &tft);
        void drawProgressGPS(ST7796S::MSP4021 &tft, uint8_t value);
        void drawGPSAction(ST7796S::MSP4021 &tft);

        void clearModule(ST7796S::MSP4021 &tft, const Field &field) {
            tft.rectFill(
                field.outerX + 1, field.outerY + 1,
                field.outerW - 2, field.outerH - 2,
                ui::settings::themes::defaults::BLACK
            );
        }
        
        void drawModuleStatus(ST7796S::MSP4021 &tft, Field &field) {
            clearModule(tft, field);
            const int gap = ui::settings::mockup::GAP;

            tft.setFont(ST7796S::DejaVuSans_Bold_18);
            tft.setTextColor(ui::settings::themes::defaults::WHITE);

            const int labelH = tft.textHeight(field.label);
            const int innerValueX = field.innerX + (gap * 4);
            const int innerValueY = field.innerY + gap + labelH;
            tft.text(field.innerX, field.innerY, field.label);

            uint16_t color;
            const char* status = nullptr;
            if (!field.state) {
                status  = "loading";
                color   = ui::settings::themes::defaults::YELLOW;
            } else if (*field.state) {
                status  = "loaded";
                color   = ui::settings::themes::defaults::GREEN;
            } else {
                status  = "failed";
                color   = ui::settings::themes::defaults::RED;
            }

            tft.setTextColor(color);
            tft.text(innerValueX, innerValueY, status);
        }

        void updateField(ST7796S::MSP4021 &tft, Field &field, bool* state) {
            if (field.state == state)
                { return; }
            field.state = state;
            drawModuleStatus(tft, field);
        }

        void drawSearchGPS(ST7796S::MSP4021 &tft) {
            const ui::widgets::buttons::ButtonArea &btn = ui::widgets::buttons::BOOT_SEARCH_GPS;
            tft.setFont(ST7796S::DejaVuSans_Bold_18);
            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.rectFill(
                btn.x,      btn.y,
                btn.width,  btn.height,
                ui::settings::themes::defaults::GREY
            );
            tft.rect(
                btn.x,      btn.y,
                btn.width,  btn.height,
                ui::settings::themes::defaults::WHITE
            );
            tft.textCenter(
                btn.x,      btn.y,
                btn.width,  btn.height,
                "Search GPS"
            );
        }

        void drawProgressGPS(ST7796S::MSP4021 &tft, uint8_t value) {
            const ui::widgets::buttons::ButtonArea &bar = ui::widgets::buttons::BOOT_SEARCH_GPS;
            constexpr size_t TEXT_SIZE = 8;

            if (value > 100)
                { value = 100; }
            tft.progressBar(
                bar.x,      bar.y,
                bar.width,  bar.height,
                value,      ui::settings::themes::defaults::CYAN
            );

            char text[TEXT_SIZE];
            snprintf(text, TEXT_SIZE, "%u %%", value);

            tft.setFont(ST7796S::DejaVuSans_Bold_18);
            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textCenter(
                bar.x,      bar.y,
                bar.width,  bar.height,
                text
            );
        }

        void drawGPSAction(ST7796S::MSP4021 &tft) {
            const ui::widgets::buttons::ButtonArea &area = ui::widgets::buttons::BOOT_SEARCH_GPS;
            tft.rectFill(
                area.x,     area.y,
                area.width, area.height,
                ui::settings::themes::defaults::BLACK
            );

            if (gps.state == nullptr) { return; }
            if (!(*gps.state)) {
                drawSearchGPS(tft);
                return;
            }
            drawProgressGPS(tft, _gpsProgress);
        }
    }

    void clear(ST7796S::MSP4021 &tft) { tft.fillScreen(ui::settings::themes::defaults::BLACK); }

    void draw(ST7796S::MSP4021 &tft) {
        const int gap       = ui::settings::mockup::GAP;
        const int imgGapX   = 10;
        const int imgW      = ST7796S::logo_width;

        const int outerX    = imgW + (imgGapX * 2);
        const int outerW    = tft.width() - outerX - (gap * 2);

        const int btnH      = ui::settings::mockup::BUTTON_HEIGHT;
        const int btnY      = tft.height() - btnH - (gap * 2);
        ui::widgets::buttons::BOOT_SEARCH_GPS = ui::widgets::buttons::makeArea(outerX, btnY, outerW, btnH);

        Field* fields[] = {&wifi, &sd, &gps};
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

            drawModuleStatus(tft, *field);
        }
        drawGPSAction(tft);
    }

    void updateWifi (ST7796S::MSP4021 &tft, bool* value) { updateField(tft, wifi, value); }
    void updateSD   (ST7796S::MSP4021 &tft, bool* value) { updateField(tft, sd,   value); }

    void updateGPS  (ST7796S::MSP4021 &tft, bool* value) {
        updateField(tft, gps, value);
        drawGPSAction(tft);
    }

    void updateGPSProgress(ST7796S::MSP4021 &tft, uint8_t progress) {
        if (_gpsProgress == progress)
            { return; }
        _gpsProgress = progress;
        drawGPSAction(tft);
    }
}
