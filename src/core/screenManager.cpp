/*
 * src/core/screenManager.cpp
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

#include <Arduino.h>
#include "core/screenManager.h"
#include "core/state.h"
#include "display/main.h"
#include "display/manager.h"
#include "display/menu.h"
#include "display/mockup.h"
#include "screens/menu.h"
#include "services/navigation.h"
#include "services/qth.h"
#include "ui/widgets/buttons.h"

namespace manager    = core::screenManager;
namespace state      = core::state;
namespace main       = display::main;
namespace menu       = display::menu;
namespace mockup     = display::mockup;
namespace sMenu      = screens::menu;
namespace navigation = services::navigation;
namespace qth        = services::qth;
namespace buttons    = ui::widgets::buttons;

namespace {
    uint32_t _lastTouchMs                = 0;
    constexpr uint32_t TOUCH_DEBOUNCE_MS = 180;

    bool _touchDebounced();
    void _toggleMARK();
    void _drawMain();
    void _drawMenu();
    bool _handleMainTouch(int x, int y);
    bool _handleMenuTouch(int x, int y);

    bool _touchDebounced() {
        const uint32_t now = millis();
        if ((now - _lastTouchMs) >= TOUCH_DEBOUNCE_MS) {
            _lastTouchMs = now;
            return false;
        }
        return true;
    }

    void _toggleMARK() {
        const navigation::MarkState markState = navigation::markState();

        if (markState == navigation::MarkState::IDLE) {
            const bool started = navigation::startMark();
            if (!started) { return; }

            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
            mockup::updateMARK();
            main::updateMARK();
            return;
        }

        if (markState == navigation::MarkState::RECORDING) {
            const bool stopped = navigation::stopMark();
            if (!stopped) { return; }
        }

        if (navigation::markState() == navigation::MarkState::READY_TO_SAVE) {
            if (!qth::isCurrentRecordLongEnough()) {
                navigation::clearMark();
                state::setButtonState(state::Button::MARK_QTH, state::ButtonState::READY);
                mockup::updateMARK();
                main::updateMARK();
                return;
            }
            
            const bool saved = qth::saveCurrentRecord();
            if (!saved) {
                state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
                mockup::updateMARK();
                main::updateMARK();
                return;
            }

            navigation::clearMark();
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::READY);
            mockup::updateMARK();
            main::updateMARK();
        }
    }

    void _drawMain() {
        main::preload();
        main::draw();
    }

    void _drawMenu() {
        menu::preload();
        menu::draw();
    }

    bool _handleMainTouch(int x, int y) {
        if (state::buttonState(state::Button::MARK_QTH) != state::ButtonState::UNAVAILABLE) {
            if (buttons::isPressed(buttons::markQTH, x, y)) {
                _toggleMARK();
                return true;
            }
        }
        if (state::buttonState(state::Button::MENU) != state::ButtonState::UNAVAILABLE) {
            if (buttons::isPressed(buttons::menu, x, y)) {
                state::setButtonState(state::Button::MENU, state::ButtonState::RUNNING);
                state::setScreen(state::Screen::MENU);
                manager::draw();
                return true;
            }
        }
        return false;
    }

    bool _handleMenuTouch(int x, int y) {
        if (sMenu::isEditing()) { return menu::handleTouch(x, y); }
        if (buttons::isPressed(buttons::menu, x, y)) {
            sMenu::reset();
            state::setButtonState(state::Button::MENU, state::ButtonState::READY);
            state::setScreen(state::Screen::MAIN);
            manager::draw();
            return true;
        }
        return menu::handleTouch(x, y);
    }
}

void manager::begin() { draw(); }
void manager::draw() {
    switch (state::currentScreen()) {
        case state::Screen::MENU:
            _drawMenu();
            break;
        case state::Screen::MAP:
            break;
        case state::Screen::SOTA:
            break;
        case state::Screen::MAIN:
        default:
            _drawMain();
            break;
    }
}

void manager::update(uint32_t &nextRefreshIn) {
    switch (state::currentScreen()) {
        case state::Screen::MENU:
            menu::update(nextRefreshIn);
            break;
        case state::Screen::MAP:
            nextRefreshIn = 1000;
            break;
        case state::Screen::SOTA:
            nextRefreshIn = 1000;
            break;
        case state::Screen::MAIN:
        default:
            main::update(nextRefreshIn);
            break;
    }
}

void manager::handleTouch() {
    int x, y;
    if (!display::TRead(x, y)) { return; }
    if (_touchDebounced())     { return; }

    switch (state::currentScreen()) {
        case state::Screen::MAIN:
            if (_handleMainTouch(x, y))
                { return; }
            break;
        case state::Screen::MENU:
            if (_handleMenuTouch(x, y))
                { return; }
            break;
        case state::Screen::MAP:
        case state::Screen::SOTA:
        default: break;
    }
}
