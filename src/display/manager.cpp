/*
 * src/display/manager.cpp
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

#include <MSP4021.h>
#include "display/internal.h"
#include "display/manager.h"
#include "services/storage.h"
#include "ui/settings/themes/defaults.h"

namespace internal = display::internal;
namespace settings = services::settings;
namespace storage  = services::storage;
namespace theme    = ui::settings::themes::defaults;

SPIClass internal::TFT_SPI(FSPI);
ST7796S::MSP4021* internal::TFT = nullptr;

namespace {
    ST7796S::MSP4021& _tft() { return *internal::TFT; }
}

void display::begin(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t touchCS, uint8_t screenCS, uint8_t screenDC, uint8_t screenRST, uint16_t width, uint16_t height) {
    internal::TFT_SPI.begin(clk, miso, mosi);

    if (!internal::TFT) {
        internal::TFT = new ST7796S::MSP4021(
            internal::TFT_SPI,
            touchCS,    screenCS,   screenDC,
            width,      height,     screenRST
        );
    }
    _tft().setRotation(static_cast<uint8_t>(settings::getTFTRotation()));

    if (TLoad())          { return; }
    while (!TCalibrate()) { delay(10); }
}

bool display::TLoad() {
    settings::Calibration calibration;
    if (!settings::getTouchCalibration(calibration))
        { return false; }
    TCalibrate(calibration);
    return true;
}

bool display::TCalibrate() {
    settings::Calibration normal;
    _tft().setRotation(static_cast<uint8_t>(settings::TFTRotation::NORMAL));
    if (_tft().TCalibrate()) { TCalibrateInfo(normal); }
    else { return false; }

    settings::Calibration reversed;
    _tft().setRotation(static_cast<uint8_t>(settings::TFTRotation::REVERSED));
    if (_tft().TCalibrate()) { TCalibrateInfo(reversed); }
    else { return false; }

    _tft().setRotation(static_cast<uint8_t>(settings::getTFTRotation()));
    if(!settings::setTouchCalibration(normal, reversed)) { return false; }

    storage::appendLogRecord("TOUCH_CALIBRATION_SAVED");
    return TLoad();
}

void display::TCalibrate(settings::Calibration &calibration) {
    _tft().TCalibrate(
        calibration.swapXY,     calibration.invertX, calibration.invertY,
        calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
        calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
    );
}

void display::TCalibrateInfo(settings::Calibration &calibration) {
    _tft().TCalibrateInfo(
        calibration.swapXY,     calibration.invertX, calibration.invertY,
        calibration.coeffXA,    calibration.coeffXB, calibration.coeffXC,
        calibration.coeffYA,    calibration.coeffYB, calibration.coeffYC
    );
}

bool display::TRead(int &x, int &y) { return _tft().TRead(x, y); }
void display::clearScreen()         { _tft().fillScreen(theme::BLACK); }
