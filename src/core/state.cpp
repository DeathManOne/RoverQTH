/*
 * core/state.cpp
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
#include <array>
#include "core/state.h"

namespace core::state {
    namespace {
        Screen _currentScreen  = Screen::MAIN;
        Screen _previousScreen = Screen::MAIN;

        std::array<ButtonState, static_cast<size_t>(Button::COUNT)> buttonStates {};
    }

    Screen currentScreen()                                  { return _currentScreen; }
    Screen previousScreen()                                 { return _previousScreen; }
    ButtonState buttonState(Button button)                  { return buttonStates[static_cast<size_t>(button)]; }
    bool isScreen(Screen screen)                            { return _currentScreen == screen; }
    void setButtonState(Button button, ButtonState state)   { buttonStates[static_cast<size_t>(button)] = state; }

    void begin() {
        _currentScreen  = Screen::MAIN;
        _previousScreen = Screen::MAIN;

        for (auto &state : buttonStates)
            { state = ButtonState::UNAVAILABLE; }
    }

    void setScreen(Screen screen) {
        if (_currentScreen == screen)
            { return; }
        _previousScreen = _currentScreen;
        _currentScreen  = screen;
    }

    void goBack() {
        const Screen target = _previousScreen;
        _previousScreen     = _currentScreen;
        _currentScreen      = target;
    }
}