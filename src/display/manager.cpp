/*
 * display/manager.cpp
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

#include "display/internal.h"
#include "display/manager.h"
#include "ui/settings/themes/defaults.h"

namespace display::internal {
    SPIClass TFT_SPI(FSPI);
    ST7796S::MSP4021* TFT = nullptr;
}

namespace display {
    namespace {
        ST7796S::MSP4021& tft() { return *display::internal::TFT; }

        uint8_t sanitizeRotation(uint8_t rotation) {
            rotation %= 4;
            if (rotation == 0) { rotation = 1; }
            else if (rotation == 2) { rotation = 3; }
            return rotation;
        }
    }

    void begin(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t touchCS, uint8_t screenCS, uint8_t screenDC, uint8_t screenRST, uint16_t width, uint16_t height, uint8_t rotation) {
        display::internal::TFT_SPI.begin(clk, miso, mosi);

        if (!display::internal::TFT) {
            display::internal::TFT = new ST7796S::MSP4021(
                display::internal::TFT_SPI,
                touchCS,
                screenCS,   screenDC,
                width,      height,
                screenRST
            );
        }

        rotation = sanitizeRotation(rotation);
        tft().setRotation(rotation);
    }

    bool TCalibrate() { return tft().TCalibrate(); }
    void TCalibrate(bool swapXY, bool invertX, bool invertY, float CXA, float CXB, float CXC, float CYA, float CYB, float CYC) {
        tft().TCalibrate(swapXY, invertX, invertY, CXA, CXB, CXC, CYA, CYB, CYC);
    }

    void TCalibrateInfo(bool &swapXY, bool &invertX, bool &invertY, float &CXA, float &CXB, float &CXC, float &CYA, float &CYB, float &CYC) {
        tft().TCalibrateInfo(swapXY, invertX, invertY, CXA, CXB, CXC, CYA, CYB, CYC);
    }

    bool TRead(int &x, int &y) { return tft().TRead(x, y); }
    void clearScreen() { tft().fillScreen(ui::settings::themes::defaults::BLACK); }
}