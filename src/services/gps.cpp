/*
 * services/gps.cpp
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

#include <cstdio>
#include <cstddef>
#include <cmath>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "services/gps.h"
#include "ui/settings/gps.h"

namespace services::gps {
    namespace {
        SFE_UBLOX_GNSS *_GPS = nullptr;
        int _SAT_FIX = 0;
        int _SAT_COUNT = 0;
        double _MASL = 0.0;
        double _HDG = 0.0;
        double _SPEED = 0.0;
        double _HDOP = 0.0;
        double _VDOP = 0.0;
        double _PDOP = 0.0;
        double _LATITUDE = 0.0;
        double _LONGITUDE = 0.0;
        int _DATE_YEAR = 0;
        int _DATE_MONTH = 0;
        int _DATE_DAY = 0;
        int _TIME_HOUR = 0;
        int _TIME_MINUTE = 0;
        int _TIME_SECOND = 0;

        void getCoordinates(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize, void (*formatter)(double, bool, char*, size_t)) {
            formatter(_LATITUDE, true, latitude, latitudeSize);
            formatter(_LONGITUDE, false, longitude, longitudeSize);
            decimalToGridLocator(_LATITUDE, _LONGITUDE, qth, qthSize);
        }

        bool readCache() {
            const bool fixOk = _GPS->getGnssFixOk();
            if (!fixOk) { return false; }

            _SAT_FIX = _GPS->getFixType();
            _SAT_COUNT = _GPS->getSIV();
            _MASL = _GPS->getAltitude() / 1000.0;
            _HDG = _GPS->getHeading() / 100000.0;
            _SPEED = (_GPS->getGroundSpeed() / 1000.0) * 3.6;
            _HDOP = _GPS->getHorizontalDOP() / 100.0;
            _VDOP = _GPS->getVerticalDOP() / 100.0;
            _PDOP = _GPS->getPDOP() / 100.0;
            _LATITUDE = _GPS->getLatitude() / 10000000.0;
            _LONGITUDE = _GPS->getLongitude() / 10000000.0;

            if (_GPS->getDateValid()) {
                _DATE_YEAR = _GPS->getYear();
                _DATE_MONTH = _GPS->getMonth();
                _DATE_DAY = _GPS->getDay();
            }

            if (_GPS->getTimeValid()) {
                _TIME_HOUR = _GPS->getHour();
                _TIME_MINUTE = _GPS->getMinute();
                _TIME_SECOND = _GPS->getSecond();
            }
            return true;
        }
    }

    char getHemisphere(const double value, const bool isLatitude) {
        if (isLatitude) { return (value >= 0) ? 'N' : 'S'; }
        return (value >= 0) ? 'E' : 'W';
    }

    bool begin(HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout) {
        if (!_GPS)
            { _GPS = new SFE_UBLOX_GNSS(); }
        static constexpr uint32_t BAUD_COUNT = 4;
        static const uint32_t bauds[BAUD_COUNT] = {9600, 38400, 57600, 115200};

        for (const uint32_t baud : bauds) {
            uart.begin(baud, SERIAL_8N1, rx, tx);
            while (uart.available())
                { uart.read(); }
            delay(100);

            const uint32_t start = millis();
            do {
                if (_GPS->begin(uart)) {
                    _GPS->setAutoPVT(true);
                    _GPS->setAutoDOPrate(1);
                    _GPS->setNavigationFrequency(5);
                    _GPS->setSerialRate(finalBaud);

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

    bool restart(HardwareSerial &uart, uint8_t rx, uint8_t tx, uint32_t finalBaud, uint32_t timeout) {
        uart.end();
        delay(500);

        if (_GPS) {
            delete _GPS;
            _GPS = nullptr;
        }

        _SAT_FIX = 0;
        _SAT_COUNT = 0;
        _HDOP = 0.0;
        _VDOP = 0.0;
        _PDOP = 0.0;

        return begin(uart, rx, tx, finalBaud, timeout);
    }

    bool update(uint32_t timeoutMs) {
        if (!_GPS)
            { return false; }
        const uint32_t start = millis();
        do {
            _GPS->checkUblox();
            if (readCache())
                { return true; }
            delay(1);
        } while ((millis() - start) < timeoutMs);
        return false;
    }

    bool poll() {
        if (!_GPS)
            { return false; }
        _GPS->checkUblox();
        return readCache();
    }

    void getDate(char* buffer, size_t size) {
        snprintf(buffer, size, "%04d %02d %02d", _DATE_YEAR, _DATE_MONTH, _DATE_DAY);
    }

    void getTime(char* buffer, size_t size, bool withSecond) {
        if (withSecond) { snprintf(buffer, size, "%02d : %02d : %02d", _TIME_HOUR, _TIME_MINUTE, _TIME_SECOND); }
        else { snprintf(buffer, size, "%02d : %02d", _TIME_HOUR, _TIME_MINUTE); }
    }

    uint8_t getAcquisitionProgress() {
        int fix, sat;
        double hdop, vdop, pdop;
        uint8_t progress = 0;

        if (!update())
            { return progress; }
        getSat(fix, sat);
        getDOP(hdop, vdop, pdop);

        if (sat >= 1)  { progress += 10; }
        if (sat >= 4)  { progress += 15; }
        if (sat >= 8)  { progress += 10; }

        if (fix >= 2)  { progress += 20; }
        if (fix >= 3)  { progress += 20; }

        const bool goodHDOP = hdop > 0 && hdop < ui::settings::gps::HDOP_FAIR;
        const bool excellentHDOP = hdop > 0 && hdop < ui::settings::gps::HDOP_GOOD;
        const bool goodPDOP = pdop > 0 && pdop < 3.0;

        if (goodHDOP)       { progress += 10; }
        if (excellentHDOP)  { progress += 10; }
        if (goodPDOP)       { progress += 5; }

        if (fix >= 3 && sat >= ui::settings::gps::SAT_MIN_FIX && goodHDOP)
            { progress = 100; }
        return (progress >= 100) ? 100 : progress;
    }

    const char* headingToCardinal(const double heading) {
        if (isnan(heading))
            { return ""; }
        static constexpr const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

        const double normalized = fmod((heading + 22.5), 360.0);
        const int index = static_cast<int>(normalized / 45.0) % 8;
        return directions[index];
    }

    void getSat(int &fix, int &count) {
        fix = _SAT_FIX;
        count = _SAT_COUNT;
    }

    void getPrecision(double &MASL, double &HDG, double &speed) {
        MASL = _MASL;
        const bool moving = _SPEED >= ui::settings::gps::MIN_HEADING_SPEED_KMH;
        if (moving) {
            HDG = _HDG;
            speed = _SPEED;
        } else {
            HDG = NAN;
            speed = 0.0;
        }
    }

    void getDOP(double &HDOP, double &VDOP, double &PDOP) {
        HDOP = _HDOP;
        VDOP = _VDOP;
        PDOP = _PDOP;
    }

    void getPosition(double &latitude, double &longitude) {
        latitude = _LATITUDE;
        longitude = _LONGITUDE;
    }

    void getDD(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
        getCoordinates(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize, decimalToDD);
    }

    void getDMS(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
        getCoordinates(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize, decimalToDMS);
    }

    void getDDM(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
        getCoordinates(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize, decimalToDDM);
    }

    void decimalToDD(const double value, const bool isLatitude, char* buffer, size_t size) {
        const char hemisphere = getHemisphere(value, isLatitude);
        const double absValue = std::abs(value);

        if (isLatitude) { snprintf(buffer, size, "%09.6f° %c", absValue, hemisphere); }
        else { snprintf(buffer, size, "%010.6f° %c", absValue, hemisphere); }
    }

    void decimalToDDM(const double value, const bool isLatitude, char* buffer, size_t size) {
        const char hemisphere = getHemisphere(value, isLatitude);
        const double absValue = std::abs(value);

        const int degrees = static_cast<int>(absValue);
        const double minutes = (absValue - degrees) * 60.0;

        if (isLatitude) { snprintf(buffer, size, "%02d°%07.4f' %c", degrees, minutes, hemisphere); }
        else { snprintf(buffer, size, "%03d°%07.4f' %c", degrees, minutes, hemisphere); }
    }

    void decimalToDMS(const double value, const bool isLatitude, char* buffer, size_t size) {
        const char hemisphere = getHemisphere(value, isLatitude);
        const double absValue = std::abs(value);

        const int degrees = static_cast<int>(absValue);
        const double minutesFull = (absValue - degrees) * 60.0;
        const int minutes = static_cast<int>(minutesFull);
        const double seconds = (minutesFull - minutes) * 60.0;

        if (isLatitude) { snprintf(buffer, size, "%02d°%02d'%05.2f\" %c", degrees, minutes, seconds, hemisphere); }
        else { snprintf(buffer, size, "%03d°%02d'%05.2f\" %c", degrees, minutes, seconds, hemisphere); }
    }

    void decimalToGridLocator(const double latitude, const double longitude, char* buffer, size_t size) {
        if (!buffer || size == 0)
            { return; }
        buffer[0] = '\0';

        if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
            snprintf(buffer, size, "ERROR");
            return;
        }

        char grid[11];
        double lat = (latitude >= 90.0) ? 89.999999 : latitude;
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
}
