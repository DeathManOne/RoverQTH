/*
 * core/boot.cpp
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

#include "core/boot.h"
#include "core/state.h"
#include "display/boot.h"
#include "display/manager.h"
#include "services/gps.h"
#include "services/storage.h"
#include "ui/widgets/buttons.h"

namespace core::boot {
    namespace {
        bool wifiOk = false;
        bool sdOk   = false;
        bool gpsOk  = false;

        void initSdCard(SPIClass &sdSPI, uint32_t timeout);
        void initGPS(HardwareSerial &gpsUART);
        void waitGPSAcquisition(HardwareSerial &gpsUART);

        void initSdCard(SPIClass &sdSPI, uint32_t timeout) {
            sdOk = services::storage::begin(sdSPI, timeout);
            display::boot::updateSD(&sdOk);

            if (sdOk) {
                uint8_t type;
                uint64_t size, totalBytes, usedBytes;
                sdOk = services::storage::readCardInfos(type, size, totalBytes, usedBytes);
                display::boot::updateSD(&sdOk);
            }
        }

        void initGPS(HardwareSerial &gpsUART) {
            gpsOk = services::gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
            display::boot::updateGPS(&gpsOk);

            if (gpsOk)
                { return; }
            while (true) {
                int x, y;
                if (display::TRead(x, y)) {
                    if (ui::widgets::buttons::isPressed(ui::widgets::buttons::BOOT_SEARCH_GPS, x, y)) {
                        display::boot::updateGPS(nullptr);

                        gpsOk = services::gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
                        display::boot::updateGPS(&gpsOk);
                        if (gpsOk)
                            { return; }
                    }
                }
                delay(50);
            }
        }

        void waitGPSAcquisition(HardwareSerial &gpsUART) {
            constexpr uint32_t CHECK_INTERVAL_MS = 500;
            constexpr uint32_t STALL_TIMEOUT_MS  = 30000;

            uint8_t progress = 0;
            uint32_t lastProgressAt = millis();

            while (true) {
                delay(CHECK_INTERVAL_MS);
                const uint8_t currentProgress = services::gps::getAcquisitionProgress();
                if (currentProgress > progress) {
                    progress = currentProgress;
                    lastProgressAt = millis();

                    display::boot::updateGPSProgress(progress);
                    if (progress >= 100)
                        { break; }
                    continue;
                }
                if (progress == 0)
                    { continue; }
                if ((millis() - lastProgressAt) >= STALL_TIMEOUT_MS) {
                    display::boot::updateGPS(nullptr);

                    gpsOk = services::gps::restart(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
                    display::boot::updateGPS(&gpsOk);

                    if (!gpsOk)
                        { initGPS(gpsUART); }
                    progress = 0;
                    lastProgressAt = millis();
                    display::boot::updateGPSProgress(progress);
                }
            }
        }
    }

    bool run(HardwareSerial &gpsUART, SPIClass &sdSPI) {
        wifiOk = false;
        sdOk   = false;
        gpsOk  = false;

        display::boot::clear();
        display::boot::drawLogo();
        display::boot::draw();

        display::boot::updateWifi(&wifiOk);

        initSdCard(sdSPI, 10);
        if (sdOk) {
            core::state::setButtonState(
                core::state::Button::MARK_QTH,
                core::state::ButtonState::READY
            );
        }
        display::boot::updateSD(&sdOk);

        initGPS(gpsUART);
        if (gpsOk) { waitGPSAcquisition(gpsUART); }

        core::state::setButtonState(
            core::state::Button::MENU,
            core::state::ButtonState::READY
        );

        return sdOk && gpsOk;
    }
}