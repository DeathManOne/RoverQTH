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
    constexpr size_t FIRMWARE_VERSION_SIZE = 16; /**< Firmware version buffer size, including the null terminator. */
    constexpr size_t SOTA_VERSION_SIZE     = 16; /**< SOTA version buffer size, including the null terminator. */
    constexpr size_t ERROR_SIZE            = 48; /**< Update error message buffer size, including the null terminator. */

    /** @brief State of an update check or installation operation. */
    enum class Status : uint8_t {
        IDLE,          /**< No operation has been requested. */
        NOT_INSTALLED, /**< The corresponding database is not installed. */
        CHECKING,      /**< The remote version or metadata is being checked. */
        UP_TO_DATE,    /**< The installed component is current. */
        AVAILABLE,     /**< An update is available. */
        DOWNLOADING,   /**< Update data is being downloaded. */
        VERIFYING,     /**< Downloaded or installed data is being verified. */
        INSTALLING,    /**< The update is being installed or converted. */
        SUCCESS,       /**< The update completed successfully. */
        ERROR          /**< The operation failed. */
    };

    /** @brief Snapshot of the firmware update state. */
    struct FirmwareSnapshot {
        Status status    = Status::IDLE; /**< Current firmware update state. */
        uint8_t progress = 0;            /**< Operation progress from 0 to 100 percent. */
        char latestVersion[FIRMWARE_VERSION_SIZE] {}; /**< Latest version reported by the manifest. */
        char error[ERROR_SIZE] {}; /**< User-facing error message, or an empty string. */
    };

    /** @brief Snapshot of the SOTA database update state. */
    struct SotaSnapshot {
        Status status    = Status::IDLE; /**< Current SOTA update state. */
        uint8_t progress = 0;            /**< Operation progress from 0 to 100 percent. */
        uint32_t records = 0;            /**< Number of records in the installed database. */
        char installedVersion[SOTA_VERSION_SIZE] {}; /**< Installed SOTA list version. */
        char latestVersion[SOTA_VERSION_SIZE] {}; /**< Version read from the downloaded SOTA list. */
        char error[ERROR_SIZE] {}; /**< User-facing error message, or an empty string. */
    };

    /** @brief Snapshot of the POTA database update state. */
    struct PotaSnapshot {
        Status status    = Status::IDLE; /**< Current POTA update state. */
        uint8_t progress = 0;            /**< Operation progress from 0 to 100 percent. */
        uint32_t records = 0;            /**< Number of records in the installed database. */
        char error[ERROR_SIZE] {}; /**< User-facing error message, or an empty string. */
    };

    /**
     * @brief Initializes the update service and resets its runtime state.
     * @return true when the service state has been initialized, false if an
     *         update or SOTA/POTA search task is active.
     */
    bool begin();

    /**
     * @brief Indicates whether an update operation is currently running.
     * @return true if an operation is in progress, false otherwise.
     */
    bool isBusy();

    /**
     * @brief Starts checking for a new firmware version.
     * @return true if the asynchronous check was started, false if Wi-Fi is
     *         unavailable, or another update/search task is active.
     */
    bool checkFirmwareUpdate();

    /**
     * @brief Starts checking for a SOTA database update.
     * @return true if the asynchronous check was started, false if the SD card
     *         or Wi-Fi is unavailable, or another update/search task is active.
     */
    bool checkSotaUpdate();

    /**
     * @brief Starts checking for a POTA database update.
     * @return true if the asynchronous check was started, false if the SD card
     *         or Wi-Fi is unavailable, or another update/search task is active.
     */
    bool checkPotaUpdate();

    /**
     * @brief Starts the firmware update.
     * @return true if the asynchronous installation was started, false if Wi-Fi
     *         is unavailable, no update is available, or another update/search
     *         task is active.
     */
    bool startFirmwareUpdate();

    /**
     * @brief Starts downloading, converting, and installing the SOTA database.
     * @return true if the asynchronous installation was started, false if no
     *         update is available, a dependency is unavailable, or another
     *         update/search task is active.
     */
    bool startSotaUpdate();

    /**
     * @brief Starts downloading, converting, and installing the POTA database.
     * @return true if the asynchronous installation was started, false if no
     *         update is available, a dependency is unavailable, or another
     *         update/search task is active.
     */
    bool startPotaUpdate();

    /**
     * @brief Returns a snapshot of the current firmware update state.
     * @return Current firmware update snapshot.
     */
    FirmwareSnapshot firmwareSnapshot();

    /**
     * @brief Returns a consistent snapshot of the SOTA update state.
     * @return Current SOTA update snapshot.
     */
    SotaSnapshot sotaSnapshot();

    /**
     * @brief Returns a consistent snapshot of the POTA update state.
     * @return Current POTA update snapshot.
     */
    PotaSnapshot potaSnapshot();

    /**
     * @brief Returns the current firmware version.
     * @return Null-terminated firmware version string.
     */
    const char* firmwareVersion();
}
