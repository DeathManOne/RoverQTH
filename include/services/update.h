/*
 * include/services/update.h
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

namespace services::update {
    constexpr size_t FIRMWARE_VERSION_SIZE = 16;
    constexpr size_t SOTA_VERSION_SIZE     = 16;
    constexpr size_t ERROR_SIZE            = 48;

    enum class Status : uint8_t {
        IDLE,
        NOT_INSTALLED,
        CHECKING,
        UP_TO_DATE,
        AVAILABLE,
        DOWNLOADING,
        VERIFYING,
        INSTALLING,
        SUCCESS,
        ERROR
    };

    struct FirmwareSnapshot {
        Status status    = Status::IDLE;
        uint8_t progress = 0;
        char latestVersion[FIRMWARE_VERSION_SIZE] {};
        char error[ERROR_SIZE] {};
    };

    struct SotaSnapshot {
        Status status    = Status::IDLE;
        uint8_t progress = 0;
        uint32_t records = 0;
        char installedVersion[SOTA_VERSION_SIZE] {};
        char latestVersion[SOTA_VERSION_SIZE] {};
        char error[ERROR_SIZE] {};
    };

    /**
     * @brief Initializes the update service and resets its runtime state.
     * @return true when the service state has been initialized.
     */
    bool begin();

    /**
     * Indicates whether an update operation is currently running.
     * @return true if an operation is in progress, false otherwise.
     */
    bool isBusy();

    /**
     * Starts checking for a new firmware version.
     * @return true if the asynchronous check was started, false if Wi-Fi is
     *         unavailable or another update task is already running.
     */
    bool checkFirmwareUpdate();

    bool checkSotaUpdate();

    /**
     * Starts the firmware update.
     * @return true if the asynchronous installation was started, false if Wi-Fi
     *         is unavailable, no update is available, or another task is running.
     */
    bool startFirmwareUpdate();

    bool startSotaUpdate();

    /**
     * Returns a snapshot of the current update state.
     * @return Current update snapshot.
     */
    FirmwareSnapshot firmwareSnapshot();

    SotaSnapshot sotaSnapshot();

    /**
     * Returns the current firmware version.
     * @return Null-terminated firmware version string.
     */
    const char* firmwareVersion();
}
