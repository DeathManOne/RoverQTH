/*
 * src/database/nvs.cpp
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

#include <Preferences.h>
#include "database/nvs.h"
#include "utilities/text.h"

namespace nvs  = database::nvs;
namespace text = utilities::text;

namespace {
    Preferences _prefs;

    bool _ready = false;
    constexpr const char* NAMESPACE = "roverqth";

    struct TouchCalibration {
        bool swapXY  = false;
        bool invertX = false;
        bool invertY = false;
        float cxa    = 0.0f;
        float cxb    = 0.0f;
        float cxc    = 0.0f;
        float cya    = 0.0f;
        float cyb    = 0.0f;
        float cyc    = 0.0f;
    };

    struct TouchCalibrationKeys {
        const char* valid;
        const char* swapXY;
        const char* invertX;
        const char* invertY;
        const char* cxa;
        const char* cxb;
        const char* cxc;
        const char* cya;
        const char* cyb;
        const char* cyc;
    };

    // TOUCHSCREEN NORMAL
    constexpr TouchCalibrationKeys TOUCH_NORMAL_KEYS {
        "tn_ok",
        "tn_sxy", "tn_ix",  "tn_iy",
        "tn_cxa", "tn_cxb", "tn_cxc",
        "tn_cya", "tn_cyb", "tn_cyc"
    };

    // TOUCHSCREEN REVERSED
    constexpr TouchCalibrationKeys TOUCH_REVERSED_KEYS {
        "tr_ok",
        "tr_sxy", "tr_ix",  "tr_iy",
        "tr_cxa", "tr_cxb", "tr_cxc",
        "tr_cya", "tr_cyb", "tr_cyc"
    };

    // GENERAL
    constexpr const char* KEY_CALLSIGN          = "callsign";
    constexpr const char* KEY_CALLSIGN_SUFFIX   = "callsign_suffix";
    constexpr const char* KEY_TFT_ROTATION      = "tft_rotation";
    constexpr const char* KEY_UNITS             = "units";
    constexpr const char* KEY_COORDINATE_FORMAT = "coord_format";

    constexpr const char* DEFAULT_CALLSIGN      = "SWL";
    constexpr uint8_t DEFAULT_CALLSIGN_SUFFIX   = 0;
    constexpr uint8_t DEFAULT_TFT_ROTATION      = 1;
    constexpr uint8_t DEFAULT_UNITS             = 0;
    constexpr uint8_t DEFAULT_COORDINATE_FORMAT = 1;

    // BATTERY
    constexpr const char* KEY_BATT_MINIMAL      = "batt_min";
    constexpr const char* KEY_BATT_MAXIMAL      = "batt_max";
    constexpr const char* KEY_BATT_RATIO_HIGH   = "batt_ratio_high";

    constexpr float DEFAULT_BATT_MINIMAL        = 3.30f;
    constexpr float DEFAULT_BATT_MAXIMAL        = 4.20f;
    constexpr uint8_t DEFAULT_BATT_RATIO_HIGH   = 50;

    // WIFI
    constexpr const char* KEY_WIFI_SSID         = "wifi_ssid";
    constexpr const char* KEY_WIFI_PASSWORD     = "wifi_pass";
    constexpr const char* KEY_WIFI_BOOT_MODE    = "wifi_boot";
    constexpr const char* KEY_WIFI_LAST_ENABLED = "wifi_last";

    constexpr const char* DEFAULT_WIFI_SSID     = "";
    constexpr const char* DEFAULT_WIFI_PASSWORD = "";
    constexpr uint8_t DEFAULT_WIFI_BOOT_MODE    = 0;
    constexpr bool DEFAULT_WIFI_LAST_ENABLED    = false;

    // OTA SELECTION
    constexpr const char* KEY_OTA_TYPE = "ota_type";
    constexpr const char* KEY_OTA_CODE = "ota_code";

    bool _getTouchCalibration(const TouchCalibrationKeys& keys, TouchCalibration& calibration) {
        if (!_ready && !nvs::begin())           { return false; }
        if (!_prefs.getBool(keys.valid, false)) { return false; }

        TouchCalibration stored {};
        stored.swapXY  = _prefs.getBool(keys.swapXY,  false);
        stored.invertX = _prefs.getBool(keys.invertX, false);
        stored.invertY = _prefs.getBool(keys.invertY, false);
        stored.cxa     = _prefs.getFloat(keys.cxa, 0.0f);
        stored.cxb     = _prefs.getFloat(keys.cxb, 0.0f);
        stored.cxc     = _prefs.getFloat(keys.cxc, 0.0f);
        stored.cya     = _prefs.getFloat(keys.cya, 0.0f);
        stored.cyb     = _prefs.getFloat(keys.cyb, 0.0f);
        stored.cyc     = _prefs.getFloat(keys.cyc, 0.0f);

        calibration = stored;
        return true;
    }

    bool _setTouchCalibration(const TouchCalibrationKeys& keys, const TouchCalibration& calibration) {
        if (!_ready && !nvs::begin()) { return false;}

        bool success = true;
        success = _prefs.putBool(keys.swapXY,  calibration.swapXY)  > 0U && success;
        success = _prefs.putBool(keys.invertX, calibration.invertX) > 0U && success;
        success = _prefs.putBool(keys.invertY, calibration.invertY) > 0U && success;
        success = _prefs.putFloat(keys.cxa, calibration.cxa) > 0U && success;
        success = _prefs.putFloat(keys.cxb, calibration.cxb) > 0U && success;
        success = _prefs.putFloat(keys.cxc, calibration.cxc) > 0U && success;
        success = _prefs.putFloat(keys.cya, calibration.cya) > 0U && success;
        success = _prefs.putFloat(keys.cyb, calibration.cyb) > 0U && success;
        success = _prefs.putFloat(keys.cyc, calibration.cyc) > 0U && success;

        if (!success) {
            _prefs.remove(keys.valid);
            return false;
        }
        return _prefs.putBool(keys.valid, true) > 0U;
    }

    bool _resetTouchCalibration(const TouchCalibrationKeys& keys) {
        if (!_ready && !nvs::begin()) { return false; }

        _prefs.remove(keys.valid);
        _prefs.remove(keys.swapXY);
        _prefs.remove(keys.invertX);
        _prefs.remove(keys.invertY);
        _prefs.remove(keys.cxa);
        _prefs.remove(keys.cxb);
        _prefs.remove(keys.cxc);
        _prefs.remove(keys.cya);
        _prefs.remove(keys.cyb);
        _prefs.remove(keys.cyc);

        return true;
    }
}

bool nvs::begin() {
    if (_ready) { return true; }
    _ready = _prefs.begin(NAMESPACE, false);
    return _ready;
}

bool nvs::getTouchCalibrationNormal(
    bool& swapXY,  bool& invertX, bool& invertY,
    float& cxa,    float& cxb,    float& cxc,
    float& cya,    float& cyb,    float& cyc
) {
    TouchCalibration calibration {};
    if (!_getTouchCalibration(TOUCH_NORMAL_KEYS, calibration)) { return false; }

    swapXY  = calibration.swapXY;
    invertX = calibration.invertX;
    invertY = calibration.invertY;
    cxa     = calibration.cxa;
    cxb     = calibration.cxb;
    cxc     = calibration.cxc;
    cya     = calibration.cya;
    cyb     = calibration.cyb;
    cyc     = calibration.cyc;

    return true;
}

bool nvs::setTouchCalibrationNormal(
    const bool swapXY, const bool invertX, const bool invertY,
    const float cxa,   const float cxb,    const float cxc,
    const float cya,   const float cyb,    const float cyc
) {
    TouchCalibration calibration {};
    calibration.swapXY  = swapXY;
    calibration.invertX = invertX;
    calibration.invertY = invertY;
    calibration.cxa     = cxa;
    calibration.cxb     = cxb;
    calibration.cxc     = cxc;
    calibration.cya     = cya;
    calibration.cyb     = cyb;
    calibration.cyc     = cyc;

    return _setTouchCalibration(TOUCH_NORMAL_KEYS, calibration);
}

bool nvs::resetTouchCalibrationNormal() {
    return _resetTouchCalibration(TOUCH_NORMAL_KEYS);
}

bool nvs::getTouchCalibrationReversed(
    bool& swapXY, bool& invertX, bool& invertY,
    float& cxa,   float& cxb,    float& cxc,
    float& cya,   float& cyb,    float& cyc
) {
    TouchCalibration calibration {};

    if (!_getTouchCalibration(TOUCH_REVERSED_KEYS, calibration)) { return false; }

    swapXY  = calibration.swapXY;
    invertX = calibration.invertX;
    invertY = calibration.invertY;
    cxa     = calibration.cxa;
    cxb     = calibration.cxb;
    cxc     = calibration.cxc;
    cya     = calibration.cya;
    cyb     = calibration.cyb;
    cyc     = calibration.cyc;

    return true;
}

bool nvs::setTouchCalibrationReversed(
    const bool swapXY, const bool invertX, const bool invertY,
    const float cxa,   const float cxb,    const float cxc,
    const float cya,   const float cyb,    const float cyc
) {
    TouchCalibration calibration {};
    calibration.swapXY  = swapXY;
    calibration.invertX = invertX;
    calibration.invertY = invertY;
    calibration.cxa     = cxa;
    calibration.cxb     = cxb;
    calibration.cxc     = cxc;
    calibration.cya     = cya;
    calibration.cyb     = cyb;
    calibration.cyc     = cyc;

    return _setTouchCalibration(TOUCH_REVERSED_KEYS, calibration);
}

bool nvs::resetTouchCalibrationReversed() {
    return _resetTouchCalibration(TOUCH_REVERSED_KEYS);
}

float nvs::getBatteryMinimal() {
    if (!_ready && !begin())
        { return DEFAULT_BATT_MINIMAL; }
    return _prefs.getFloat(KEY_BATT_MINIMAL,  DEFAULT_BATT_MINIMAL);
}

bool nvs::setBatteryMinimal(float voltage) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putFloat(KEY_BATT_MINIMAL, voltage) > 0;
}

float nvs::getBatteryMaximal() {
    if (!_ready && !begin())
        { return DEFAULT_BATT_MAXIMAL; }
    return _prefs.getFloat(KEY_BATT_MAXIMAL,  DEFAULT_BATT_MAXIMAL);
}

bool nvs::setBatteryMaximal(float voltage) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putFloat(KEY_BATT_MAXIMAL, voltage) > 0;
}

uint8_t nvs::getBatteryRatioHigh() {
    if (!_ready && !begin())
        { return DEFAULT_BATT_RATIO_HIGH; }
    return _prefs.getUChar(KEY_BATT_RATIO_HIGH, DEFAULT_BATT_RATIO_HIGH);
}

bool nvs::setBatteryRatioHigh(uint8_t ratio) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putUChar(KEY_BATT_RATIO_HIGH, ratio) > 0;
}

bool nvs::getCallsign(char* buffer, size_t size) {
    if (!_ready && !begin())              { return false; }
    if (buffer == nullptr || size == 0U)  { return false; }

    const String value = _prefs.getString(KEY_CALLSIGN, DEFAULT_CALLSIGN);
    return text::copy(buffer, size, value.c_str());
}

bool nvs::setCallsign(const char* callsign) {
    if (!_ready && !begin()) { return false; }
    if (callsign == nullptr) { return false; }
    return _prefs.putString(KEY_CALLSIGN, callsign) > 0;
}

bool nvs::resetCallsign() {
    if (!_ready && !begin())
        { return false; }
    _prefs.remove(KEY_CALLSIGN);
    return true;
}

uint8_t nvs::getCallsignSuffix() {
    if (!_ready && !begin())
        { return DEFAULT_CALLSIGN_SUFFIX; }
    return _prefs.getUChar(KEY_CALLSIGN_SUFFIX, DEFAULT_CALLSIGN_SUFFIX);
}

bool nvs::setCallsignSuffix(uint8_t suffix) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putUChar(KEY_CALLSIGN_SUFFIX, suffix) > 0;
}

uint8_t nvs::getTFTRotation() {
    if (!_ready && !begin())
        { return DEFAULT_TFT_ROTATION; }
    return _prefs.getUChar(KEY_TFT_ROTATION, DEFAULT_TFT_ROTATION);
}

bool nvs::setTFTRotation(uint8_t rotation) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putUChar(KEY_TFT_ROTATION, rotation) > 0;
}

uint8_t nvs::getUnits() {
    if (!_ready && !begin())
        { return DEFAULT_UNITS; }
    return _prefs.getUChar(KEY_UNITS, DEFAULT_UNITS);
}

bool nvs::setUnits(uint8_t units) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putUChar(KEY_UNITS, units) > 0;
}

uint8_t nvs::getCoordinateFormat() {
    if (!_ready && !begin())
        { return DEFAULT_COORDINATE_FORMAT; }
    return _prefs.getUChar(KEY_COORDINATE_FORMAT, DEFAULT_COORDINATE_FORMAT);
}

bool nvs::setCoordinateFormat(uint8_t format) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putUChar(KEY_COORDINATE_FORMAT, format) > 0;
}

bool nvs::getWifiSSID(char* buffer, size_t size) {
    if (!_ready && !begin())  { return false; }
    if (!buffer || size == 0) { return false; }

    const String value = _prefs.getString(KEY_WIFI_SSID, DEFAULT_WIFI_SSID);
    return text::copy(buffer, size, value.c_str());
}

bool nvs::setWifiSSID(const char* ssid) {
    if (!_ready && !begin()) { return false; }
    if (!ssid)               { return false; }
    return (_prefs.putString(KEY_WIFI_SSID, ssid) > 0);
}

bool nvs::resetWifiSSID() {
    if (!_ready && !begin())
        { return false; }
    _prefs.remove(KEY_WIFI_SSID);
    return true;
}

bool nvs::getWifiPassword(char* buffer, size_t size) {
    if (!_ready && !begin())  { return false; }
    if (!buffer || size == 0) { return false; }

    const String value = _prefs.getString(KEY_WIFI_PASSWORD, DEFAULT_WIFI_PASSWORD);
    return text::copy(buffer, size, value.c_str());
}

bool nvs::setWifiPassword(const char* password) {
    if (!_ready && !begin()) { return false; }
    if (!password)           { return false; }

    if (password[0] == '\0') {
        _prefs.remove(KEY_WIFI_PASSWORD);
        return true;
    }
    return (_prefs.putString(KEY_WIFI_PASSWORD, password) > 0);
}

bool nvs::resetWifiPassword() {
    if (!_ready && !begin())
        { return false; }
    _prefs.remove(KEY_WIFI_PASSWORD);
    return true;
}

uint8_t nvs::getWifiBootMode() {
    if ( !_ready && !begin())
        { return DEFAULT_WIFI_BOOT_MODE; }
    return _prefs.getUChar(KEY_WIFI_BOOT_MODE, DEFAULT_WIFI_BOOT_MODE);
}

bool nvs::setWifiBootMode(uint8_t mode) {
    if (!_ready && !begin())
        { return false; }
    return (_prefs.putUChar(KEY_WIFI_BOOT_MODE, mode) > 0);
}

bool nvs::getWifiLastEnabled() {
    if (!_ready && !begin())
        { return DEFAULT_WIFI_LAST_ENABLED; }
    return _prefs.getBool(KEY_WIFI_LAST_ENABLED, DEFAULT_WIFI_LAST_ENABLED);
}

bool nvs::setWifiLastEnabled(bool enabled) {
    if (!_ready && !begin())
        { return false; }
    return _prefs.putBool(KEY_WIFI_LAST_ENABLED, enabled) > 0U;
}

bool nvs::getOtaSelection(char* const type, const size_t typeSize, char* const code, const size_t codeSize) {
    if (!_ready && !begin())                                                    { return false; }
    if (type == nullptr || typeSize == 0U || code == nullptr || codeSize == 0U) { return false; }

    const String storedType = _prefs.getString(KEY_OTA_TYPE, "");
    const String storedCode = _prefs.getString(KEY_OTA_CODE, "");

    if ((storedType != "SOTA" && storedType != "POTA") || storedCode.length() == 0U ||
        storedType.length() >= typeSize                || storedCode.length() >= codeSize
    ) { return false; }

    return
        text::copy(type, typeSize, storedType.c_str()) &&
        text::copy(code, codeSize, storedCode.c_str());
}

bool nvs::setOtaSelection(const char* const type, const char* const code) {
    if (!_ready && !begin()) { return false; }

    if (type == nullptr || code == nullptr || code[0] == '\0' ||
        (!text::equals(type, "SOTA") && !text::equals(type, "POTA"))
    ) { return false; }

    _prefs.remove(KEY_OTA_TYPE);
    if (_prefs.isKey(KEY_OTA_TYPE)) { return false; }
    if (_prefs.putString(KEY_OTA_CODE, code) == 0U) {
        _prefs.remove(KEY_OTA_CODE);
        return false;
    }
    if (_prefs.putString(KEY_OTA_TYPE, type) == 0U) {
        _prefs.remove(KEY_OTA_TYPE);
        _prefs.remove(KEY_OTA_CODE);
        return false;
    }
    return true;
}

bool nvs::resetOtaSelection() {
    if (!_ready && !begin()) { return false; }

    _prefs.remove(KEY_OTA_TYPE);
    _prefs.remove(KEY_OTA_CODE);

    return !_prefs.isKey(KEY_OTA_TYPE) && !_prefs.isKey(KEY_OTA_CODE);
}
