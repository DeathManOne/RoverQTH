/*
 * src/ui/mockup/buttons.cpp
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
#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/mockup/buttons.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"
#include "ui/widgets/buttons.h"

namespace state     = core::state;
namespace buttons   = ui::mockup::buttons;
namespace grid      = ui::mockup::grid;
namespace uiMockup  = ui::settings::mockup;
namespace theme     = ui::settings::themes::defaults;
namespace uiButtons = ui::widgets::buttons;

namespace {
    struct Area {
        int outerX = 0;
        int outerY = 0;
        int outerW = 0;
        int outerH = 0;
    };

    struct ButtonField {
        const uiButtons::ButtonArea &area;
        const char* label;
        state::Button id;
    };

    void _drawButton(ST7796S::MSP4021 &tft, const ButtonField &field);
    
    Area _area;
    ButtonField _mark {uiButtons::markQTH, " MARK QTH ", state::Button::MARK_QTH};
    ButtonField _sota {uiButtons::sota,     " SOTA ",     state::Button::SOTA};
    ButtonField _menu {uiButtons::menu,     " MENU ",     state::Button::MENU};

    void _drawButton(ST7796S::MSP4021 &tft, const ButtonField &field) {
        const auto &button = field.area;
        const auto state   = state::buttonState(field.id);
        const int radius   = uiMockup::RADIUS;

        uint16_t borderColor = theme::GREY;
        uint16_t fillColor   = theme::BLACK;
        uint16_t textColor   = theme::GREY;
        switch (state) {
            case state::ButtonState::UNAVAILABLE:
                fillColor = theme::GREY;
                textColor = theme::BLACK;
                break;
            case state::ButtonState::READY:
                borderColor = theme::GREEN;
                textColor   = theme::GREEN;
                break;
            case state::ButtonState::RUNNING:
                borderColor = theme::GREEN;
                fillColor   = theme::GREEN;
                textColor   = theme::BLACK;
                break;
        }

        if (fillColor != theme::BLACK) {
            tft.rectRoundFill(
                button.x,       button.y,
                button.width,   button.height,
                radius,         fillColor
            );
        } else {
            tft.rectFill(
                button.x,       button.y,
                button.width,   button.height,
                theme::BLACK
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

void buttons::clear(ST7796S::MSP4021 &tft) {
    tft.rectFill(
        _area.outerX, _area.outerY,
        _area.outerW, _area.outerH,
        theme::BLACK
    );
}

void buttons::draw(ST7796S::MSP4021 &tft) {
    const int gap = uiMockup::GAP;

    _area.outerX = 0;
    _area.outerY = grid::outerY() + grid::outerHeight() + gap;
    _area.outerW = grid::outerWidth();
    _area.outerH = tft.height() - _area.outerY;

    const int btnCount = uiMockup::BUTTON_COUNT;
    const int btnW = (_area.outerW - (gap * (btnCount - 1))) / btnCount;
    uiButtons::markQTH = uiButtons::makeArea(_area.outerX,                      _area.outerY, btnW, _area.outerH);
    uiButtons::sota    = uiButtons::makeArea(_area.outerX + btnW + gap,         _area.outerY, btnW, _area.outerH);
    uiButtons::menu    = uiButtons::makeArea(_area.outerX + ((btnW + gap) * 2), _area.outerY, btnW, _area.outerH);

    _drawButton(tft, _mark);
    _drawButton(tft, _sota);
    _drawButton(tft, _menu);
}

void buttons::updateMARK(ST7796S::MSP4021 &tft) { _drawButton(tft, _mark); }
void buttons::updateSOTA(ST7796S::MSP4021 &tft) { _drawButton(tft, _sota); }
void buttons::updateMENU(ST7796S::MSP4021 &tft) { _drawButton(tft, _menu); }
