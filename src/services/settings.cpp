/*
 * src/services/settings.cpp
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
#include "database/nvs.h"
#include "services/settings.h"
#include "services/storage.h"

namespace nvs      = database::nvs;
namespace settings = services::settings;
namespace storage  = services::storage;

namespace {
    bool _checkWrite(bool result, const char* errorCode) {
        if (!result) { storage::appendErrorRecord(errorCode); }
        return result;
    }
}

bool settings::begin() { return nvs::begin(); }

bool settings::getTouchCalibration(Calibration &calibration) {
    switch (getTFTRotation()) {
        case TFTRotation::NORMAL:
            return nvs::getTouchCalibrationNormal(
                calibration.swapXY,     calibration.invertX, calibration.invertY,
                calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
                calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
            );
        case TFTRotation::REVERSED:
            return nvs::getTouchCalibrationReversed(
                calibration.swapXY,     calibration.invertX, calibration.invertY,
                calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
                calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
            );
        default:
            return false;
    }
}

bool settings::setTouchCalibration(const Calibration &normal, const Calibration &reversed) {
    bool ok = true;
    ok = nvs::setTouchCalibrationNormal(
        normal.swapXY,     normal.invertX, normal.invertY,
        normal.coeffXA,    normal.coeffXB, normal.coeffXC,
        normal.coeffYA,    normal.coeffYB, normal.coeffYC
    ) && ok;
    ok = nvs::setTouchCalibrationReversed(
        reversed.swapXY,    reversed.invertX, reversed.invertY,
        reversed.coeffXA,   reversed.coeffXB, reversed.coeffXC,
        reversed.coeffYA,   reversed.coeffYB, reversed.coeffYC
    ) && ok;
    return _checkWrite(ok, "TOUCH_CALIBRATION_SAVE_FAILED");
}

bool settings::resetTouchCalibration() {
    bool ok = true;
    ok = nvs::resetTouchCalibrationNormal()   && ok;
    ok = nvs::resetTouchCalibrationReversed() && ok;
    return _checkWrite(ok, "TOUCH_CALIBRATION_RESET_FAILED");
}

uint32_t settings::getBatteryCapacity() {
    return nvs::getBatteryCapacity();
}

bool settings::setBatteryCapacity(uint32_t capacity) {
    return _checkWrite(nvs::setBatteryCapacity(capacity), "BATTERY_CAPACITY_SAVE_FAILED");
}

bool settings::resetBatteryCapacity() {
    return _checkWrite(nvs::resetBatteryCapacity(), "BATTERY_CAPACITY_RESET_FAILED");
}

float settings::getBatteryMinimal() {
    return nvs::getBatteryMinimal();
}

bool settings::setBatteryMinimal(float voltage) {
    return _checkWrite(nvs::setBatteryMinimal(voltage), "BATTERY_MINIMAL_SAVE_FAILED");
}

bool settings::resetBatteryMinimal() {
    return _checkWrite(nvs::resetBatteryMinimal(), "BATTERY_MINIMAL_RESET_FAILED");
}

float settings::getBatteryNominal() {
    return nvs::getBatteryNominal();
}

bool settings::setBatteryNominal(float voltage) {
    return _checkWrite(nvs::setBatteryNominal(voltage), "BATTERY_NOMINAL_SAVE_FAILED");
}

bool settings::resetBatteryNominal() {
    return _checkWrite(nvs::resetBatteryNominal(), "BATTERY_NOMINAL_RESET_FAILED");
}

float settings::getBatteryMaximal() {
    return nvs::getBatteryMaximal();
}

bool settings::setBatteryMaximal(float voltage) {
    return _checkWrite(nvs::setBatteryMaximal(voltage), "BATTERY_MAXIMAL_SAVE_FAILED");
}

bool settings::resetBatteryMaximal() {
    return _checkWrite(nvs::resetBatteryMaximal(), "BATTERY_MAXIMAL_RESET_FAILED");
}

uint8_t settings::getBatteryRatioHigh() {
    return nvs::getBatteryRatioHigh();
}

bool settings::setBatteryRatioHigh(uint8_t ratio) {
    return _checkWrite(nvs::setBatteryRatioHigh(ratio), "BATTERY_RATIO_SAVE_FAILED");
}

bool settings::resetBatteryRatioHigh() {
    return _checkWrite(nvs::resetBatteryRatioHigh(), "BATTERY_RATIO_RESET_FAILED");
}

bool settings::getCallsign(char* buffer, unsigned int size) {
    return nvs::getCallsign(buffer, size);
}

bool settings::setCallsign(const char* callsign) {
    return _checkWrite(nvs::setCallsign(callsign), "CALLSIGN_SAVE_FAILED");
}

bool settings::resetCallsign() {
    return _checkWrite(nvs::resetCallsign(), "CALLSIGN_RESET_FAILED");
}

settings::CallsignSuffix settings::getCallsignSuffix() {
    uint8_t value = nvs::getCallsignSuffix();
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

bool settings::setCallsignSuffix(CallsignSuffix callsignSuffix) {
    return _checkWrite(nvs::setCallsignSuffix(static_cast<uint8_t>(callsignSuffix)), "CALLSIGN_SUFFIX_SAVE_FAILED");
}

bool settings::resetCallsignSuffix() {
    return _checkWrite(nvs::resetCallsignSuffix(), "CALLSIGN_SUFFIX_RESET_FAILED");
}

bool settings::getFullCallsign(char* buffer, unsigned int size) {
    if (buffer == nullptr || size == 0) { return false; }

    char callsign[32];
    if (!getCallsign(callsign, sizeof(callsign))) { return false; }

    const auto suffix = getCallsignSuffix();
    const char* suffixText = "";

    switch (suffix) {
        case CallsignSuffix::NONE:
            suffixText = "";
            break;
        case CallsignSuffix::P:
            suffixText = "/P";
            break;
        case CallsignSuffix::M:
            suffixText = "/M";
            break;
        case CallsignSuffix::MM:
            suffixText = "/MM";
            break;
        case CallsignSuffix::AM:
            suffixText = "/AM";
            break;
    }
    snprintf(buffer, size, "%s%s", callsign, suffixText);
    return true;
}

settings::Theme settings::getTheme() {
    uint8_t value = nvs::getTheme();
    switch (static_cast<Theme>(value)) {
        case Theme::DEFAULTS:
        case Theme::NIGHT:
        case Theme::HIGHS:
            return static_cast<Theme>(value);
        default:
            return Theme::DEFAULTS;
    }
}

bool settings::setTheme(Theme theme) {
    return _checkWrite(nvs::setTheme(static_cast<uint8_t>(theme)), "THEME_SAVE_FAILED");
}

bool settings::resetTheme() {
    return _checkWrite(nvs::resetTheme(), "THEME_RESET_FAILED");
}

settings::TFTRotation settings::getTFTRotation() {
    uint8_t value = nvs::getTFTRotation();
    switch (static_cast<TFTRotation>(value)) {
        case TFTRotation::NORMAL:
        case TFTRotation::REVERSED:
            return static_cast<TFTRotation>(value);
        default:
            return TFTRotation::NORMAL;
    }
}

bool settings::setTFTRotation(TFTRotation rotation) {
    return _checkWrite(nvs::setTFTRotation(static_cast<uint8_t>(rotation)), "TFT_ROTATION_SAVE_FAILED");
}

bool settings::resetTFTRotation() {
    return _checkWrite(nvs::resetTFTRotation(), "TFT_ROTATION_RESET_FAILED");
}

settings::Units settings::getUnits() {
    uint8_t value = nvs::getUnits();
    switch (static_cast<Units>(value)) {
        case Units::METRIC:
        case Units::IMPERIAL:
            return static_cast<Units>(value);
        default:
            return Units::METRIC;
    }
}

bool settings::setUnits(Units units) {
    return _checkWrite(nvs::setUnits(static_cast<uint8_t>(units)), "UNITS_SAVE_FAILED");
}

bool settings::resetUnits() {
    return _checkWrite(database::nvs::resetUnits(), "UNITS_RESET_FAILED");
}

bool settings::resetAll() {
    bool ok = true;
    ok = resetTouchCalibration()    && ok;
    ok = resetBatteryCapacity()     && ok;
    ok = resetBatteryMinimal()      && ok;
    ok = resetBatteryNominal()      && ok;
    ok = resetBatteryMaximal()      && ok;
    ok = resetBatteryRatioHigh()    && ok;
    ok = resetCallsign()            && ok;
    ok = resetCallsignSuffix()      && ok;
    ok = resetTheme()               && ok;
    ok = resetTFTRotation()         && ok;
    ok = resetUnits()               && ok;
    return ok;
}
