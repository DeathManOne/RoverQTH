/*
 * database/nvs.cpp
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

namespace database::nvs {
    namespace {
        Preferences _prefs;
        bool _READY = false;

        constexpr const char* KEY_TOUCH_OK  = "touch_ok";
        constexpr const char* KEY_SWAP_XY   = "swap_xy";
        constexpr const char* KEY_INVERT_X  = "invert_x";
        constexpr const char* KEY_INVERT_Y  = "invert_y";
        constexpr const char* KEY_CXA       = "cxa";
        constexpr const char* KEY_CXB       = "cxb";
        constexpr const char* KEY_CXC       = "cxc";
        constexpr const char* KEY_CYA       = "cya";
        constexpr const char* KEY_CYB       = "cyb";
        constexpr const char* KEY_CYC       = "cyc";

        constexpr const char* NAMESPACE             = "roverqth";

        constexpr const char* KEY_TN_TOUCH_OK       = "tn_ok";
        constexpr const char* KEY_TN_SWAP_XY        = "tn_sxy";
        constexpr const char* KEY_TN_INVERT_X       = "tn_ix";
        constexpr const char* KEY_TN_INVERT_Y       = "tn_iy";
        constexpr const char* KEY_TN_CXA            = "tn_cxa";
        constexpr const char* KEY_TN_CXB            = "tn_cxb";
        constexpr const char* KEY_TN_CXC            = "tn_cxc";
        constexpr const char* KEY_TN_CYA            = "tn_cya";
        constexpr const char* KEY_TN_CYB            = "tn_cyb";
        constexpr const char* KEY_TN_CYC            = "tn_cyc";

        constexpr const char* KEY_TR_TOUCH_OK       = "tr_ok";
        constexpr const char* KEY_TR_SWAP_XY        = "tr_sxy";
        constexpr const char* KEY_TR_INVERT_X       = "tr_ix";
        constexpr const char* KEY_TR_INVERT_Y       = "tr_iy";
        constexpr const char* KEY_TR_CXA            = "tr_cxa";
        constexpr const char* KEY_TR_CXB            = "tr_cxb";
        constexpr const char* KEY_TR_CXC            = "tr_cxc";
        constexpr const char* KEY_TR_CYA            = "tr_cya";
        constexpr const char* KEY_TR_CYB            = "tr_cyb";
        constexpr const char* KEY_TR_CYC            = "tr_cyc";

        constexpr const char* KEY_CALLSIGN          = "callsign";
        constexpr const char* KEY_CALLSIGN_SUFFIX   = "callsignSuffix";
        constexpr const char* KEY_THEME             = "theme";
        constexpr const char* KEY_TFT_ROTATION      = "tft_rotation";
        constexpr const char* KEY_UNITS             = "units";

        constexpr const char* DEFAULT_CALLSIGN      = "SWL";
        constexpr uint8_t DEFAULT_CALLSIGN_SUFFIX   = 0;
        constexpr uint8_t DEFAULT_THEME             = 0;
        constexpr uint8_t DEFAULT_TFT_ROTATION      = 3;
        constexpr uint8_t DEFAULT_UNITS             = 0;
    }

    bool begin() {
        if (_READY)
            { return true; }
        _READY = _prefs.begin(NAMESPACE, false);
        return _READY;
    }

    bool getTouchCalibration(bool &swapXY, bool &invertX, bool &invertY, float &cxa, float &cxb, float &cxc, float &cya, float &cyb, float &cyc) {
        if (!_READY && !begin())
            { return false; }
        if (!_prefs.getBool(KEY_TOUCH_OK, false))
            { return false; }
        swapXY  = _prefs.getBool(KEY_SWAP_XY, false);
        invertX = _prefs.getBool(KEY_INVERT_X, false);
        invertY = _prefs.getBool(KEY_INVERT_Y, false);

        cxa = _prefs.getFloat(KEY_CXA, 0.0f);
        cxb = _prefs.getFloat(KEY_CXB, 0.0f);
        cxc = _prefs.getFloat(KEY_CXC, 0.0f);
        cya = _prefs.getFloat(KEY_CYA, 0.0f);
        cyb = _prefs.getFloat(KEY_CYB, 0.0f);
        cyc = _prefs.getFloat(KEY_CYC, 0.0f);

        return true;
    }

    bool setTouchCalibration(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc) {
        if (!_READY && !begin())
            { return false; }
        _prefs.putBool(KEY_SWAP_XY, swapXY);
        _prefs.putBool(KEY_INVERT_X, invertX);
        _prefs.putBool(KEY_INVERT_Y, invertY);

        _prefs.putFloat(KEY_CXA, cxa);
        _prefs.putFloat(KEY_CXB, cxb);
        _prefs.putFloat(KEY_CXC, cxc);
        _prefs.putFloat(KEY_CYA, cya);
        _prefs.putFloat(KEY_CYB, cyb);
        _prefs.putFloat(KEY_CYC, cyc);

        _prefs.putBool(KEY_TOUCH_OK, true);
        return true;
    }

    bool resetTouchCalibration() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_TOUCH_OK);
        _prefs.remove(KEY_SWAP_XY);
        _prefs.remove(KEY_INVERT_X);
        _prefs.remove(KEY_INVERT_Y);
        _prefs.remove(KEY_CXA);
        _prefs.remove(KEY_CXB);
        _prefs.remove(KEY_CXC);
        _prefs.remove(KEY_CYA);
        _prefs.remove(KEY_CYB);
        _prefs.remove(KEY_CYC);
        return true;
    }

    bool getCallsign(char* buffer, unsigned int size) {
        if (!_READY && !begin())
            { return false; }
        if (buffer == nullptr || size == 0)
            { return false; }
        const String value = _prefs.getString(KEY_CALLSIGN, DEFAULT_CALLSIGN);
        value.toCharArray(buffer, size);
        return true;
    }

    bool setCallsign(const char* callsign) {
        if (!_READY && !begin())
            { return false; }
        if (callsign == nullptr)
            { return false; }
        return _prefs.putString(KEY_CALLSIGN, callsign) > 0;
    }

    bool resetCallsign() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_CALLSIGN);
        return true;
    }

    uint8_t getCallsignSuffix() {
        if (!_READY && !begin())
            { return DEFAULT_CALLSIGN_SUFFIX; }
        return _prefs.getUChar(KEY_CALLSIGN_SUFFIX, DEFAULT_CALLSIGN_SUFFIX);
    }

    bool setCallsignSuffix(uint8_t suffix) {
        if (!_READY && !begin())
            { return false; }
        return _prefs.putUChar(KEY_CALLSIGN_SUFFIX, suffix) > 0;
    }

    bool resetCallsignSuffix() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_CALLSIGN_SUFFIX);
        return true;
    }

    uint8_t getTheme() {
        if (!_READY && !begin())
            { return DEFAULT_THEME; }
        return _prefs.getUChar(KEY_THEME, DEFAULT_THEME);
    }

    bool setTheme(uint8_t theme) {
        if (!_READY && !begin())
            { return false; }
        return _prefs.putUChar(KEY_THEME, theme) > 0;
    }

    bool resetTheme() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_THEME);
        return true;
    }

    uint8_t getTFTRotation() {
        if (!_READY && !begin())
            { return DEFAULT_TFT_ROTATION; }
        return _prefs.getUChar(KEY_TFT_ROTATION, DEFAULT_TFT_ROTATION);
    }

    bool setTFTRotation(uint8_t rotation) {
        if (!_READY && !begin())
            { return false; }
        return _prefs.putUChar(KEY_TFT_ROTATION, rotation) > 0;
    }

    bool resetTFTRotation() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_TFT_ROTATION);
        return true;
    }

    uint8_t getUnits() {
        if (!_READY && !begin())
            { return DEFAULT_UNITS; }
        return _prefs.getUChar(KEY_UNITS, DEFAULT_UNITS);
    }

    bool setUnits(uint8_t units) {
        if (!_READY && !begin())
            { return false; }
        return _prefs.putUChar(KEY_UNITS, units) > 0;
    }

    bool resetUnits() {
        if (!_READY && !begin())
            { return false; }
        _prefs.remove(KEY_UNITS);
        return true;
    }
}
