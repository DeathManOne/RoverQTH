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
#include <cstdint>

namespace services::settings {
    enum class Units          {METRIC, IMPERIAL};
    enum class Theme          {DEFAULTS, NIGHT, HIGHS};
    enum class TFTRotation    {NORMAL = 1, REVERSED = 3};
    enum class CallsignSuffix {NONE, P, M, MM, AM};

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

    bool begin();

    bool getTouchCalibration(Calibration &calibration);
    bool setTouchCalibration(const Calibration &normal, const Calibration &reversed);
    bool resetTouchCalibration();

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

    bool getCallsign(char* buffer, unsigned int size);
    bool setCallsign(const char* callsign);
    bool resetCallsign();

    CallsignSuffix getCallsignSuffix();
    bool setCallsignSuffix(CallsignSuffix callsignSuffix);
    bool resetCallsignSuffix();

    bool getFullCallsign(char* buffer, unsigned int size);

    Theme getTheme();
    bool setTheme(Theme theme);
    bool resetTheme();

    TFTRotation getTFTRotation();
    bool setTFTRotation(TFTRotation rotation);
    bool resetTFTRotation();

    Units getUnits();
    bool setUnits(Units units);
    bool resetUnits();

    bool resetAll();
}