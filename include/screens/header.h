/*
 * include/screens/header.h
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

#include <MSP4021.h>

namespace screens::header {
    /** @brief Copies the currently cached battery-level text. */
    void getBatteryLevel(char* buffer, size_t size);

    /** @brief Stores the callsign text for the next complete draw. */
    void setCallsign(const char* value);
    /** @brief Stores the date text for the next complete draw. */
    void setDate    (const char* value);
    /** @brief Stores the time text for the next complete draw. */
    void setTime    (const char* value);
    /** @brief Stores the battery text for the next complete draw. */
    void setBattery (const char* value);

    /** @brief Replaces and redraws the callsign field. */
    void updateCallsign(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the date field. */
    void updateDate    (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the time field. */
    void updateTime    (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the battery field. */
    void updateBattery (ST7796S::MSP4021 &tft, const char* value);

    /** @brief Draws the complete shared screen header. */
    void draw(ST7796S::MSP4021 &tft);
}
