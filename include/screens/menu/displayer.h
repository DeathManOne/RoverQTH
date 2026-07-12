/*
 * include/screens/menu/displayer.h
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

#include <MSP4021.h>
#include "screens/menu/page.h"
#include "services/settings.h"

namespace screens::menu {
    class Displayer final : public Page {
        public:
            void draw       (ST7796S::MSP4021 &tft) override;
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class _Action {NONE, ROTATION, CALIBRATION};

            char _widthValue[8]  = "";
            char _heightValue[8] = "";

            Field<_Action> _rotationField    = _makeField("Rotation",    _Action::ROTATION,    ui::settings::themes::defaults::GREEN);
            Field<_Action> _widthField       = _makeField("Width",       _Action::NONE,        ui::settings::themes::defaults::WHITE);
            Field<_Action> _heightField      = _makeField("Height",      _Action::NONE,        ui::settings::themes::defaults::WHITE);
            Field<_Action> _calibrationField = _makeField("Calibration", _Action::CALIBRATION, ui::settings::themes::defaults::GREEN);

            Field<_Action>* _fields[4] = {
                &_rotationField,
                &_widthField,
                &_heightField,
                &_calibrationField
            };

            static services::settings::TFTRotation _nextRotation(services::settings::TFTRotation rotation);
            static const char* _rotationToText(services::settings::TFTRotation rotation);
            void _actionRotation(ST7796S::MSP4021 &tft);

            void _actionCalibration(ST7796S::MSP4021 &tft);
    };
}
