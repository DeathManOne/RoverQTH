/*
 * src/services/gps.cpp
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

#include <new>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

#include "services/dtc.h"
#include "services/gps.h"
#include "services/storage.h"
#include "ui/settings/gps.h"

namespace dtc      = services::dtc;
namespace gps      = services::gps;
namespace storage  = services::storage;
namespace settings = ui::settings::gps;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
    enum class GPSInitResult : uint8_t {SUCCESS, ALLOCATION_FAILED, DEVICE_NOT_FOUND};

    SFE_UBLOX_GNSS *_gps = nullptr;
    int _dateYear        = 0;
    int _dateMonth       = 0;
    int _dateDay         = 0;
    bool _hasSnapshot    = false;
    bool _initialized    = false;
    gps::Snapshot _snapshot {};

    bool _readCache() {
        gps::Snapshot next {};
        next.fixValid   = _gps->getGnssFixOk();
        next.fixType    = static_cast<uint8_t>(_gps->getFixType());
        next.satellites = static_cast<uint8_t>(_gps->getSIV());
        next.hdop       = _gps->getHorizontalDOP() / 100.0;
        next.pdop       = _gps->getPDOP() / 100.0;

        if (next.fixValid) {
            next.altitude      = _gps->getAltitude()     / 1000.0;
            next.heading       = _gps->getHeading()      / 100000.0;
            next.speed         = (_gps->getGroundSpeed() / 1000.0) * 3.6;
            next.latitude      = _gps->getLatitude()     / 10000000.0;
            next.longitude     = _gps->getLongitude()    / 10000000.0;
            next.positionValid = true;
        }

        if (_gps->getDateValid()) {
            _dateYear  = _gps->getYear();
            _dateMonth = _gps->getMonth();
            _dateDay   = _gps->getDay();
        }

        if (_gps->getTimeValid()) {
            next.hour      = static_cast<uint8_t>(_gps->getHour());
            next.minute    = static_cast<uint8_t>(_gps->getMinute());
            next.second    = static_cast<uint8_t>(_gps->getSecond());
            next.timeValid = true;
        }

        if (_gps->getDateValid() && _gps->getTimeValid()) {
            const uint32_t utcEpoch = dtc::toEpochUTC(
                    _dateYear, _dateMonth,  _dateDay,
                    next.hour, next.minute, next.second
                );
            if (utcEpoch != 0) { dtc::sync(utcEpoch); }
        }

        portENTER_CRITICAL(&_lock);
        _snapshot    = next;
        _hasSnapshot = true;
        portEXIT_CRITICAL(&_lock);
        return next.fixValid;
    }

    void _resetCache() {
        portENTER_CRITICAL(&_lock);
        _snapshot    = {};
        _hasSnapshot = false;
        portEXIT_CRITICAL(&_lock);

        _dateYear    = 0;
        _dateMonth   = 0;
        _dateDay     = 0;
    }

    GPSInitResult _initializeGPS(HardwareSerial &uart, const uint8_t rx, const uint8_t tx, const uint32_t finalBaud, const uint32_t timeout) {
        if (!_gps) {
            _gps = new (std::nothrow) SFE_UBLOX_GNSS();
            if (!_gps) { return GPSInitResult::ALLOCATION_FAILED; }
        }

        static constexpr uint32_t BAUD_COUNT    = 4U;
        static const uint32_t bauds[BAUD_COUNT] = {
            9600U, 38400U, 57600U, 115200U
        };

        for (const uint32_t baud : bauds) {
            uart.begin(baud, SERIAL_8N1, rx, tx);
            while (uart.available())
                { uart.read(); }
            delay(100);

            const uint32_t start = millis();
            do {
                if (_gps->begin(uart)) {
                    _gps->setAutoPVT(true);
                    _gps->setAutoDOPrate(1);
                    _gps->setNavigationFrequency(settings::NAVIGATION_RATE_HZ);
                    _gps->setSerialRate(finalBaud);

                    delay(100);
                    uart.updateBaudRate(finalBaud);
                    delay(100);
                    return GPSInitResult::SUCCESS;
                }
                delay(250);
            } while ((millis() - start) < timeout * 1000U);
        }
        return GPSInitResult::DEVICE_NOT_FOUND;
    }
}

void gps::begin(HardwareSerial &uart, const uint8_t rx, const uint8_t tx, const uint32_t finalBaud, const uint32_t timeout) {
    const GPSInitResult result = _initializeGPS(uart, rx, tx, finalBaud, timeout);
    _initialized               = result == GPSInitResult::SUCCESS;

    if (_initialized) {
        storage::appendLogRecord("GPS_READY");
        return;
    }

    storage::appendErrorRecord(
        result == GPSInitResult::ALLOCATION_FAILED
            ? "GPS_ALLOCATION_FAILED"
            : "GPS_INIT_FAILED"
    );
}

bool gps::restart(HardwareSerial &uart, const uint8_t rx, const uint8_t tx, const uint32_t finalBaud, const uint32_t timeout) {
    _initialized = false;
    uart.end();
    delay(500);

    if (_gps) {
        delete _gps;
        _gps = nullptr;
    }

    _resetCache();

    const GPSInitResult result = _initializeGPS(uart, rx, tx, finalBaud, timeout);
    _initialized               = result == GPSInitResult::SUCCESS;

    if (_initialized) {
        storage::appendLogRecord("GPS_RESTARTED");
        return true;
    }

    storage::appendErrorRecord(
        result == GPSInitResult::ALLOCATION_FAILED
            ? "GPS_RESTART_ALLOCATION_FAILED"
            : "GPS_RESTART_FAILED"
    );
    return false;
}

bool gps::isInitialized() {
    return _initialized;
}

bool gps::update(uint32_t timeoutMs) {
    if (!_gps)
        { return false; }
    const uint32_t start = millis();
    do {
        _gps->checkUblox();
        if (_readCache())
            { return true; }
        delay(1);
    } while ((millis() - start) < timeoutMs);
    return false;
}

bool gps::getSnapshot(Snapshot& snapshot) {
    portENTER_CRITICAL(&_lock);
    const bool available = _hasSnapshot;
    snapshot             = _snapshot;
    portEXIT_CRITICAL(&_lock);

    return available;
}

uint8_t gps::getAcquisitionProgress() {
    update();

    Snapshot snapshot {};
    if (!getSnapshot(snapshot)) { return 0; }

    uint8_t progress = 0;
    if (snapshot.satellites >= 1U) { progress += 10U; }
    if (snapshot.satellites >= 4U) { progress += 15U; }
    if (snapshot.satellites >= 8U) { progress += 10U; }

    if (snapshot.fixType >= 2U) { progress += 20U; }
    if (snapshot.fixType >= 3U) { progress += 20U; }

    const bool goodHDOP      = snapshot.hdop > 0.0 && snapshot.hdop < settings::HDOP_FAIR;
    const bool excellentHDOP = snapshot.hdop > 0.0 && snapshot.hdop < settings::HDOP_GOOD;
    const bool goodPDOP      = snapshot.pdop > 0.0 && snapshot.pdop < 3.0;

    if (goodHDOP)      { progress += 10U; }
    if (excellentHDOP) { progress += 10U; }
    if (goodPDOP)      { progress += 5U; }

    if (snapshot.fixValid && snapshot.fixType >= 3U && snapshot.satellites >= settings::SAT_MIN_FIX && goodHDOP)
        { return 100U; }
    return progress > 100U ? 100U : progress;
}
