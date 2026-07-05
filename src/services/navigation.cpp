/*
 * services/navigation.cpp
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

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "services/gps.h"
#include "services/navigation.h"
#include "services/settings.h"

namespace services::navigation {
    namespace {
        //constexpr double PI = 3.14159265358979323846;
        constexpr double EARTH_RADIUS_KM = 6371.0;

        bool _hasCurrent = false;
        bool _hasMark = false;
        bool _hasSOTA = false;

        Coordinate _current {};
        Coordinate _mark {};
        SOTATarget _sota {};
        MarkState _markState = MarkState::IDLE;
        MarkSnapshot _markSnapshot {};

        double toRad(double deg) {
            return deg * PI / 180.0;
        }

        double toDeg(double rad) {
            return rad * 180.0 / PI;
        }

        void copyText(char* dest, size_t size, const char* src) {
            if (!dest || size == 0)
                { return; }
            std::snprintf(dest, size, "%s", src ? src : "");
        }
    }

    void begin() {
        _hasCurrent = false;
        _hasMark = false;
        _hasSOTA = false;
        _current = {};
        _mark = {};
        _sota = {};
        _markState = MarkState::IDLE;
        _markSnapshot = {};
    }

    bool startMark() {
        if (!_hasCurrent)
            { return false; }
        _markSnapshot.start = _current;
        _markSnapshot.end = {};
        _markSnapshot.startedAt = millis() / 1000;
        _markSnapshot.stoppedAt = 0;
        _markSnapshot.hasEnd = false;
        _mark = _current;
        _hasMark = true;
        _markState = MarkState::RECORDING;
        return true;
    }

    bool stopMark() {
        if (_markState != MarkState::RECORDING || !_hasCurrent)
            { return false; }
        _markSnapshot.end = _current;
        _markSnapshot.stoppedAt = millis() / 1000;
        _markSnapshot.hasEnd = true;
        _markState = MarkState::READY_TO_SAVE;
        return true;
    }

    bool isMarkRecording() {
        return _markState == MarkState::RECORDING;
    }

    MarkState markState() {
        return _markState;
    }

    uint32_t markElapsedSeconds() {
        if (_markState == MarkState::IDLE)
            { return 0; }
        if (_markState == MarkState::READY_TO_SAVE)
            { return markDurationSeconds(); }
        return (millis() / 1000) - _markSnapshot.startedAt;
    }

    uint32_t markDurationSeconds() {
        if (!_markSnapshot.hasEnd)
            { return 0; }
        return _markSnapshot.stoppedAt - _markSnapshot.startedAt;
    }

    Coordinate markStartPosition() {
        return _markSnapshot.start;
    }

    Coordinate markEndPosition() {
        return _markSnapshot.end;
    }

    double markTotalDistanceKm() {
        if (!_markSnapshot.hasEnd)
            { return -1.0; }
        return distanceKm(_markSnapshot.start, _markSnapshot.end);
    }

    double markCurrentDistanceKm() {
        if (_markState == MarkState::IDLE)
            { return -1.0; }
        if (_markState == MarkState::READY_TO_SAVE) {
            const double km = markTotalDistanceKm();
            return (km <= 0.0) ? -1.0 : km;
        }
        return distanceKm(_current, _markSnapshot.start);
    }

    double markCurrentBearingDeg() {
        if (_markState == MarkState::IDLE)
            { return -1.0; }
        if (_markState == MarkState::READY_TO_SAVE) {
            const double km = markTotalDistanceKm();
            if (km <= 0.0)
                { return -1.0; }
            return bearingDeg(_markSnapshot.start, _markSnapshot.end);
        }
        return bearingDeg(_current, _markSnapshot.start);
    }

    void getMarkStartLocator(char* buffer, size_t size) {
        if (_markState == MarkState::IDLE) {
            copyText(buffer, size, "---");
            return;
        }
        services::gps::decimalToGridLocator(_markSnapshot.start.latitude, _markSnapshot.start.longitude, buffer, size);
    }

    void getMarkEndLocator(char* buffer, size_t size) {
        if (!_markSnapshot.hasEnd) {
            copyText(buffer, size, "---");
            return;
        }
        services::gps::decimalToGridLocator(_markSnapshot.end.latitude, _markSnapshot.end.longitude, buffer, size);
    }

    void setCurrentPosition(double latitude, double longitude) {
        _current.latitude = latitude;
        _current.longitude = longitude;
        _hasCurrent = true;
    }

    bool hasCurrentPosition() {
        return _hasCurrent;
    }

    Coordinate currentPosition() {
        return _current;
    }

    double distanceKm(const Coordinate &from, const Coordinate &to) {
        const double lat1 = toRad(from.latitude);
        const double lat2 = toRad(to.latitude);
        const double dLat = toRad(to.latitude - from.latitude);
        const double dLon = toRad(to.longitude - from.longitude);

        const double a =
            std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
            std::cos(lat1) * std::cos(lat2) *
            std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return EARTH_RADIUS_KM * c;
    }

    double bearingDeg(const Coordinate &from, const Coordinate &to) {
        const double lat1 = toRad(from.latitude);
        const double lat2 = toRad(to.latitude);
        const double dLon = toRad(to.longitude - from.longitude);
        const double y = std::sin(dLon) * std::cos(lat2);
        const double x =
            std::cos(lat1) * std::sin(lat2) -
            std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
        double bearing = toDeg(std::atan2(y, x));

        if (bearing < 0.0)
            { bearing += 360.0; }
        return bearing;
    }

    void setMark(double latitude, double longitude) {
        _mark.latitude = latitude;
        _mark.longitude = longitude;
        _hasMark = true;
    }

    bool setMarkFromCurrentPosition() {
        if (!_hasCurrent)
            { return false; }
        _mark = _current;
        _hasMark = true;
        return true;
    }

    void clearMark() {
        _hasMark = false;
        _mark = {};
        _markSnapshot = {};
        _markState = MarkState::IDLE;
    }

    bool hasMark() {
        return _hasMark;
    }

    Coordinate markPosition() {
        return _mark;
    }

    double markDistanceKm() {
        if (!_hasCurrent || !_hasMark)
            { return -1.0; }
        return distanceKm(_current, _mark);
    }

    double markBearingDeg() {
        if (!_hasCurrent || !_hasMark)
            { return -1.0; }
        return bearingDeg(_current, _mark);
    }

    void formatDistance(double km, char* buffer, size_t size) {
        if (!buffer || size == 0) { return; }
        if (km < 0.0) {
            std::snprintf(buffer, size, "--");
            return;
        }

        if (services::settings::getUnits() == services::settings::Units::IMPERIAL) {
            const double miles = km * 0.621371;
            if (miles < 0.1) { std::snprintf(buffer, size, "%.0f ft", miles * 5280.0); }
            else { std::snprintf(buffer, size, "%.1f mi", miles); }
        } else {
            if (km < 1.0) { std::snprintf(buffer, size, "%.0f m", km * 1000.0); }
            else if (km < 10.0) { std::snprintf(buffer, size, "%.2f km", km); }
            else { std::snprintf(buffer, size, "%.1f km", km); }
        }
    }

    void formatBearing(double deg, char* buffer, size_t size) {
        if (!buffer || size == 0)
            { return; }
        if (deg < 0.0) {
            std::snprintf(buffer, size, "--");
            return;
        }
        std::snprintf(buffer, size, "%.0f°", deg);
    }

    void setSOTA(const char* code, double latitude, double longitude, int points, int altitude) {
        copyText(_sota.code, sizeof(_sota.code), code);
        _sota.coordinate.latitude = latitude;
        _sota.coordinate.longitude = longitude;
        _sota.points = points;
        _sota.altitude = altitude;
        _hasSOTA = true;
    }

    void clearSOTA() {
        _hasSOTA = false;
        _sota = {};
    }

    bool hasSOTA() {
        return _hasSOTA;
    }

    double sotaDistanceKm() {
        if (!_hasCurrent || !_hasSOTA)
            { return -1.0; }
        return distanceKm(_current, _sota.coordinate);
    }

    double sotaBearingDeg() {
        if (!_hasCurrent || !_hasSOTA)
            { return -1.0; }
        return bearingDeg(_current, _sota.coordinate);
    }

    void getSOTACode(char* buffer, size_t size) {
        copyText(buffer, size, _hasSOTA ? _sota.code : "--");
    }

    int getSOTAPoints() {
        return _hasSOTA ? _sota.points : 0;
    }

    int getSOTAAltitude() {
        return _hasSOTA ? _sota.altitude : 0;
    }
}