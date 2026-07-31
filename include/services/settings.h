/*
 * include/services/settings.h
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

namespace services::settings {
    constexpr size_t CALLSIGN_SIZE      = 32;
    constexpr size_t WIFI_SSID_SIZE     = 33;
    constexpr size_t WIFI_PASSWORD_SIZE = 64;

    enum class Units            : uint8_t {METRIC, IMPERIAL};
    enum class TFTRotation      : uint8_t {NORMAL = 1, REVERSED = 3};
    enum class CallsignSuffix   : uint8_t {NONE, P, M, MM, AM};
    enum class CoordinateFormat : uint8_t {DD, DDM, DMS};
    enum class WifiBootMode     : uint8_t {NEVER, ALWAYS, LAST_STATE};

    struct Calibration {
        bool swapXY     = false;
        bool invertX    = false;
        bool invertY    = false;
        float coeffXA   = 0.0;
        float coeffXB   = 0.0;
        float coeffXC   = 0.0;
        float coeffYA   = 0.0;
        float coeffYB   = 0.0;
        float coeffYC   = 0.0;
    };

    struct Battery {
        float minimal     = 0.0f;
        float maximal     = 0.0f;
        uint8_t ratioHigh = 0;
    };

    struct General {
        char callsign[CALLSIGN_SIZE] {};
        CallsignSuffix suffix             = CallsignSuffix::NONE;
        Units units                       = Units::METRIC;
        CoordinateFormat coordinateFormat = CoordinateFormat::DDM;
    };

    struct Wifi {
        char ssid[WIFI_SSID_SIZE]         {};
        char password[WIFI_PASSWORD_SIZE] {};
        WifiBootMode bootMode = WifiBootMode::NEVER;
        bool lastEnabled      = false;
    };

    bool begin();

    bool getTouchCalibration(Calibration &calibration);
    bool setTouchCalibration(const Calibration &normal, const Calibration &reversed);
    bool resetTouchCalibration();

    Battery battery();
    bool setBatteryMinimal(float voltage);
    bool setBatteryMaximal(float voltage);
    bool setBatteryRatioHigh(uint8_t ratio);

    General general();
    bool getFullCallsign(char* buffer, size_t size);
    bool setCallsign(const char* callsign);
    bool setCallsignSuffix(CallsignSuffix callsignSuffix);
    bool setUnits(Units units);
    bool setCoordinateFormat(CoordinateFormat format);
    const char* callsignSuffixText(CallsignSuffix suffix);

    TFTRotation getTFTRotation();
    bool setTFTRotation(TFTRotation rotation);

    Wifi wifi();
    bool setWifiSSID(const char* ssid);
    bool setWifiPassword(const char* password);
    bool setWifiBootMode(WifiBootMode mode);
    bool setWifiLastEnabled(bool enabled);
    bool shouldConnectWifiAtBoot();
}