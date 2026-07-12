/*
 * src/core/state.cpp
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

namespace state = core::state;

namespace {
    state::Screen _currentScreen  = state::Screen::MAIN;
    state::Screen _previousScreen = state::Screen::MAIN;
    std::array<state::ButtonState, static_cast<size_t>(state::Button::COUNT)> _buttonStates {};
}

state::Screen state::currentScreen()                        { return _currentScreen; }
state::Screen state::previousScreen()                       { return _previousScreen; }
state::ButtonState state::buttonState(state::Button button) { return _buttonStates[static_cast<size_t>(button)]; }
bool state::isScreen(state::Screen screen)                  { return _currentScreen == screen; }

void state::setButtonState(state::Button button, state::ButtonState state)   { _buttonStates[static_cast<size_t>(button)] = state; }

void state::begin() {
    _currentScreen  = Screen::MAIN;
    _previousScreen = Screen::MAIN;
    for (auto &state : _buttonStates)
        { state = ButtonState::UNAVAILABLE; }
}

void state::setScreen(Screen screen) {
    if (_currentScreen == screen) { return; }
    _previousScreen = _currentScreen;
    _currentScreen  = screen;
}

void state::goBack() {
    const Screen target = _previousScreen;
    _previousScreen     = _currentScreen;
    _currentScreen      = target;
}