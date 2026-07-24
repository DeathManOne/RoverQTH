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

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

#include "services/gps.h"
#include "ui/settings/gps.h"
#include "utilities/clock.h"

namespace gps      = services::gps;
namespace settings = ui::settings::gps;
namespace uClock    = utilities::clock;

namespace {
    SFE_UBLOX_GNSS *_gps = nullptr;
    int _satFix          = 0;
    int _satCount        = 0;
    double _masl         = 0.0;
    double _hdg          = 0.0;
    double _speed        = 0.0;
    double _hdop         = 0.0;
    double _vdop         = 0.0;
    double _pdop         = 0.0;
    double _latitude     = 0.0;
    double _longitude    = 0.0;
    int _dateYear        = 0;
    int _dateMonth       = 0;
    int _dateDay         = 0;
    int _timeHour        = 0;
    int _timeMinute      = 0;
    int _timeSecond      = 0;
    bool _timeValid      = false;

    bool _readCache();
    void _getCoordinates(
        char* latitude,  size_t latitudeSize,
        char* longitude, size_t longitudeSize,
        char* qth,       size_t qthSize,
        void (*formatter)(double, bool, char*, size_t)
    );

    void _getCoordinates(
        char* latitude,  size_t latitudeSize,
        char* longitude, size_t longitudeSize,
        char* qth,       size_t qthSize,
        void (*formatter)(double, bool, char*, size_t)
    ) {
        formatter(_latitude,  true,  latitude,  latitudeSize);
        formatter(_longitude, false, longitude, longitudeSize);
        gps::decimalToGridLocator(_latitude, _longitude, qth, qthSize);
    }

    bool _readCache() {
        const bool fixOk = _gps->getGnssFixOk();
        if (!fixOk) { return false; }

        _satFix    = _gps->getFixType();
        _satCount  = _gps->getSIV();
        _masl      = _gps->getAltitude() / 1000.0;
        _hdg       = _gps->getHeading() / 100000.0;
        _speed     = (_gps->getGroundSpeed() / 1000.0) * 3.6;
        _hdop      = _gps->getHorizontalDOP() / 100.0;
        _vdop      = _gps->getVerticalDOP() / 100.0;
        _pdop      = _gps->getPDOP() / 100.0;
        _latitude  = _gps->getLatitude() / 10000000.0;
        _longitude = _gps->getLongitude() / 10000000.0;

        if (_gps->getDateValid()) {
            _dateYear  = _gps->getYear();
            _dateMonth = _gps->getMonth();
            _dateDay   = _gps->getDay();
        }

        if (_gps->getTimeValid()) {
            _timeHour   = _gps->getHour();
            _timeMinute = _gps->getMinute();
            _timeSecond = _gps->getSecond();
            _timeValid  = true;
        }

        if (_gps->getDateValid() && _gps->getTimeValid()) {
            const uint32_t utcEpoch = uClock::toEpochUTC(
                _dateYear, _dateMonth, _dateDay,
                _timeHour, _timeMinute, _timeSecond
            );
            if (utcEpoch != 0) { uClock::sync(utcEpoch); }
        }
        return true;
    }
}

char gps::getHemisphere(const double value, const bool isLatitude) {
    if (isLatitude)
        { return (value >= 0) ? 'N' : 'S'; }
    return (value >= 0) ? 'E' : 'W';
}

bool gps::begin(HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout) {
    if (!_gps) { _gps = new SFE_UBLOX_GNSS(); }

    static constexpr uint32_t BAUD_COUNT    = 4;
    static const uint32_t bauds[BAUD_COUNT] = {9600, 38400, 57600, 115200};

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
                return true;
            }
            delay(250);
        } while ((millis() - start) < timeout * 1000);
    }
    return false;
}

bool gps::restart(HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout) {
    uart.end();
    delay(500);

    if (_gps) {
        delete _gps;
        _gps = nullptr;
    }

    _satFix     = 0;
    _satCount   = 0;
    _hdop       = 0.0;
    _vdop       = 0.0;
    _pdop       = 0.0;
    _timeHour   = 0;
    _timeMinute = 0;
    _timeSecond = 0;
    _timeValid  = false;
    return begin(uart, rx, tx, finalBaud, timeout);
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

bool gps::poll() {
    if (!_gps)
        { return false; }
    _gps->checkUblox();
    return _readCache();
}

void gps::getTime(char* buffer, size_t size, bool withSecond) {
    if (buffer == nullptr || size == 0) { return; }
    if (!_timeValid) {
        snprintf(buffer, size, withSecond ? "-- : -- : --" : "-- : --");
        return;
    }

    if (withSecond) {
        snprintf(
            buffer, size,
            "%02d : %02d : %02d",
            _timeHour, _timeMinute, _timeSecond
        );
    } else {
        snprintf(
            buffer, size,
            "%02d : %02d",
            _timeHour, _timeMinute
        );
    }
}

uint8_t gps::getAcquisitionProgress() {
    int fix          = 0;
    int sat          = 0;
    double hdop      = 0.0;
    double vdop      = 0.0;
    double pdop      = 0.0;
    uint8_t progress = 0;
    if (!update()) { return progress; }

    getSat(fix, sat);
    getDOP(hdop, vdop, pdop);

    if (sat >= 1) { progress += 10; }
    if (sat >= 4) { progress += 15; }
    if (sat >= 8) { progress += 10; }

    if (fix >= 2) { progress += 20; }
    if (fix >= 3) { progress += 20; }

    const bool goodHDOP      = hdop > 0 && hdop < settings::HDOP_FAIR;
    const bool excellentHDOP = hdop > 0 && hdop < settings::HDOP_GOOD;
    const bool goodPDOP      = pdop > 0 && pdop < 3.0;

    if (goodHDOP)      { progress += 10; }
    if (excellentHDOP) { progress += 10; }
    if (goodPDOP)      { progress += 5; }

    if (fix >= 3 && sat >= settings::SAT_MIN_FIX && goodHDOP)
        { progress = 100; }
    return (progress >= 100) ? 100 : progress;
}

const char* gps::headingToCardinal(const double heading) {
    if (isnan(heading)) { return ""; }

    static constexpr const char* directions[] = {
        "N", "NE",
        "E", "SE",
        "S", "SW",
        "W", "NW"
    };

    const double normalized = fmod((heading + 22.5), 360.0);
    const int index         = static_cast<int>(normalized / 45.0) % 8;
    return directions[index];
}

double gps::getAltitude() { return _masl; }

void gps::getSat(int &fix, int &count) {
    fix   = _satFix;
    count = _satCount;
}

void gps::getPrecision(double &MASL, double &HDG, double &speed) {
    MASL              = _masl;
    const bool moving = _speed >= settings::MIN_HEADING_SPEED_KMH;

    if (moving) {
        HDG   = _hdg;
        speed = _speed;
    } else {
        HDG   = NAN;
        speed = 0.0;
    }
}

void gps::getDOP(double &HDOP, double &VDOP, double &PDOP) {
    HDOP = _hdop;
    VDOP = _vdop;
    PDOP = _pdop;
}

void gps::getPosition(double &latitude, double &longitude) {
    latitude  = _latitude;
    longitude = _longitude;
}

void gps::getDD(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
    _getCoordinates(
        latitude,  latitudeSize,
        longitude, longitudeSize,
        qth,       qthSize,
        decimalToDD
    );
}

void gps::getDMS(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
    _getCoordinates(
        latitude,  latitudeSize,
        longitude, longitudeSize,
        qth,       qthSize,
        decimalToDMS
    );
}

void gps::getDDM(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
    _getCoordinates(
        latitude,  latitudeSize,
        longitude, longitudeSize,
        qth,       qthSize,
        decimalToDDM
    );
}

void gps::decimalToDD(const double value, const bool isLatitude, char* buffer, size_t size) {
    const char hemisphere = getHemisphere(value, isLatitude);
    const double absValue = std::abs(value);

    if (isLatitude) {
        snprintf(
            buffer, size,
            "%09.6f° %c",
            absValue, hemisphere
        );
    } else {
        snprintf(
            buffer, size,
            "%010.6f° %c",
            absValue, hemisphere
        );
    }
}

void gps::decimalToDDM(const double value, const bool isLatitude, char* buffer, size_t size) {
    const char hemisphere = getHemisphere(value, isLatitude);
    const double absValue = std::abs(value);
    const int degrees     = static_cast<int>(absValue);
    const double minutes  = (absValue - degrees) * 60.0;

    if (isLatitude) {
        snprintf(
            buffer, size,
            "%02d°%07.4f' %c",
            degrees, minutes, hemisphere
        );
    } else {
        snprintf(
            buffer, size,
            "%03d°%07.4f' %c",
            degrees, minutes, hemisphere
        );
    }
}

void gps::decimalToDMS(const double value, const bool isLatitude, char* buffer, size_t size) {
    const char hemisphere    = getHemisphere(value, isLatitude);
    const double absValue    = std::abs(value);
    const int degrees        = static_cast<int>(absValue);
    const double minutesFull = (absValue - degrees) * 60.0;
    const int minutes        = static_cast<int>(minutesFull);
    const double seconds     = (minutesFull - minutes) * 60.0;

    if (isLatitude) {
        snprintf(
            buffer, size,
            "%02d°%02d'%05.2f\" %c",
            degrees, minutes, seconds, hemisphere
        );
    } else {
        snprintf(
            buffer, size,
            "%03d°%02d'%05.2f\" %c",
            degrees, minutes, seconds, hemisphere
        );
    }
}

void gps::decimalToGridLocator(const double latitude, const double longitude, char* buffer, size_t size) {
    if (!buffer || size == 0) { return; }
    buffer[0] = '\0';

    if (latitude < -90.0   || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0
    ) {
        snprintf(buffer, size, "ERROR");
        return;
    }

    char grid[11];
    double lat = (latitude >= 90.0)   ? 89.999999  : latitude;
    double lon = (longitude >= 180.0) ? 179.999999 : longitude;

    lon += 180.0;
    lat += 90.0;
    grid[0] = 'A' + static_cast<int>(lon / 20.0);
    grid[1] = 'A' + static_cast<int>(lat / 10.0);

    lon = fmod(lon, 20.0);
    lat = fmod(lat, 10.0);
    grid[2] = '0' + static_cast<int>(lon / 2.0);
    grid[3] = '0' + static_cast<int>(lat);

    lon = fmod(lon, 2.0);
    lat = fmod(lat, 1.0);
    grid[4] = 'A' + static_cast<int>(lon * 12.0);
    grid[5] = 'A' + static_cast<int>(lat * 24.0);

    lon = fmod(lon, 2.0 / 24.0);
    lat = fmod(lat, 1.0 / 24.0);
    grid[6] = '0' + static_cast<int>(lon * 120.0);
    grid[7] = '0' + static_cast<int>(lat * 240.0);

    lon = fmod(lon, 2.0 / 240.0);
    lat = fmod(lat, 1.0 / 240.0);
    grid[8] = 'A' + static_cast<int>(lon * 2880.0);
    grid[9] = 'A' + static_cast<int>(lat * 5760.0);
    grid[10] = '\0';

    snprintf(
        buffer, size,
        "%c%c%c%c %c%c %c%c %c%c",
        grid[0], grid[1], grid[2], grid[3],
        grid[4], grid[5],
        grid[6], grid[7],
        grid[8], grid[9]
    );
}
