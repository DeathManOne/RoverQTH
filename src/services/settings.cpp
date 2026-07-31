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
    bool _checkWrite(const bool result, const char* const errorCode) {
        if (!result) { storage::appendErrorRecord(errorCode); }
        return result;
    }

    bool _isValid(const settings::CallsignSuffix suffix) {
        switch (suffix) {
            case settings::CallsignSuffix::NONE:
            case settings::CallsignSuffix::P:
            case settings::CallsignSuffix::M:
            case settings::CallsignSuffix::MM:
            case settings::CallsignSuffix::AM:
                return true;
            default:
                return false;
        }
    }

    bool _isValid(const settings::Units units) {
        switch (units) {
            case settings::Units::METRIC:
            case settings::Units::IMPERIAL:
                return true;
            default:
                return false;
        }
    }

    bool _isValid(const settings::CoordinateFormat format) {
        switch (format) {
            case settings::CoordinateFormat::DD:
            case settings::CoordinateFormat::DDM:
            case settings::CoordinateFormat::DMS:
                return true;
            default:
                return false;
        }
    }

    bool _isValid(const settings::TFTRotation rotation) {
        switch (rotation) {
            case settings::TFTRotation::NORMAL:
            case settings::TFTRotation::REVERSED:
                return true;
            default:
                return false;
        }
    }

    bool _isValid(const settings::WifiBootMode mode) {
        switch (mode) {
            case settings::WifiBootMode::NEVER:
            case settings::WifiBootMode::ALWAYS:
            case settings::WifiBootMode::LAST_STATE:
                return true;
            default:
                return false;
        }
    }
}

bool settings::begin() {
    return nvs::begin();
}

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

settings::Battery settings::battery() {
    Battery value;
    value.minimal   = nvs::getBatteryMinimal();
    value.maximal   = nvs::getBatteryMaximal();
    value.ratioHigh = nvs::getBatteryRatioHigh();
    return value;
}

bool settings::setBatteryMinimal(const float voltage) {
    const Battery configuration = battery();
    if (voltage <= 0.0f || voltage >= configuration.maximal)
        { return false; }
    return _checkWrite(nvs::setBatteryMinimal(voltage), "BATTERY_MINIMAL_SAVE_FAILED");
}

bool settings::setBatteryMaximal(const float voltage) {
    const Battery configuration = battery();
    if (voltage <= 0.0f || voltage <= configuration.minimal)
        { return false; }
    return _checkWrite(nvs::setBatteryMaximal(voltage), "BATTERY_MAXIMAL_SAVE_FAILED");
}

bool settings::setBatteryRatioHigh(uint8_t ratio) {
    if (ratio == 0 || ratio >= 100) { return false; }
    return _checkWrite(nvs::setBatteryRatioHigh(ratio), "BATTERY_RATIO_SAVE_FAILED");
}

settings::General settings::general() {
    General value;

    if (!nvs::getCallsign(value.callsign, sizeof(value.callsign)))
        { value.callsign[0] = '\0'; }

    const CallsignSuffix suffix = static_cast<CallsignSuffix>(nvs::getCallsignSuffix());
    value.suffix                = _isValid(suffix) ? suffix : CallsignSuffix::NONE;

    const Units units = static_cast<Units>(nvs::getUnits());
    value.units       = _isValid(units) ? units : Units::METRIC;

    const CoordinateFormat coordinateFormat = static_cast<CoordinateFormat>(nvs::getCoordinateFormat());
    value.coordinateFormat                  = _isValid(coordinateFormat) ? coordinateFormat : CoordinateFormat::DDM;

    return value;
}

bool settings::getFullCallsign(char* const buffer, const size_t size) {
    if (buffer == nullptr || size == 0) { return false; }

    const General value = general();
    if (value.callsign[0] == '\0') {
        buffer[0] = '\0';
        return false;
    }

    const char* const suffix = callsignSuffixText(value.suffix);
    const int written = std::snprintf(buffer, size, "%s%s", value.callsign, suffix);
    return written >= 0 && static_cast<size_t>(written) < size;
}

bool settings::setCallsign(const char* const callsign) {
    if (callsign == nullptr) { return false; }

    if (callsign[0] == '\0')
        { return _checkWrite(nvs::resetCallsign(), "CALLSIGN_RESET_FAILED"); }
    return _checkWrite(nvs::setCallsign(callsign), "CALLSIGN_SAVE_FAILED");
}

bool settings::setCallsignSuffix(const CallsignSuffix suffix) {
    if (!_isValid(suffix)) { return false; }
    return _checkWrite(nvs::setCallsignSuffix(static_cast<uint8_t>(suffix)), "CALLSIGN_SUFFIX_SAVE_FAILED");
}

bool settings::setUnits(const Units units) {
    if (!_isValid(units)) { return false; }
    return _checkWrite(nvs::setUnits(static_cast<uint8_t>(units)), "UNITS_SAVE_FAILED");
}

bool settings::setCoordinateFormat(const CoordinateFormat format) {
    if (!_isValid(format)) { return false; }
    return _checkWrite(nvs::setCoordinateFormat(static_cast<uint8_t>(format)), "COORDINATE_FORMAT_SAVE_FAILED");
}

const char* settings::callsignSuffixText(const CallsignSuffix suffix) {
    switch (suffix) {
        case CallsignSuffix::P:    return "/P";
        case CallsignSuffix::M:    return "/M";
        case CallsignSuffix::MM:   return "/MM";
        case CallsignSuffix::AM:   return "/AM";
        case CallsignSuffix::NONE:
        default:                   return "";
    }
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

bool settings::setTFTRotation(const TFTRotation rotation) {
    if (!_isValid(rotation)) { return false; }
    return _checkWrite(nvs::setTFTRotation(static_cast<uint8_t>(rotation)), "TFT_ROTATION_SAVE_FAILED");
}

settings::Wifi settings::wifi() {
    Wifi value;

    if (!nvs::getWifiSSID(value.ssid, sizeof(value.ssid)))             { value.ssid[0] = '\0'; }
    if (!nvs::getWifiPassword(value.password, sizeof(value.password))) { value.password[0] = '\0'; }

    const WifiBootMode bootMode = static_cast<WifiBootMode>(nvs::getWifiBootMode());
    value.bootMode    = _isValid(bootMode) ? bootMode : WifiBootMode::NEVER;
    value.lastEnabled = nvs::getWifiLastEnabled();
    return value;
}

bool settings::setWifiSSID(const char* const ssid) {
    if (ssid == nullptr) { return false; }
    if (ssid[0] == '\0') { return _checkWrite(nvs::resetWifiSSID(), "WIFI_SSID_RESET_FAILED"); }
    return _checkWrite(nvs::setWifiSSID(ssid), "WIFI_SSID_SAVE_FAILED");
}

bool settings::setWifiPassword(const char* const password) {
    if (password == nullptr) { return false; }
    if (password[0] == '\0') { return _checkWrite(nvs::resetWifiPassword(), "WIFI_PASSWORD_RESET_FAILED"); }
    return _checkWrite(nvs::setWifiPassword(password), "WIFI_PASSWORD_SAVE_FAILED");
}

bool settings::setWifiBootMode(const WifiBootMode mode) {
    if (!_isValid(mode)) { return false; }
    return _checkWrite(nvs::setWifiBootMode(static_cast<uint8_t>(mode)), "WIFI_BOOT_MODE_SAVE_FAILED");
}

bool settings::setWifiLastEnabled(const bool enabled) {
    return _checkWrite(nvs::setWifiLastEnabled(enabled), "WIFI_LAST_STATE_SAVE_FAILED");
}

bool settings::shouldConnectWifiAtBoot() {
    const Wifi value = wifi();

    if (value.ssid[0] == '\0') { return false; }
    switch (value.bootMode) {
        case WifiBootMode::ALWAYS:     return true;
        case WifiBootMode::LAST_STATE: return value.lastEnabled;
        case WifiBootMode::NEVER:
        default:                       return false;
    }
}
