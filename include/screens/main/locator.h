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
    /**
     * @brief Stores the current locator for the next complete draw.
     * @param value Locator text to store.
     */
    void setLocator(const char* value);

    /**
     * @brief Stores the lower status text for the next complete draw.
     * @param value Lower status text to store.
     */
    void setStatusBottom(const char* value);

    /**
     * @brief Stores the selected OTA code.
     * @param value OTA code text to store.
     */
    void setOTACode(const char* value);

    /**
     * @brief Stores the selected OTA bearing.
     * @param value OTA bearing text to store.
     */
    void setOTABearing(const char* value);

    /**
     * @brief Stores the selected OTA distance.
     * @param value OTA distance text to store.
     */
    void setOTADistance(const char* value);

    /**
     * @brief Stores the selected OTA points.
     * @param value OTA points text to store.
     */
    void setOTAPoints(const char* value);

    /**
     * @brief Stores the selected OTA altitude.
     * @param value OTA altitude text to store.
     */
    void setOTAAltitude(const char* value);

    /**
     * @brief Stores the marked-QTH locator.
     * @param value Marked-QTH locator text to store.
     */
    void setMarkLocator(const char* value);

    /**
     * @brief Stores the marked-QTH bearing.
     * @param value Marked-QTH bearing text to store.
     */
    void setMarkBearing(const char* value);

    /**
     * @brief Stores the marked-QTH distance.
     * @param value Marked-QTH distance text to store.
     */
    void setMarkDistance(const char* value);

    /**
     * @brief Stores the marked-QTH elapsed-time text.
     * @param value Marked-QTH elapsed-time text to store.
     */
    void setMarkTimer(const char* value);

    /**
     * @brief Replaces and redraws the current locator.
     * @param tft Display instance used to redraw the field.
     * @param value New locator text to display.
     */
    void updateLocator(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the lower status text.
     * @param tft Display instance used to redraw the field.
     * @param value New lower status text to display.
     */
    void updateStatusBottom(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the selected OTA code.
     * @param tft Display instance used to redraw the field.
     * @param value New OTA code text to display.
     */
    void updateOTACode(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the selected OTA bearing.
     * @param tft Display instance used to redraw the field.
     * @param value New OTA bearing text to display.
     */
    void updateOTABearing(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the selected OTA distance.
     * @param tft Display instance used to redraw the field.
     * @param value New OTA distance text to display.
     */
    void updateOTADistance(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the selected OTA points.
     * @param tft Display instance used to redraw the field.
     * @param value New OTA points text to display.
     */
    void updateOTAPoints(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the selected OTA altitude.
     * @param tft Display instance used to redraw the field.
     * @param value New OTA altitude text to display.
     */
    void updateOTAAltitude(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the marked-QTH locator.
     * @param tft Display instance used to redraw the field.
     * @param value New marked-QTH locator text to display.
     */
    void updateMarkLocator(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the marked-QTH bearing.
     * @param tft Display instance used to redraw the field.
     * @param value New marked-QTH bearing text to display.
     */
    void updateMarkBearing(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the marked-QTH distance.
     * @param tft Display instance used to redraw the field.
     * @param value New marked-QTH distance text to display.
     */
    void updateMarkDistance(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Replaces and redraws the marked-QTH elapsed time.
     * @param tft Display instance used to redraw the field.
     * @param value New marked-QTH elapsed-time text to display.
     */
    void updateMarkTimer(ST7796S::MSP4021 &tft, const char* value);

    /**
     * @brief Draws the complete locator, OTA and marked-QTH panel.
     * @param tft Display instance used to draw the panel.
     */
    void draw(ST7796S::MSP4021 &tft);
}
