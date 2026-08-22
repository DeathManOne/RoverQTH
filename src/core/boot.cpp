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

#include <cstring>

#include "core/boot.h"
#include "services/display.h"
#include "services/gps.h"
#include "services/qth.h"
#include "services/settings.h"
#include "services/storage.h"
#include "services/wifi.h"

namespace boot     = core::boot;
namespace sDisplay = services::display;
namespace gps      = services::gps;
namespace qth      = services::qth;
namespace settings = services::settings;
namespace storage  = services::storage;
namespace wifi     = services::wifi;

namespace {
    bool _wifiOk = false;
    bool _sdOk   = false;
    bool _gpsOk  = false;

    void _initWifi();
    void _initSdCard(SPIClass &sdSPI, uint32_t timeout);
    void _initGPS           (HardwareSerial &gpsUART);
    void _waitGPSAcquisition(HardwareSerial &gpsUART);
    void _restoreQTH();

    void _initWifi() {
        if (!wifi::isInitialized()) {
            _wifiOk = false;
            sDisplay::updateBootWifi(&_wifiOk);
            return;
        }

        if (!settings::shouldConnectWifiAtBoot()) {
            _wifiOk = true;
            sDisplay::updateBootWifi(&_wifiOk);
            return;
        }

        settings::Wifi configuration = settings::wifi();
        if (configuration.ssid[0] == '\0') {
            std::memset(configuration.password, 0, sizeof(configuration.password));
            storage::appendErrorRecord("WIFI_SSID_LOAD_FAILED");

            _wifiOk = false;
            sDisplay::updateBootWifi(&_wifiOk);
            return;
        }

        const bool started = wifi::connect(configuration.ssid, configuration.password, 5);
        std::memset(configuration.password, 0, sizeof(configuration.password));

        if (!started) {
            storage::appendErrorRecord("WIFI_BOOT_CONNECT_FAILED");
            _wifiOk = false;
            sDisplay::updateBootWifi(&_wifiOk);
            return;
        }

        while (wifi::isConnecting()) {
            wifi::update();
            delay(20);
        }

        _wifiOk = wifi::isConnected();
        sDisplay::updateBootWifi(&_wifiOk);
    }

    void _initSdCard(SPIClass &sdSPI, uint32_t timeout) {
        storage::begin(sdSPI, timeout);
        _sdOk = storage::isReady();

        sDisplay::updateBootSD(&_sdOk);
        if (_sdOk) { sDisplay::setButtonState(sDisplay::Button::MARK_QTH, sDisplay::ButtonState::READY); }
    }

    void _initGPS(HardwareSerial &gpsUART) {
        gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
        _gpsOk = gps::isInitialized();

        sDisplay::updateBootGPS(&_gpsOk);
        if (_gpsOk) {
            _waitGPSAcquisition(gpsUART);
            return;
        }

        while (true) {
            int x, y;
            if (sDisplay::readTouch(x, y) &&
                sDisplay::handleBootTouch(x, y)
            ) {
                sDisplay::updateBootGPS(nullptr);
                gps::begin(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);
                _gpsOk = gps::isInitialized();

                sDisplay::updateBootGPS(&_gpsOk);
                if (_gpsOk) { break; }
            }
            delay(50);
        }
        _waitGPSAcquisition(gpsUART);
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
                progress       = currentProgress;
                lastProgressAt = millis();

                sDisplay::updateBootGPSProgress(progress);
                if (progress >= 100) {
                    storage::appendLogRecord("GPS_ACQUISITION_COMPLETE");
                    break;
                }
                continue;
            }

            if (progress == 0U) { continue; }
            if ((millis() - lastProgressAt) >= STALL_TIMEOUT_MS) {
                storage::appendErrorRecord("GPS_ACQUISITION_STALLED");
                sDisplay::updateBootGPS(nullptr);

                _gpsOk = gps::restart(gpsUART, GPS_RX, GPS_TX, GPS_BAUD, 10);

                sDisplay::updateBootGPS(&_gpsOk);
                if (!_gpsOk) { _initGPS(gpsUART); }

                progress       = 0;
                lastProgressAt = millis();
                sDisplay::updateBootGPSProgress(progress);
            }
        }
    }

    void _restoreQTH() {
        if (!_sdOk || !_gpsOk) { return; }

        const qth::RecoveryStatus status = qth::recoverTemporaryRecord();
        switch (status) {
            case qth::RecoveryStatus::NONE:
                return;
            case qth::RecoveryStatus::RECORDING_RESTORED:
                sDisplay::setButtonState(sDisplay::Button::MARK_QTH, sDisplay::ButtonState::RUNNING);
                storage::appendLogRecord("QTH_RECORDING_RESTORED");
                return;
            case qth::RecoveryStatus::RECORD_FINALIZED:
                storage::appendLogRecord("QTH_RECORD_FINALIZED");
                return;
            case qth::RecoveryStatus::ERROR:
            default:
                storage::appendErrorRecord("QTH_RECOVERY_FAILED");
                return;
        }
    }
}

bool boot::run(HardwareSerial &gpsUART, SPIClass &sdSPI) {
    _wifiOk = false;
    _sdOk   = false;
    _gpsOk  = false;    

    sDisplay::clearBoot();
    sDisplay::drawBootLogo();
    sDisplay::drawBoot();

    _initWifi();
    _initSdCard(sdSPI, 10);
    _initGPS(gpsUART);
    _restoreQTH();

    sDisplay::setButtonState(sDisplay::Button::MENU, sDisplay::ButtonState::READY);
    return _sdOk && _gpsOk;
}
