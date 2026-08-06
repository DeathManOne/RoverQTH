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
    /**
     * Initializes the RoverQTH NVS namespace.
     * Repeated calls are harmless after a successful initialization.
     * @return true if the namespace is ready, false otherwise.
     */
    bool begin();

    /**
     * Retrieves the touchscreen calibration for the normal display orientation.
     * Output parameters are modified only when a stored calibration is available.
     * @param swapXY Receives whether the X and Y axes are swapped.
     * @param invertX Receives whether the X axis is inverted.
     * @param invertY Receives whether the Y axis is inverted.
     * @param cxa Receives the X-axis coefficient A.
     * @param cxb Receives the X-axis coefficient B.
     * @param cxc Receives the X-axis coefficient C.
     * @param cya Receives the Y-axis coefficient A.
     * @param cyb Receives the Y-axis coefficient B.
     * @param cyc Receives the Y-axis coefficient C.
     * @return true if a complete stored calibration was retrieved, false otherwise.
     */
    bool getTouchCalibrationNormal(bool &swapXY, bool &invertX, bool &invertY, float &cxa, float &cxb, float &cxc, float &cya, float &cyb, float &cyc);

    /**
     * Stores the touchscreen calibration for the normal display orientation.
     * The calibration is marked as valid only after every calibration value is written.
     * @param swapXY Whether the X and Y axes are swapped.
     * @param invertX Whether the X axis is inverted.
     * @param invertY Whether the Y axis is inverted.
     * @param cxa X-axis coefficient A.
     * @param cxb X-axis coefficient B.
     * @param cxc X-axis coefficient C.
     * @param cya Y-axis coefficient A.
     * @param cyb Y-axis coefficient B.
     * @param cyc Y-axis coefficient C.
     * @return true if the complete calibration was stored, false otherwise.
     */
    bool setTouchCalibrationNormal(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc);

    /**
     * Requests removal of the touchscreen calibration for the normal display orientation.
     * @return true if the NVS namespace is available, false otherwise.
     */
    bool resetTouchCalibrationNormal();

    /**
     * Retrieves the touchscreen calibration for the reversed display orientation.
     * Output parameters are modified only when a stored calibration is available.
     * @param swapXY Receives whether the X and Y axes are swapped.
     * @param invertX Receives whether the X axis is inverted.
     * @param invertY Receives whether the Y axis is inverted.
     * @param cxa Receives the X-axis coefficient A.
     * @param cxb Receives the X-axis coefficient B.
     * @param cxc Receives the X-axis coefficient C.
     * @param cya Receives the Y-axis coefficient A.
     * @param cyb Receives the Y-axis coefficient B.
     * @param cyc Receives the Y-axis coefficient C.
     * @return true if a complete stored calibration was retrieved, false otherwise.
     */
    bool getTouchCalibrationReversed(bool &swapXY, bool &invertX, bool &invertY, float &cxa, float &cxb, float &cxc, float &cya, float &cyb, float &cyc);

    /**
     * Stores the touchscreen calibration for the reversed display orientation.
     * The calibration is marked as valid only after every calibration value is written.
     * @param swapXY Whether the X and Y axes are swapped.
     * @param invertX Whether the X axis is inverted.
     * @param invertY Whether the Y axis is inverted.
     * @param cxa X-axis coefficient A.
     * @param cxb X-axis coefficient B.
     * @param cxc X-axis coefficient C.
     * @param cya Y-axis coefficient A.
     * @param cyb Y-axis coefficient B.
     * @param cyc Y-axis coefficient C.
     * @return true if the complete calibration was stored, false otherwise.
     */
    bool setTouchCalibrationReversed(bool swapXY, bool invertX, bool invertY, float cxa, float cxb, float cxc, float cya, float cyb, float cyc);

    /**
     * Requests removal of the touchscreen calibration for the reversed display orientation.
     * @return true if the NVS namespace is available, false otherwise.
     */
    bool resetTouchCalibrationReversed();

    /**
     * Returns the configured minimum battery voltage.
     * @return Stored voltage, or the default value if unavailable.
     */
    float getBatteryMinimal();

    /**
     * Stores the minimum battery voltage.
     * @param voltage Voltage in volts.
     * @return true if the value was stored, false otherwise.
     */
    bool setBatteryMinimal(float voltage);

    /**
     * Returns the configured maximum battery voltage.
     * @return Stored voltage, or the default value if unavailable.
     */
    float getBatteryMaximal();

    /**
     * Stores the maximum battery voltage.
     * @param voltage Voltage in volts.
     * @return true if the value was stored, false otherwise.
     */
    bool setBatteryMaximal(float voltage);

    /**
     * Returns the high-side share of the battery voltage-divider ratio.
     * @return Stored percentage, or the default value if unavailable.
     */
    uint8_t getBatteryRatioHigh();

    /**
     * Stores the high-side share of the battery voltage-divider ratio.
     * @param ratio Ratio percentage.
     * @return true if the value was stored, false otherwise.
     */
    bool setBatteryRatioHigh(uint8_t ratio);

    /**
     * Retrieves the stored callsign.
     * After successful NVS initialization, a valid destination is always
     * null-terminated. If the callsign does not
     * @param buffer Destination buffer.
     * @param size Total destination capacity, including the null terminator.
     * @return true if the complete callsign was copied, false otherwise.
     */
    bool getCallsign(char* buffer, size_t size);

    /**
     * Stores a callsign.
     * @param callsign Null-terminated callsign.
     * @return true if the value was stored, false otherwise.
     */
    bool setCallsign(const char* callsign);

    /**
     * Requests removal of the stored callsign so that the default is used.
     * @return true if the NVS namespace is available, false otherwise.
     */
    bool resetCallsign();

    /**
     * Returns the stored callsign suffix identifier.
     * @return Stored identifier, or the default value if unavailable.
     */
    uint8_t getCallsignSuffix();

    /**
     * Stores a callsign suffix identifier.
     * @param suffix Suffix identifier.
     * @return true if the value was stored, false otherwise.
     */
    bool setCallsignSuffix(uint8_t suffix);

    /**
     * Returns the stored TFT rotation identifier.
     * @return Stored identifier, or the default value if unavailable.
     */
    uint8_t getTFTRotation();

    /**
     * Stores a TFT rotation identifier.
     * @param rotation Rotation identifier.
     * @return true if the value was stored, false otherwise.
     */
    bool setTFTRotation(uint8_t rotation);

    /**
     * Returns the stored unit-system identifier.
     * @return Stored identifier, or the default value if unavailable.
     */
    uint8_t getUnits();

    /**
     * Stores a unit-system identifier.
     * @param unit Unit-system identifier.
     * @return true if the value was stored, false otherwise.
     */
    bool setUnits(uint8_t unit);

    /**
     * Returns the stored coordinate-format identifier.
     * @return Stored identifier, or the default value if unavailable.
     */
    uint8_t getCoordinateFormat();

    /**
     * Stores a coordinate-format identifier.
     * @param format Coordinate-format identifier.
     * @return true if the value was stored, false otherwise.
     */
    bool setCoordinateFormat(uint8_t format);

    /**
     * Retrieves the stored Wi-Fi SSID.
     * After successful NVS initialization, a valid destination is always null-terminated. If the SSID does not fit,
     * @param buffer Destination buffer.
     * @param size Total destination capacity, including the null terminator.
     * @return true if the complete SSID was copied, false otherwise.
     */
    bool getWifiSSID(char* buffer, size_t size);

    /**
     * Stores a Wi-Fi SSID.
     * @param ssid Null-terminated SSID.
     * @return true if the value was stored, false otherwise.
     */
    bool setWifiSSID(const char* ssid);

    /**
     * Requests removal of the stored Wi-Fi SSID.
     * @return true if the NVS namespace is available, false otherwise.
     */
    bool resetWifiSSID();

    /**
     * Retrieves the stored Wi-Fi password.
     * After successful NVS initialization, a valid destination is always null-terminated. If the password does not
     * @param buffer Destination buffer.
     * @param size Total destination capacity, including the null terminator.
     * @return true if the complete password was copied, false otherwise.
     */
    bool getWifiPassword(char* buffer, size_t size);

    /**
     * Stores a Wi-Fi password.
     * An empty value requests removal of the stored password.
     * @param password Null-terminated password.
     * @return true if the value was stored or removed, false otherwise.
     */
    bool setWifiPassword(const char* password);

    /**
     * Requests removal of the stored Wi-Fi password.
     * @return true if the NVS namespace is available, false otherwise.
     */
    bool resetWifiPassword();

    /**
     * Returns the stored Wi-Fi boot-mode identifier.
     * @return Stored identifier, or the default value if unavailable.
     */
    uint8_t getWifiBootMode();

    /**
     * Stores a Wi-Fi boot-mode identifier.
     * @param mode Boot-mode identifier.
     * @return true if the value was stored, false otherwise.
     */
    bool setWifiBootMode(uint8_t mode);

    /**
     * Returns the last stored Wi-Fi enabled state.
     * @return Stored state, or the default value if unavailable.
     */
    bool getWifiLastEnabled();

    /**
     * Stores the current Wi-Fi enabled state.
     * @param enabled Whether Wi-Fi is enabled.
     * @return true if the value was stored, false otherwise.
     */
    bool setWifiLastEnabled(bool enabled);
}
