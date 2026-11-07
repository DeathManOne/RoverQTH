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
    /** @brief Maximum stored callsign size, including its terminator. */
    constexpr size_t CALLSIGN_SIZE          = 32;

    /** @brief Maximum number of characters appended by a callsign suffix. */
    constexpr size_t CALLSIGN_SUFFIX_LENGTH = 3;

    /** @brief Capacity of a formatted callsign with its optional suffix. */
    constexpr size_t FULL_CALLSIGN_SIZE     = CALLSIGN_SIZE + CALLSIGN_SUFFIX_LENGTH;

    /** @brief Maximum stored Wi-Fi SSID size, including its terminator. */
    constexpr size_t WIFI_SSID_SIZE         = 33;

    /** @brief Maximum stored Wi-Fi password size, including its terminator. */
    constexpr size_t WIFI_PASSWORD_SIZE     = 64;

    /** @brief Maximum stored OTA reference size, including its terminator. */
    constexpr size_t OTA_CODE_SIZE          = 16;

    /** @brief Selects metric or imperial presentation units. */
    enum class Units            : uint8_t {METRIC, IMPERIAL};

    /** @brief Selects a supported physical display rotation. */
    enum class TFTRotation      : uint8_t {NORMAL = 1, REVERSED = 3};

    /** @brief Selects the portable or mobile suffix appended to a callsign. */
    enum class CallsignSuffix   : uint8_t {NONE, P, M, MM, AM};

    /** @brief Selects the displayed geographic-coordinate notation. */
    enum class CoordinateFormat : uint8_t {DD, DDM, DMS};

    /** @brief Selects when Wi-Fi is enabled during application startup. */
    enum class WifiBootMode     : uint8_t {NEVER, ALWAYS, LAST_STATE};

    /** @brief Identifies a persisted SOTA or POTA selection. */
    enum class OtaType          : uint8_t {NONE, SOTA, POTA};

    /** @brief Persistent affine calibration parameters for touch input. */
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

    /** @brief Persistent battery voltage limits and critical ratio. */
    struct Battery {
        float minimal     = 0.0f;
        float maximal     = 0.0f;
        uint8_t ratioHigh = 0;
    };

    /** @brief Persistent general display and operator settings. */
    struct General {
        char callsign[CALLSIGN_SIZE] {};
        CallsignSuffix suffix             = CallsignSuffix::NONE;
        Units units                       = Units::METRIC;
        CoordinateFormat coordinateFormat = CoordinateFormat::DDM;
    };

    /** @brief Persistent Wi-Fi credentials and startup policy. */
    struct Wifi {
        char ssid[WIFI_SSID_SIZE]         {};
        char password[WIFI_PASSWORD_SIZE] {};
        WifiBootMode bootMode = WifiBootMode::NEVER;
        bool lastEnabled      = false;
    };

    /** @brief Persistent OTA reference selected for the main screen. */
    struct OtaSelection {
        OtaType type = OtaType::NONE;
        char code[OTA_CODE_SIZE] {};
    };

    /**
     * Initializes the settings service.
     */
    void begin();

    /**
     * Retrieves the touchscreen calibration.
     * @param calibration Receives the calibration parameters.
     * @return `true` when the calibration was successfully retrieved, otherwise `false`.
     */
    bool getTouchCalibration(Calibration &calibration);

    /**
     * Stores the touchscreen calibration.
     * @param normal   Calibration for the normal display orientation.
     * @param reversed Calibration for the reversed display orientation.
     * @return `true` when the calibration was successfully stored, otherwise `false`.
     */
    bool setTouchCalibration(const Calibration &normal, const Calibration &reversed);

    /**
     * Restores the default touchscreen calibration.
     * @return `true` when the calibration was successfully reset, otherwise `false`.
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
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setBatteryMinimal(float voltage);

    /**
     * Sets the maximum battery voltage.
     * @param voltage Maximum battery voltage.
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setBatteryMaximal(float voltage);

    /**
     * @brief Sets the high-side share of the battery voltage-divider ratio.
     * @param ratio Ratio percentage from 1 to 99.
     * @return `true` when the setting was successfully updated, otherwise `false`.
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
     * @return `true` when the full callsign fits in the buffer, otherwise `false`.
     *         The buffer is cleared on failure when possible.
     */
    bool getFullCallsign(char* buffer, size_t size);

    /**
     * Sets the callsign.
     * @param callsign Callsign.
     * @return `true` when the value fits in CALLSIGN_SIZE and was saved successfully, otherwise `false`. An empty value restores the default.
     */
    bool setCallsign(const char* callsign);

    /**
     * Sets the callsign suffix.
     * @param callsignSuffix Callsign suffix.
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setCallsignSuffix(CallsignSuffix callsignSuffix);

    /**
     * Sets the unit system.
     * @param units Unit system.
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setUnits(Units units);

    /**
     * Sets the coordinate display format.
     * @param format Coordinate format.
     * @return `true` when the setting was successfully updated, otherwise `false`.
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
     * @return `true` when the setting was successfully updated, otherwise `false`.
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
     * @return `true` when the value fits in WIFI_SSID_SIZE and was saved successfully,
     *         otherwise `false`. An empty value clears the stored SSID.
     */
    bool setWifiSSID(const char* ssid);

    /**
     * Sets the Wi-Fi password.
     * @param password Wi-Fi password.
     * @return `true` when the value fits in WIFI_PASSWORD_SIZE and was saved successfully,
     *         otherwise `false`. An empty value clears the stored password.
     */
    bool setWifiPassword(const char* password);

    /**
     * Sets the Wi-Fi boot mode.
     * @param mode Wi-Fi boot mode.
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setWifiBootMode(WifiBootMode mode);

    /**
     * Stores the last Wi-Fi enabled state.
     * @param enabled `true` when Wi-Fi was enabled, otherwise `false`.
     * @return `true` when the setting was successfully updated, otherwise `false`.
     */
    bool setWifiLastEnabled(bool enabled);

    /**
     * Indicates whether Wi-Fi should be enabled during startup.
     * @return `true` when Wi-Fi should be enabled, otherwise `false`.
     */
    bool shouldConnectWifiAtBoot();

    /**
     * @brief Loads the OTA type and code selected for the main screen.
     * The output is reset to an empty selection before loading.
     * @param selection Receives the persisted OTA selection.
     * @return `true` when a valid selection was loaded, otherwise `false`.
     */
    bool getOtaSelection(OtaSelection &selection);

    /**
     * @brief Persists the OTA type and reference selected by the user.
     * @param type OTA type associated with the reference.
     * @param code Null-terminated OTA reference code.
     * @return `true` when the complete selection was stored, otherwise `false`.
     */
    bool setOtaSelection(OtaType type, const char* code);

    /**
     * @brief Removes the persisted OTA selection.
     * @return `true` when the selection was removed successfully, otherwise `false`.
     */
    bool resetOtaSelection();
}
