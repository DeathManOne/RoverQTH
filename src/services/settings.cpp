/*
 * services/settings.cpp
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
//#include <cstring>
#include "database/nvs.h"
#include "services/settings.h"

namespace services::settings {
    bool begin() {
        return database::nvs::begin();
    }

    bool getTouchCalibration(Calibration &calibration) {
        switch (getTFTRotation()) {
            case TFTRotation::NORMAL:
                return database::nvs::getTouchCalibrationNormal(
                    calibration.swapXY,     calibration.invertX, calibration.invertY,
                    calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
                    calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
                );
            case TFTRotation::REVERSED:
                return database::nvs::getTouchCalibrationReversed(
                    calibration.swapXY,     calibration.invertX, calibration.invertY,
                    calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
                    calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
                );
            default:
                return false;
        }
    }

    bool setTouchCalibration(const Calibration &normal, const Calibration &reversed) {
        bool ok = true;
        ok = database::nvs::setTouchCalibrationNormal(
            normal.swapXY,     normal.invertX, normal.invertY,
            normal.coeffXA,    normal.coeffXB, normal.coeffXC,
            normal.coeffYA,    normal.coeffYB, normal.coeffYC
        ) && ok;
        ok = database::nvs::setTouchCalibrationReversed(
            reversed.swapXY,    reversed.invertX, reversed.invertY,
            reversed.coeffXA,   reversed.coeffXB, reversed.coeffXC,
            reversed.coeffYA,   reversed.coeffYB, reversed.coeffYC
        ) && ok;
        return ok;
    }

    bool resetTouchCalibration() {
        bool ok = true;
        ok = database::nvs::resetTouchCalibrationNormal()   && ok;
        ok = database::nvs::resetTouchCalibrationReversed() && ok;
        return ok;
    }

    bool getCallsign(char* buffer, unsigned int size) {
        return database::nvs::getCallsign(buffer, size);
    }

    bool setCallsign(const char* callsign) {
        return database::nvs::setCallsign(callsign);
    }

    bool resetCallsign() {
        return database::nvs::resetCallsign();
    }

    CallsignSuffix getCallsignSuffix() {
        uint8_t value = database::nvs::getCallsignSuffix();
        switch (static_cast<CallsignSuffix>(value)) {
            case CallsignSuffix::NONE:
            case CallsignSuffix::P:
            case CallsignSuffix::M:
            case CallsignSuffix::MM:
            case CallsignSuffix::AM:
                return static_cast<CallsignSuffix>(value);
            default:
                return CallsignSuffix::NONE;
        }
    }

    bool setCallsignSuffix(CallsignSuffix callsignSuffix) {
        return database::nvs::setCallsignSuffix(static_cast<uint8_t>(callsignSuffix));
    }

    bool resetCallsignSuffix() {
        return database::nvs::resetCallsignSuffix();
    }

    bool getFullCallsign(char* buffer, unsigned int size) {
        if (buffer == nullptr || size == 0)
            { return false; }
        char callsign[32];
        if (!services::settings::getCallsign(callsign, sizeof(callsign)))
            { return false; }
        const auto suffix = services::settings::getCallsignSuffix();
        const char* suffixText = "";

        switch (suffix) {
            case services::settings::CallsignSuffix::NONE:
                suffixText = "";
                break;
            case services::settings::CallsignSuffix::P:
                suffixText = "/P";
                break;
            case services::settings::CallsignSuffix::M:
                suffixText = "/M";
                break;
            case services::settings::CallsignSuffix::MM:
                suffixText = "/MM";
                break;
            case services::settings::CallsignSuffix::AM:
                suffixText = "/AM";
                break;
        }
        snprintf(buffer, size, "%s%s", callsign, suffixText);
        return true;
    }

    Theme getTheme() {
        uint8_t value = database::nvs::getTheme();
        switch (static_cast<Theme>(value)) {
            case Theme::DEFAULTS:
            case Theme::NIGHT:
            case Theme::HIGHS:
                return static_cast<Theme>(value);
            default:
                return Theme::DEFAULTS;
        }
    }

    bool setTheme(Theme theme) {
        return database::nvs::setTheme(static_cast<uint8_t>(theme));
    }

    bool resetTheme() {
        return database::nvs::resetTheme();
    }

    TFTRotation getTFTRotation() {
        uint8_t value = database::nvs::getTFTRotation();
        switch (static_cast<TFTRotation>(value)) {
            case TFTRotation::NORMAL:
            case TFTRotation::REVERSED:
                return static_cast<TFTRotation>(value);
            default:
                return TFTRotation::NORMAL;
        }
    }

    bool setTFTRotation(TFTRotation rotation) {
        return database::nvs::setTFTRotation(static_cast<uint8_t>(rotation));
    }

    bool resetTFTRotation() {
        return database::nvs::resetTFTRotation();
    }

    Units getUnits() {
        uint8_t value = database::nvs::getUnits();
        switch (static_cast<Units>(value)) {
            case Units::METRIC:
            case Units::IMPERIAL:
                return static_cast<Units>(value);
            default:
                return Units::METRIC;
        }
    }

    bool setUnits(Units units) {
        return database::nvs::setUnits(static_cast<uint8_t>(units));
    }

    bool resetUnits() {
        return database::nvs::resetUnits();
    }

    bool resetAll() {
        bool ok = true;
        ok = resetTouchCalibration()    && ok;
        ok = resetCallsign()            && ok;
        ok = resetCallsignSuffix()      && ok;
        ok = resetTheme()               && ok;
        ok = resetTFTRotation()         && ok;
        ok = resetUnits()               && ok;
        return ok;
    }
}