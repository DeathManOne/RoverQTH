/*
 * core/state.h
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

#pragma once

namespace core::state {
    enum class Screen       {MAIN, MENU, MAP, SOTA};
    enum class Button       {MARK_QTH, SOTA, MENU, COUNT};
    enum class ButtonState  {UNAVAILABLE, READY, RUNNING};

    Screen currentScreen();
    Screen previousScreen();
    ButtonState buttonState(Button button);

    bool isScreen(Screen screen);
    void setButtonState(Button button, ButtonState state);

    void begin();
    void setScreen(Screen screen);
    void goBack();

}