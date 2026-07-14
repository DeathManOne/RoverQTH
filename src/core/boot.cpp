/*
 * src/core/boot.cpp
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

namespace boot    = core::boot;
namespace state   = core::state;
namespace dBoot   = display::boot;
namespace gps     = services::gps;
namespace storage = services::storage;
namespace buttons = ui::widgets::buttons;

namespace {
    bool _wifiOk = false;
    bool _sdOk   = false;
    bool _gpsOk  = false;

    void _initSdCard(SPIClass &sdSPI, uint32_t timeout);
    void _initGPS           (HardwareSerial &gpsUART);
    void _waitGPSAcquisition(HardwareSerial &gpsUART);

    void _initSdCard(SPIClass &sdSPI, uint32_t timeout) {
        _sdOk = storage::begin(sdSPI, timeout);

        dBoot::updateSD(&_sdOk);
        if (!_sdOk) { return; }

        uint8_t type        = 0;
        uint64_t size       = 0;
        uint64_t totalBytes = 0;
        uint64_t usedBytes  = 0;

        _sdOk = storage::readCardInfos(type, size, totalBytes, usedBytes);
        if (!_sdOk) { storage::appendErrorRecord("SD_CARD_INFO_FAILED"); }
        else { storage::appendLogRecord("SD_READY"); }

        dBoot::updateSD(&_sdOk);
    }

    void _initGPS(HardwareSerial &gpsUART) {
        _gpsOk = gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
        dBoot::updateGPS(&_gpsOk);

        if (_gpsOk) {
            storage::appendLogRecord("GPS_READY");
            return;
        }
        storage::appendErrorRecord("GPS_INIT_FAILED");

        while (true) {
            int x, y;
            if (display::TRead(x, y)) {
                if (buttons::isPressed(buttons::bootSearchGPS, x, y)) {
                    dBoot::updateGPS(nullptr);
                    _gpsOk = gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);

                    dBoot::updateGPS(&_gpsOk);
                    if (_gpsOk) {
                        storage::appendLogRecord("GPS_READY");
                        return;
                    }
                }
            }
            delay(50);
        }
    }

    void _waitGPSAcquisition(HardwareSerial &gpsUART) {
        constexpr uint32_t CHECK_INTERVAL_MS = 500;
        constexpr uint32_t STALL_TIMEOUT_MS  = 30000;

        uint8_t progress        = 0;
        uint32_t lastProgressAt = millis();

        while (true) {
            delay(CHECK_INTERVAL_MS);
            const uint8_t currentProgress = gps::getAcquisitionProgress();
            if (currentProgress > progress) {
                progress = currentProgress;
                lastProgressAt = millis();

                dBoot::updateGPSProgress(progress);
                if (progress >= 100) {
                    storage::appendLogRecord("GPS_ACQUISITION_COMPLETE");
                    break;
                }
                continue;
            }

            if (progress == 0) { continue; }
            if ((millis() - lastProgressAt) >= STALL_TIMEOUT_MS) {
                storage::appendErrorRecord("GPS_ACQUISITION_STALLED");
                dBoot::updateGPS(nullptr);

                _gpsOk = gps::restart(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
                if (!_gpsOk) { storage::appendErrorRecord("GPS_RESTART_FAILED"); }
                else { storage::appendLogRecord("GPS_RESTARTED"); }

                dBoot::updateGPS(&_gpsOk);
                if (!_gpsOk) { _initGPS(gpsUART); }

                progress       = 0;
                lastProgressAt = millis();
                dBoot::updateGPSProgress(progress);
            }
        }
    }
}

bool boot::run(HardwareSerial &gpsUART, SPIClass &sdSPI) {
    _wifiOk = false;
    _sdOk   = false;
    _gpsOk  = false;

    dBoot::clear();
    dBoot::drawLogo();
    dBoot::draw();
    dBoot::updateWifi(&_wifiOk);

    _initSdCard(sdSPI, 10);
    if (_sdOk) {
        state::setButtonState(
            state::Button::MARK_QTH,
            state::ButtonState::READY
        );
    }
    dBoot::updateSD(&_sdOk);

    _initGPS(gpsUART);
    if (_gpsOk) { _waitGPSAcquisition(gpsUART); }

    state::setButtonState(
        state::Button::MENU,
        state::ButtonState::READY
    );
    return _sdOk && _gpsOk;
}
