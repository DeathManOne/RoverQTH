/*
 * include/services/power.h
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

namespace services::power {
    enum class ShutdownReason : uint8_t {BUTTON, BATTERY_CRITICAL};

    /**
     * Initializes the power management service.
     * @param buttonPin GPIO connected to the power button.
     * @return true if the service was successfully initialized, false otherwise.
     */
    bool begin(uint8_t buttonPin);

    /**
     * Updates the power management service.
     */
    void update();

    /**
     * Shuts down the device.
     * @param reason Reason for shutting down the device.
     */
    void shutdown(ShutdownReason reason);
}
