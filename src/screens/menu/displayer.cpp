/*
 * src/screens/menu/displayer.cpp
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

#include "display/manager.h"
#include "screens/menu.h"
#include "screens/menu/displayer.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::Displayer;
namespace menu     = screens::menu;
namespace settings = services::settings;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

settings::TFTRotation Displayer::_nextRotation(settings::TFTRotation rotation) {
    switch (rotation) {
        case settings::TFTRotation::NORMAL:
            return settings::TFTRotation::REVERSED;
        case settings::TFTRotation::REVERSED:
        default:
            return settings::TFTRotation::NORMAL;
    }
}

const char* Displayer::_rotationToText(settings::TFTRotation rotation) {
    switch (rotation) {
        case settings::TFTRotation::REVERSED:
            return "Reversed";
        case settings::TFTRotation::NORMAL:
        default:
            return "Normal";
    }
}

void Displayer::_actionRotation(ST7796S::MSP4021 &tft) {
    const settings::TFTRotation rotation = _nextRotation(settings::getTFTRotation());
    if (!settings::setTFTRotation(rotation)) { return; }

    tft.setRotation(static_cast<uint8_t>(rotation));
    display::TLoad();
    display::clearScreen();
    menu::draw(tft);
}

void Displayer::_actionCalibration(ST7796S::MSP4021 &tft) {
    if (!settings::resetTouchCalibration()) { return; }
    while (!display::TCalibrate())          { delay(10); }
    display::clearScreen();
    menu::draw(tft);
}

void Displayer::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()       + (gap * 2);
    const int y     = grid::innerY()       + (gap * 2);
    const int w     = grid::innerWidth()   - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    std::snprintf(_widthValue,  sizeof(_widthValue),  "%d px", TFT_WIDTH);
    std::snprintf(_heightValue, sizeof(_heightValue), "%d px", TFT_HEIGHT);
    _rotationField.value = _rotationToText(settings::getTFTRotation());

    _widthField.value       = _widthValue;
    _heightField.value      = _heightValue;
    _calibrationField.value = "Recalibrate";

    _drawTitle(tft, x, y, w, rowH, gap, "display");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

bool Displayer::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::ROTATION:
                _actionRotation(tft);
                return true;
            case _Action::CALIBRATION:
                _actionCalibration(tft);
                return true;
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}

