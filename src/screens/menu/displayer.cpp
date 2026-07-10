/*
 * screens/menu/displayer.cpp
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
#include "screens/mockup/grid.h"

namespace screens::menu {
    services::settings::TFTRotation Displayer::nextRotation(services::settings::TFTRotation rotation) {
        switch (rotation) {
            case services::settings::TFTRotation::NORMAL:
                return services::settings::TFTRotation::REVERSED;
            case services::settings::TFTRotation::REVERSED:
            default:
                return services::settings::TFTRotation::NORMAL;
        }
    }

    const char* Displayer::rotationToText(services::settings::TFTRotation rotation) {
        switch (rotation) {
            case services::settings::TFTRotation::REVERSED:
                return "Reversed";
            case services::settings::TFTRotation::NORMAL:
            default:
                return "Normal";
        }
    }

    void Displayer::actionRotation(ST7796S::MSP4021 &tft) {
        const services::settings::TFTRotation rotation = nextRotation(services::settings::getTFTRotation());
        services::settings::setTFTRotation(rotation);
        tft.setRotation(static_cast<uint8_t>(rotation));
        display::TLoad();
        display::clearScreen();
        screens::menu::draw(tft);
    }

    void Displayer::actionCalibration(ST7796S::MSP4021 &tft) {
        services::settings::resetTouchCalibration();
        while (!display::TCalibrate())
            { delay(10); }
        display::clearScreen();
        screens::menu::draw(tft);
    }

    void Displayer::draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field<Action>* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        std::snprintf(widthValue,  sizeof(widthValue),  "%d px", TFT_WIDTH);
        std::snprintf(heightValue, sizeof(heightValue), "%d px", TFT_HEIGHT);
        rotationField.value = rotationToText(services::settings::getTFTRotation());

        widthField.value       = widthValue;
        heightField.value      = heightValue;
        calibrationField.value = "Recalibrate";

        drawTitle(tft, x, y, w, rowH, gap, "display");
        for (Field<Action>* field : fields)
            { drawLine(tft, *field); }
    }

    bool Displayer::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
        for (Field<Action>* field : fields) {
            if (!isPressed(*field, x, y))
                { continue; }
            switch (field->action) {
                case Action::ROTATION:
                    actionRotation(tft);
                    return true;
                case Action::CALIBRATION:
                    actionCalibration(tft);
                    return true;
                case Action::NONE:
                default:
                    return false;
            }
        }
        return false;
    }
}
