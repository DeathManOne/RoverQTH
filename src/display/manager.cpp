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
    }

    void begin(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t touchCS, uint8_t screenCS, uint8_t screenDC, uint8_t screenRST, uint16_t width, uint16_t height) {
        display::internal::TFT_SPI.begin(clk, miso, mosi);

        if (!display::internal::TFT) {
            display::internal::TFT = new ST7796S::MSP4021(
                display::internal::TFT_SPI,
                touchCS,    screenCS,   screenDC,
                width,      height,     screenRST
            );
        }
        tft().setRotation(static_cast<uint8_t>(services::settings::getTFTRotation()));

        if (TLoad())            { return; }
        while (!TCalibrate())   { delay(10); }
    }

    bool TLoad() {
        services::settings::Calibration calibration;
        if (!services::settings::getTouchCalibration(calibration))
            { return false; }
        TCalibrate(calibration);
        return true;
    }

    bool TCalibrate() {
        services::settings::Calibration normal;
        tft().setRotation(static_cast<uint8_t>(services::settings::TFTRotation::NORMAL));
        if (tft().TCalibrate()) { TCalibrateInfo(normal); }
        else { return false; }

        services::settings::Calibration reversed;
        tft().setRotation(static_cast<uint8_t>(services::settings::TFTRotation::REVERSED));
        if (tft().TCalibrate()) { TCalibrateInfo(reversed); }
        else { return false; }

        tft().setRotation(static_cast<uint8_t>(services::settings::getTFTRotation()));

        if(!services::settings::setTouchCalibration(normal, reversed))
            { return false; }
        return TLoad();
    }

    void TCalibrate(services::settings::Calibration &calibration) {
        tft().TCalibrate(
            calibration.swapXY,     calibration.invertX, calibration.invertY,
            calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
            calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
        );
    }

    void TCalibrateInfo(services::settings::Calibration &calibration) {
        tft().TCalibrateInfo(
            calibration.swapXY,     calibration.invertX, calibration.invertY,
            calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
            calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
        );
    }

    bool TRead(int &x, int &y)  { return tft().TRead(x, y); }
    void clearScreen()          { tft().fillScreen(ui::settings::themes::defaults::BLACK); }
}