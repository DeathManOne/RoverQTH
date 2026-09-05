/*
 * include/screens/main/datas.h
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
#include <MSP4021.h>

namespace screens::main::datas {
    /** @brief Stores the latitude text for the next complete draw. */
    void setLatitude    (const char* value);
    /** @brief Stores the longitude text for the next complete draw. */
    void setLongitude   (const char* value);
    /** @brief Stores the speed text for the next complete draw. */
    void setSpeed       (const char* value);
    /** @brief Stores the heading text for the next complete draw. */
    void setHeading     (const char* value);
    /** @brief Stores the altitude-above-sea-level text for the next complete draw. */
    void setASL         (const char* value);
    /** @brief Stores the update-time text for the next complete draw. */
    void setUpdate      (const char* value);

    /** @brief Replaces and redraws the latitude field. */
    void updateLatitude (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the longitude field. */
    void updateLongitude(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the speed field. */
    void updateSpeed    (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the heading field. */
    void updateHeading  (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the altitude-above-sea-level field. */
    void updateASL      (ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the update-time field. */
    void updateUpdate   (ST7796S::MSP4021 &tft, const char* value);

    /** @brief Draws the complete GNSS data panel. */
    void draw(ST7796S::MSP4021 &tft);
}
