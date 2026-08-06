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
    constexpr size_t CALLSIGN_SIZE           = 32;
    constexpr size_t CALLSIGN_SUFFIX_LENGTH  = 3;
    constexpr size_t FULL_CALLSIGN_SIZE      = CALLSIGN_SIZE + CALLSIGN_SUFFIX_LENGTH;
    constexpr size_t WIFI_SSID_SIZE          = 33;
    constexpr size_t WIFI_PASSWORD_SIZE      = 64;

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

    /**
     * Initializes the settings service.
     * @return true if the persistent NVS backend was initialized successfully, false otherwise.
     */
    bool begin();

    /**
     * Retrieves the touchscreen calibration.
     * @param calibration Receives the calibration parameters.
     * @return true if the calibration was successfully retrieved, false otherwise.
     */
    bool getTouchCalibration(Calibration &calibration);

    /**
     * Stores the touchscreen calibration.
     * @param normal   Calibration for the normal display orientation.
     * @param reversed Calibration for the reversed display orientation.
     * @return true if the calibration was successfully stored, false otherwise.
     */
    bool setTouchCalibration(const Calibration &normal, const Calibration &reversed);

    /**
     * Restores the default touchscreen calibration.
     * @return true if the calibration was successfully reset, false otherwise.
     */
    bool resetTouchCalibration();

    /**
     * Returns the battery settings.
     * @return Current battery settings.
     */
    Battery battery();

    /**
     * Sets the minimum battery voltage.
     * @param voltage Minimum battery voltage.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setBatteryMinimal(float voltage);

    /**
     * Sets the maximum battery voltage.
     * @param voltage Maximum battery voltage.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setBatteryMaximal(float voltage);

    /**
     * @brief Sets the high-side share of the battery voltage-divider ratio.
     * @param ratio Ratio percentage from 1 to 99.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setBatteryRatioHigh(uint8_t ratio);

    /**
     * Returns the general settings.
     * @return Current general settings.
     */
    General general();

    /**
     * Builds the complete callsign.
     * @param buffer Destination buffer, preferably FULL_CALLSIGN_SIZE bytes.
     * @param size Destination buffer size.
     * @return true if the full callsign fits in the buffer, false otherwise.
     *         The buffer is cleared on failure when possible.
     */
    bool getFullCallsign(char* buffer, size_t size);

    /**
     * Sets the callsign.
     * @param callsign Callsign.
     * @return true if the value fits in CALLSIGN_SIZE and was saved successfully, false otherwise. An empty value restores the default.
     */
    bool setCallsign(const char* callsign);

    /**
     * Sets the callsign suffix.
     * @param callsignSuffix Callsign suffix.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setCallsignSuffix(CallsignSuffix callsignSuffix);

    /**
     * Sets the unit system.
     * @param units Unit system.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setUnits(Units units);

    /**
     * Sets the coordinate display format.
     * @param format Coordinate format.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setCoordinateFormat(CoordinateFormat format);

    /**
     * Returns the text associated with a callsign suffix.
     * @param suffix Callsign suffix.
     * @return Null-terminated suffix text.
     */
    const char* callsignSuffixText(CallsignSuffix suffix);

    /**
     * Returns the current TFT rotation.
     * @return Current TFT rotation.
     */
    TFTRotation getTFTRotation();

    /**
     * Sets the TFT rotation.
     * @param rotation TFT rotation.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setTFTRotation(TFTRotation rotation);

    /**
     * Returns the Wi-Fi settings.
     * @return Current Wi-Fi settings.
     */
    Wifi wifi();

    /**
     * Sets the Wi-Fi SSID.
     * @param ssid Wi-Fi SSID.
     * @return true if the value fits in WIFI_SSID_SIZE and was saved successfully,
     *         false otherwise. An empty value clears the stored SSID.
     */
    bool setWifiSSID(const char* ssid);

    /**
     * Sets the Wi-Fi password.
     * @param password Wi-Fi password.
     * @return true if the value fits in WIFI_PASSWORD_SIZE and was saved successfully,
     *         false otherwise. An empty value clears the stored password.
     */
    bool setWifiPassword(const char* password);

    /**
     * Sets the Wi-Fi boot mode.
     * @param mode Wi-Fi boot mode.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setWifiBootMode(WifiBootMode mode);

    /**
     * Stores the last Wi-Fi enabled state.
     * @param enabled true if Wi-Fi was enabled, false otherwise.
     * @return true if the setting was successfully updated, false otherwise.
     */
    bool setWifiLastEnabled(bool enabled);

    /**
     * Indicates whether Wi-Fi should be enabled during startup.
     * @return true if Wi-Fi should be enabled, false otherwise.
     */
    bool shouldConnectWifiAtBoot();
}
