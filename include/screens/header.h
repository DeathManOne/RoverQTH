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
    /**
     * @brief Copies the current battery-level text.
     * @param buffer Destination buffer receiving the formatted battery level.
     * @param size Capacity of the destination buffer in bytes.
     */
    void getBatteryLevel(char* buffer, size_t size);

    /**
     * @brief Stores the callsign text for the next complete draw.
     * @param value Callsign text to store.
     */
    void setCallsign(const char* value);

    /**
     * @brief Stores the date text for the next complete draw.
     * @param value Date text to store.
     */
    void setDate(const char* value);

    /**
     * @brief Stores the time text for the next complete draw.
     * @param value Time text to store.
     */
    void setTime(const char* value);

    /**
     * @brief Stores the battery text for the next complete draw.
     * @param value Battery text to store.
     */
    void setBattery(const char* value);

    /**
     * @brief Replaces and redraws the callsign field.
     * @param tft Display instance used to redraw the field.
     * @param value New callsign text to display.
     */
    void updateCallsign(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the date field.
     * @param tft Display instance used to redraw the field.
     * @param value New date text to display.
     */
    void updateDate(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the time field.
     * @param tft Display instance used to redraw the field.
     * @param value New time text to display.
     */
    void updateTime(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the battery field.
     * @param tft Display instance used to redraw the field.
     * @param value New battery text to display.
     */
    void updateBattery(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Draws the complete shared screen header.
     * @param tft Display instance used to draw the header.
     */
    void draw(ST7796S::MSP4021 &tft);
}
