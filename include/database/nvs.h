/*
 * include/database/nvs.h
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
#include <cstddef>
#include <cstdint>

namespace database::nvs {
    bool begin();

    bool getTouchCalibrationNormal(bool &swapXY, bool &invertX, bool &invertY, float &cxa, float &cxb, float &cxc, float &cya, float &cyb, float &cyc);
    bool setTouchCalibrationNormal(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc);
    bool resetTouchCalibrationNormal();
    
    bool getTouchCalibrationReversed(bool &swapXY, bool &invertX, bool &invertY, float &cxa, float &cxb, float &cxc, float &cya, float &cyb, float &cyc);
    bool setTouchCalibrationReversed(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc);
    bool resetTouchCalibrationReversed();

    uint32_t getBatteryCapacity();
    bool setBatteryCapacity(uint32_t capacity);
    bool resetBatteryCapacity();

    float getBatteryMinimal();
    bool setBatteryMinimal(float voltage);
    bool resetBatteryMinimal();

    float getBatteryNominal();
    bool setBatteryNominal(float voltage);
    bool resetBatteryNominal();

    float getBatteryMaximal();
    bool setBatteryMaximal(float voltage);
    bool resetBatteryMaximal();

    uint8_t getBatteryRatioHigh();
    bool setBatteryRatioHigh(uint8_t ratio);
    bool resetBatteryRatioHigh();

    bool getCallsign(char* buffer, size_t size);
    bool setCallsign(const char* callsign);
    bool resetCallsign();

    uint8_t getCallsignSuffix();
    bool setCallsignSuffix(uint8_t suffix);
    bool resetCallsignSuffix();

    uint8_t getTheme();
    bool setTheme(uint8_t theme);
    bool resetTheme();

    uint8_t getTFTRotation();
    bool setTFTRotation(uint8_t rotation);
    bool resetTFTRotation();

    uint8_t getUnits();
    bool setUnits(uint8_t unit);
    bool resetUnits();

    bool getWifiSSID(char* buffer, size_t size);
    bool setWifiSSID(const char* ssid);
    bool resetWifiSSID();

    bool getWifiPassword(char* buffer, size_t size);
    bool setWifiPassword(const char* password);
    bool resetWifiPassword();

    uint8_t getWifiBootMode();
    bool setWifiBootMode(uint8_t mode);
    bool resetWifiBootMode();

    bool getWifiLastEnabled();
    bool setWifiLastEnabled(bool enabled);
    bool resetWifiLastEnabled();
}
