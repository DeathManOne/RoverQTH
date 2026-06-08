/*
 * screens/mockup/buttons.cpp
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

#include "core/state.h"
#include "screens/mockup/buttons.h"
#include "screens/mockup/grid.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"
#include "ui/widgets/buttons.h"

#include "ui/fonts/RobotoMono_Bold_16.h"

namespace screens::mockup::buttons {
    namespace {
        struct Area {
            int outerX = 0;
            int outerY = 0;
            int outerW = 0;
            int outerH = 0;
        };

        struct ButtonField {
            const ui::widgets::buttons::ButtonArea &area;
            const char* label;
            core::state::Button id;
        };
        
        Area area;
        ButtonField mark {ui::widgets::buttons::MARK_QTH,   " MARK QTH ",   core::state::Button::MARK_QTH};
        ButtonField sota {ui::widgets::buttons::SOTA,       " SOTA ",       core::state::Button::SOTA};
        ButtonField menu {ui::widgets::buttons::MENU,       " MENU ",       core::state::Button::MENU};

        void drawButton(ST7796S::MSP4021 &tft, const ButtonField &field) {
            const auto &button = field.area;
            const auto state = core::state::buttonState(field.id);
            const int radius = ui::settings::mockup::RADIUS;

            uint16_t borderColor = ui::settings::themes::defaults::GREY;
            uint16_t fillColor   = ui::settings::themes::defaults::BLACK;
            uint16_t textColor   = ui::settings::themes::defaults::GREY;
            switch (state) {
                case core::state::ButtonState::UNAVAILABLE:
                    fillColor = ui::settings::themes::defaults::GREY;
                    textColor = ui::settings::themes::defaults::BLACK;
                    break;
                case core::state::ButtonState::READY:
                    borderColor = ui::settings::themes::defaults::GREEN;
                    textColor   = ui::settings::themes::defaults::GREEN;
                    break;
                case core::state::ButtonState::RUNNING:
                    borderColor = ui::settings::themes::defaults::GREEN;
                    fillColor   = ui::settings::themes::defaults::GREEN;
                    textColor   = ui::settings::themes::defaults::BLACK;
                    break;
            }

            if (fillColor != ui::settings::themes::defaults::BLACK) {
                tft.rectRoundFill(
                    button.x,       button.y,
                    button.width,   button.height,
                    radius,         fillColor
                );
            } else {
                tft.rectFill(
                    button.x,       button.y,
                    button.width,   button.height,
                    ui::settings::themes::defaults::BLACK
                );
            }

            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(textColor);
            tft.rectRound(
                button.x,       button.y,
                button.width,   button.height,
                radius,         borderColor
            );
            tft.textCenter(
                button.x,       button.y,
                button.width,   button.height,
                field.label
            );
        }
    }

    void clear(ST7796S::MSP4021 &tft) {
        tft.rectFill(
            area.outerX, area.outerY,
            area.outerW, area.outerH,
            ui::settings::themes::defaults::BLACK
        );
    }

    void draw(ST7796S::MSP4021 &tft) {
        const int gap = ui::settings::mockup::GAP;

        area.outerX = 0;
        area.outerY = screens::mockup::grid::outerY()
            + screens::mockup::grid::outerHeight()
            + gap;
        area.outerW = screens::mockup::grid::outerWidth();
        area.outerH = tft.height() - area.outerY;

        const int btnCount = ui::settings::mockup::BUTTON_COUNT;
        const int btnW = (area.outerW - (gap * (btnCount - 1))) / btnCount;
        ui::widgets::buttons::MARK_QTH  = ui::widgets::buttons::makeArea(area.outerX,       area.outerY, btnW, area.outerH);
        ui::widgets::buttons::SOTA      = ui::widgets::buttons::makeArea(btnW + gap,        area.outerY, btnW, area.outerH);
        ui::widgets::buttons::MENU      = ui::widgets::buttons::makeArea((btnW + gap) * 2,  area.outerY, btnW, area.outerH);

        drawButton(tft, mark);
        drawButton(tft, sota);
        drawButton(tft, menu);
    }

    void updateMARK(ST7796S::MSP4021 &tft) { drawButton(tft, mark); }
    void updateSOTA(ST7796S::MSP4021 &tft) { drawButton(tft, sota); }
    void updateMENU(ST7796S::MSP4021 &tft) { drawButton(tft, menu); }
}