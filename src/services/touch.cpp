/*
 * services/touch.cpp
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
#include "display/manager.h"
#include "services/nvs.h"
#include "services/touch.h"

namespace services::touch {
    bool loadCalibration() {
        bool swapXY, invertX, invertY;
        float cxa, cxb, cxc, cya, cyb, cyc;

        const bool ok = services::nvs::getTouchCalibration(swapXY, invertX, invertY, cxa, cxb, cxc, cya, cyb, cyc);
        if (!ok)
            { return false; }
        display::TCalibrate(swapXY, invertX, invertY, cxa, cxb, cxc, cya, cyb, cyc);
        return true;
    }

    bool saveCalibration(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc) {
        return services::nvs::setTouchCalibration(swapXY, invertX, invertY, cxa, cxb, cxc, cya, cyb, cyc);
    }

    bool calibrate() {
        if (!display::TCalibrate())
            { return false; }
        bool swapXY, invertX, invertY;
        float cxa, cxb, cxc, cya, cyb, cyc;

        display::TCalibrateInfo(swapXY, invertX, invertY, cxa, cxb, cxc, cya, cyb, cyc);
        return saveCalibration(swapXY, invertX, invertY, cxa, cxb, cxc, cya, cyb, cyc);
    }

    bool begin() {
        if (loadCalibration())
            { return true; }
        while (!calibrate())
            { delay(10); }
        return true;
    }
}