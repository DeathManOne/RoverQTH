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
    constexpr size_t VERSION_SIZE = 16;
    constexpr size_t ERROR_SIZE   = 48;

    enum class Status : uint8_t {
        IDLE,
        CHECKING,
        UP_TO_DATE,
        AVAILABLE,
        DOWNLOADING,
        VERIFYING,
        INSTALLING,
        SUCCESS,
        ERROR
    };

    struct Snapshot {
        Status status    = Status::IDLE;
        uint8_t progress = 0;

        char latestVersion[VERSION_SIZE] {};
        char error[ERROR_SIZE] {};
    };

    /**
     * Initializes the update service.
     * @return true if the service was successfully initialized, false otherwise.
     */
    bool begin();

    /**
     * Indicates whether an update operation is currently running.
     * @return true if an operation is in progress, false otherwise.
     */
    bool isBusy();

    /**
     * Starts checking for a new firmware version.
     * @return true if the check was successfully started, false otherwise.
     */
    bool checkUpdate();

    /**
     * Starts the firmware update.
     * @return true if the update was successfully started, false otherwise.
     */
    bool startUpdate();

    /**
     * Returns a snapshot of the current update state.
     * @return Current update snapshot.
     */
    Snapshot snapshot();

    /**
     * Returns the current firmware version.
     * @return Null-terminated firmware version string.
     */
    const char* currentVersion();
}
