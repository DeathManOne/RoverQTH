/*
 * include/services/wifi.h
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

namespace services::wifi {
    /**
     * Initializes the Wi-Fi service.
     * @return true if the service was successfully initialized, false otherwise.
     */
    bool begin();

    /**
     * Indicates whether the Wi-Fi service has been initialized.
     * @return true if the service is initialized, false otherwise.
     */
    bool isInitialized();

    /**
     * Indicates whether the device is connected to a Wi-Fi network.
     * @return true if the device is connected, false otherwise.
     */
    bool isConnected();

    /**
     * Indicates whether a Wi-Fi connection attempt is in progress.
     * @return true if a connection attempt is in progress, false otherwise.
     */
    bool isConnecting();

    /**
     * Connects to a Wi-Fi network.
     * @param ssid       Wi-Fi network SSID.
     * @param password   Wi-Fi network password.
     * @param timeoutSec Connection timeout in seconds.
     * @return true if the connection was successfully established, false otherwise.
     */
    bool connect(const char* ssid, const char* password, uint32_t timeoutSec = 15);

    /**
     * Updates the Wi-Fi service.
     */
    void update();

    /**
     * Disconnects from the current Wi-Fi network.
     */
    void disconnect();
}
