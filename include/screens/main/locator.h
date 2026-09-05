/*
 * include/screens/main/locator.h
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

namespace screens::main::locator {
    /** @brief Stores the current locator for the next complete draw. */
    void setLocator(const char* value);
    /** @brief Stores the lower status text for the next complete draw. */
    void setStatusBottom(const char* value);
    /** @brief Stores the selected OTA code. */
    void setOTACode(const char* value);
    /** @brief Stores the selected OTA bearing. */
    void setOTABearing(const char* value);
    /** @brief Stores the selected OTA distance. */
    void setOTADistance(const char* value);
    /** @brief Stores the selected OTA points. */
    void setOTAPoints(const char* value);
    /** @brief Stores the selected OTA altitude. */
    void setOTAAltitude(const char* value);
    /** @brief Stores the marked-QTH locator. */
    void setMarkLocator(const char* value);
    /** @brief Stores the marked-QTH bearing. */
    void setMarkBearing(const char* value);
    /** @brief Stores the marked-QTH distance. */
    void setMarkDistance(const char* value);
    /** @brief Stores the marked-QTH elapsed-time text. */
    void setMarkTimer(const char* value);

    /** @brief Replaces and redraws the current locator. */
    void updateLocator(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the lower status text. */
    void updateStatusBottom(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the selected OTA code. */
    void updateOTACode(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the selected OTA bearing. */
    void updateOTABearing(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the selected OTA distance. */
    void updateOTADistance(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the selected OTA points. */
    void updateOTAPoints(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the selected OTA altitude. */
    void updateOTAAltitude(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the marked-QTH locator. */
    void updateMarkLocator(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the marked-QTH bearing. */
    void updateMarkBearing(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the marked-QTH distance. */
    void updateMarkDistance(ST7796S::MSP4021 &tft, const char* value);
    /** @brief Replaces and redraws the marked-QTH elapsed time. */
    void updateMarkTimer(ST7796S::MSP4021 &tft, const char* value);

    /** @brief Draws the complete locator, OTA and marked-QTH panel. */
    void draw(ST7796S::MSP4021 &tft);
}
