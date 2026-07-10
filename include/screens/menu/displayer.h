/*
 * screens/menu/displayer.h
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

#include "screens/menu/page.h"
#include "services/settings.h"

namespace screens::menu {
    class Displayer final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class Action {NONE, ROTATION, CALIBRATION};

            char widthValue[8]  = "";
            char heightValue[8] = "";

            Field<Action> rotationField    = makeField("Rotation",    Action::ROTATION,    ui::settings::themes::defaults::GREEN);
            Field<Action> widthField       = makeField("Width",       Action::NONE,        ui::settings::themes::defaults::WHITE);
            Field<Action> heightField      = makeField("Height",      Action::NONE,        ui::settings::themes::defaults::WHITE);
            Field<Action> calibrationField = makeField("Calibration", Action::CALIBRATION, ui::settings::themes::defaults::GREEN);

            Field<Action>* fields[4] = {
                &rotationField,
                &widthField,
                &heightField,
                &calibrationField
            };

            static services::settings::TFTRotation nextRotation(services::settings::TFTRotation rotation);
            static const char* rotationToText(services::settings::TFTRotation rotation);

            void actionRotation(ST7796S::MSP4021 &tft);
            void actionCalibration(ST7796S::MSP4021 &tft);
    };
}
