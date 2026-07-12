/*
 * src/services/navigation.cpp
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

namespace gps        = services::gps;
namespace navigation = services::navigation;
namespace settings   = services::settings;

namespace {
    //constexpr double PI = 3.14159265358979323846;
    constexpr double EARTH_RADIUS_KM = 6371.0;

    bool _hasCurrent = false;
    bool _hasMark    = false;
    bool _hasSOTA    = false;

    double _toRad(double deg);
    double _toDeg(double rad);
    void _copyText(char* dest, size_t size, const char* src);

    navigation::Coordinate _current {};
    navigation::Coordinate _mark {};
    navigation::SOTATarget _sota {};
    navigation::MarkState _markState = navigation::MarkState::IDLE;
    navigation::MarkSnapshot _markSnapshot {};

    double _toRad(double deg) { return deg * PI / 180.0; }
    double _toDeg(double rad) { return rad * 180.0 / PI; }

    void _copyText(char* dest, size_t size, const char* src) {
        if (!dest || size == 0) { return; }
        std::snprintf(dest, size, "%s", src ? src : "");
    }
}

void navigation::begin() {
    _hasCurrent   = false;
    _hasMark      = false;
    _hasSOTA      = false;
    _current      = {};
    _mark         = {};
    _sota         = {};
    _markSnapshot = {};
    _markState    = MarkState::IDLE;
}

bool navigation::startMark() {
    if (!_hasCurrent) { return false; }
    _markSnapshot.start     = _current;
    _markSnapshot.end       = {};
    _markSnapshot.startedAt = millis() / 1000;
    _markSnapshot.stoppedAt = 0;
    _markSnapshot.hasEnd    = false;
    _mark                   = _current;
    _hasMark                = true;
    _markState              = MarkState::RECORDING;
    return true;
}

bool navigation::stopMark() {
    if (_markState != MarkState::RECORDING || !_hasCurrent)
        { return false; }
    _markSnapshot.end       = _current;
    _markSnapshot.stoppedAt = millis() / 1000;
    _markSnapshot.hasEnd    = true;
    _markState              = MarkState::READY_TO_SAVE;
    return true;
}

bool navigation::isMarkRecording()            { return _markState == MarkState::RECORDING; }
navigation::MarkState navigation::markState() { return _markState; }

uint32_t navigation::markElapsedSeconds() {
    if (_markState == MarkState::IDLE)
        { return 0; }
    if (_markState == MarkState::READY_TO_SAVE)
        { return markDurationSeconds(); }
    return (millis() / 1000) - _markSnapshot.startedAt;
}

uint32_t navigation::markDurationSeconds() {
    if (!_markSnapshot.hasEnd) { return 0; }
    return _markSnapshot.stoppedAt - _markSnapshot.startedAt;
}

navigation::Coordinate navigation::markStartPosition() { return _markSnapshot.start; }
navigation::Coordinate navigation::markEndPosition()   { return _markSnapshot.end; }

double navigation::markTotalDistanceKm() {
    if (!_markSnapshot.hasEnd) { return -1.0; }
    return distanceKm(_markSnapshot.start, _markSnapshot.end);
}

double navigation::markCurrentDistanceKm() {
    if (_markState == MarkState::IDLE)  { return -1.0; }
    if (_markState == MarkState::READY_TO_SAVE) {
        const double km = markTotalDistanceKm();
        return (km <= 0.0) ? -1.0 : km;
    }
    return distanceKm(_current, _markSnapshot.start);
}

double navigation::markCurrentBearingDeg() {
    if (_markState == MarkState::IDLE) { return -1.0; }
    if (_markState == MarkState::READY_TO_SAVE) {
        const double km = markTotalDistanceKm();
        if (km <= 0.0) { return -1.0; }
        return bearingDeg(_markSnapshot.start, _markSnapshot.end);
    }
    return bearingDeg(_current, _markSnapshot.start);
}

void navigation::getMarkStartLocator(char* buffer, size_t size) {
    if (_markState == MarkState::IDLE) {
        _copyText(buffer, size, "---");
        return;
    }
    gps::decimalToGridLocator(_markSnapshot.start.latitude, _markSnapshot.start.longitude, buffer, size);
}

void navigation::getMarkEndLocator(char* buffer, size_t size) {
    if (!_markSnapshot.hasEnd) {
        _copyText(buffer, size, "---");
        return;
    }
    gps::decimalToGridLocator(_markSnapshot.end.latitude, _markSnapshot.end.longitude, buffer, size);
}

void navigation::setCurrentPosition(double latitude, double longitude) {
    _current.latitude  = latitude;
    _current.longitude = longitude;
    _hasCurrent        = true;
}

bool navigation::hasCurrentPosition()                { return _hasCurrent; }
navigation::Coordinate navigation::currentPosition() { return _current; }

double navigation::distanceKm(const Coordinate &from, const Coordinate &to) {
    const double lat1 = _toRad(from.latitude);
    const double lat2 = _toRad(to.latitude);
    const double dLat = _toRad(to.latitude - from.latitude);
    const double dLon = _toRad(to.longitude - from.longitude);

    const double a =
        std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
        std::cos(lat1)       * std::cos(lat2)       *
        std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

double navigation::bearingDeg(const Coordinate &from, const Coordinate &to) {
    const double lat1 = _toRad(from.latitude);
    const double lat2 = _toRad(to.latitude);
    const double dLon = _toRad(to.longitude - from.longitude);

    const double y = std::sin(dLon) * std::cos(lat2);
    const double x =
        std::cos(lat1) * std::sin(lat2) -
        std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
    double bearing = _toDeg(std::atan2(y, x));

    if (bearing < 0.0) { bearing += 360.0; }
    return bearing;
}

void navigation::setMark(double latitude, double longitude) {
    _mark.latitude  = latitude;
    _mark.longitude = longitude;
    _hasMark        = true;
}

bool navigation::setMarkFromCurrentPosition() {
    if (!_hasCurrent) { return false; }
    _mark    = _current;
    _hasMark = true;
    return true;
}

void navigation::clearMark() {
    _hasMark      = false;
    _mark         = {};
    _markSnapshot = {};
    _markState    = MarkState::IDLE;
}

bool navigation::hasMark()                        { return _hasMark; }
navigation::Coordinate navigation::markPosition() { return _mark; }

double navigation::markDistanceKm() {
    if (!_hasCurrent || !_hasMark) { return -1.0; }
    return distanceKm(_current, _mark);
}

double navigation::markBearingDeg() {
    if (!_hasCurrent || !_hasMark) { return -1.0; }
    return bearingDeg(_current, _mark);
}

void navigation::formatDistance(double km, char* buffer, size_t size) {
    if (!buffer || size == 0) { return; }
    if (km < 0.0) {
        std::snprintf(buffer, size, "--");
        return;
    }

    if (settings::getUnits() == settings::Units::IMPERIAL) {
        const double miles = km * 0.621371;
        if (miles < 0.1) { std::snprintf(buffer, size, "%.0f ft", miles * 5280.0); }
        else             { std::snprintf(buffer, size, "%.1f mi", miles); }
    } else {
        if      (km < 1.0)  { std::snprintf(buffer, size, "%.0f m", km * 1000.0); }
        else if (km < 10.0) { std::snprintf(buffer, size, "%.2f km", km); }
        else                { std::snprintf(buffer, size, "%.1f km", km); }
    }
}

void navigation::formatBearing(double deg, char* buffer, size_t size) {
    if (!buffer || size == 0)  { return; }
    if (deg < 0.0) {
        std::snprintf(buffer, size, "--");
        return;
    }
    std::snprintf(buffer, size, "%.0f°", deg);
}

void navigation::setSOTA(const char* code, double latitude, double longitude, int points, int altitude) {
    _copyText(_sota.code, sizeof(_sota.code), code);
    _sota.coordinate.latitude  = latitude;
    _sota.coordinate.longitude = longitude;
    _sota.points               = points;
    _sota.altitude             = altitude;
    _hasSOTA                   = true;
}

void navigation::clearSOTA() {
    _hasSOTA = false;
    _sota    = {};
}

bool navigation::hasSOTA() { return _hasSOTA; }

double navigation::sotaDistanceKm() {
    if (!_hasCurrent || !_hasSOTA) { return -1.0; }
    return distanceKm(_current, _sota.coordinate);
}

double navigation::sotaBearingDeg() {
    if (!_hasCurrent || !_hasSOTA) { return -1.0; }
    return bearingDeg(_current, _sota.coordinate);
}

void navigation::getSOTACode(char* buffer, size_t size) { _copyText(buffer, size, _hasSOTA ? _sota.code : "--"); }
int navigation::getSOTAPoints()                         { return _hasSOTA ? _sota.points   : 0; }
int navigation::getSOTAAltitude()                       { return _hasSOTA ? _sota.altitude : 0; }
