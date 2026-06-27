/*
 * core/screenManager.cpp
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
#include "ui/widgets/buttons.h"

namespace core::screenManager {
    namespace {
        uint32_t lastTouchMs                    = 0;
        constexpr uint32_t TOUCH_DEBOUNCE_MS    = 180;

        bool touchDebounced();
        void drawMain();
        void toggleMARK();
        bool handleMainTouch(int x, int y);
        void drawMenu();
        bool handleMenuTouch(int x, int y);

        bool touchDebounced() {
            const uint32_t now = millis();
            if ((now - lastTouchMs) >= TOUCH_DEBOUNCE_MS) {
                lastTouchMs = now;
                return false;
            }
            return true;
        }

        void drawMain() {
            display::main::preload();
            display::main::draw();
        }

        void toggleMARK() {
            if (!services::navigation::isMarkRecording()) {
                services::navigation::startMark();

                core::state::setButtonState(
                    core::state::Button::MARK_QTH,
                    core::state::ButtonState::RUNNING
                );

                display::mockup::updateMARK();
                display::main::updateMARK();
                return;
            }
            services::navigation::stopMark();

            // TODO: écriture SD
            services::navigation::clearMark();

            core::state::setButtonState(
                core::state::Button::MARK_QTH,
                core::state::ButtonState::READY
            );

            display::mockup::updateMARK();
            display::main::updateMARK();
        }

        bool handleMainTouch(int x, int y) {
            if (core::state::buttonState(core::state::Button::MARK_QTH) != core::state::ButtonState::UNAVAILABLE) {
                if (ui::widgets::buttons::isPressed(ui::widgets::buttons::MARK_QTH, x, y)) {
                    toggleMARK();
                    return true;
                }
            }
            if (core::state::buttonState(core::state::Button::MENU) != core::state::ButtonState::UNAVAILABLE) {
                if (ui::widgets::buttons::isPressed(ui::widgets::buttons::MENU, x, y)) {
                    core::state::setButtonState(core::state::Button::MENU, core::state::ButtonState::RUNNING);
                    core::state::setScreen(core::state::Screen::MENU);
                    draw();
                    return true;
                }
            }
            return false;
        }
    
        void drawMenu() {
            display::menu::preload();
            display::menu::draw();
        }

        bool handleMenuTouch(int x, int y) {
            if (ui::widgets::buttons::isPressed(ui::widgets::buttons::MENU, x, y)) {
                screens::menu::reset();
                core::state::setButtonState(core::state::Button::MENU, core::state::ButtonState::READY);
                core::state::setScreen(core::state::Screen::MAIN);
                draw();
                return true;
            }
            return display::menu::handleTouch(x, y);
        }
    }

    void begin() { draw(); }
    void draw() {
        switch (core::state::currentScreen()) {
            case core::state::Screen::MAIN:
                drawMain();
                break;
            case core::state::Screen::MENU:
                drawMenu();
                break;
            case core::state::Screen::MAP:
                break;
            case core::state::Screen::SOTA:
                break;
        }
    }

    void update(uint32_t &nextRefreshIn) {
        switch (core::state::currentScreen()) {
            case core::state::Screen::MAIN:
                display::main::update(nextRefreshIn);
                break;
            case core::state::Screen::MENU:
                display::menu::update(nextRefreshIn);
                break;
            case core::state::Screen::MAP:
                nextRefreshIn = 1000;
                break;
            case core::state::Screen::SOTA:
                nextRefreshIn = 1000;
                break;
        }
    }

    void handleTouch() {
        int x, y;
        if (!display::TRead(x, y))  { return; }
        if (touchDebounced())       { return; }

        switch (core::state::currentScreen()) {
            case core::state::Screen::MAIN:
                if (handleMainTouch(x, y))
                    { return; }
                break;
            case core::state::Screen::MENU:
                if (handleMenuTouch(x, y))
                    { return; }
                break;
            case core::state::Screen::MAP:
                break;
            case core::state::Screen::SOTA:
                break;
        }
    }
}