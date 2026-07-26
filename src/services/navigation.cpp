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

#include "services/gps.h"
#include "services/navigation.h"
#include "utilities/clock.h"
#include "utilities/distance.h"
#include "utilities/locator.h"
#include "utilities/text.h"

namespace gps        = services::gps;
namespace navigation = services::navigation;
namespace uClock     = utilities::clock;
namespace distance   = utilities::distance;
namespace locator    = utilities::locator;
namespace text       = utilities::text;

namespace {
    bool _hasCurrent      = false;
    bool _hasMark         = false;
    bool _hasSOTA         = false;
    bool _currentFixValid = false;

    navigation::Coordinate _current {};
    navigation::Coordinate _mark {};
    navigation::SOTATarget _sota {};
    navigation::MarkState _markState = navigation::MarkState::IDLE;
    navigation::MarkSnapshot _markSnapshot {};
}

void navigation::begin() {
    _hasCurrent         = false;
    _hasMark            = false;
    _hasSOTA            = false;
    _current            = {};
    _mark               = {};
    _sota               = {};
    _markSnapshot       = {};
    _markState          = MarkState::IDLE;
    _currentFixValid    = false;
}

bool navigation::startMark() {
    if (!_hasCurrent || !_currentFixValid || !uClock::isSynced())
        { return false; }
    _markSnapshot = {};
    _markSnapshot.start              = _current;
    _markSnapshot.end                = {};
    _markSnapshot.startUTC           = uClock::now();
    _markSnapshot.stopUTC            = 0;
    _markSnapshot.startedAtMillis    = millis();
    _markSnapshot.stoppedAtMillis    = 0;
    _markSnapshot.minAltitude        = _current.altitude;
    _markSnapshot.maxAltitude        = _current.altitude;
    _markSnapshot.hasEnd             = false;

    _mark      = _current;
    _hasMark   = true;
    _markState = MarkState::RECORDING;
    return true;
}

bool navigation::stopMark() {
    if (_markState != MarkState::RECORDING || !_hasCurrent || !_currentFixValid || !uClock::isSynced())
        { return false; }
    _markSnapshot.end             = _current;
    _markSnapshot.stopUTC         = uClock::now();
    _markSnapshot.stoppedAtMillis = millis();
    _markSnapshot.hasEnd          = true;

    _markState = MarkState::READY_TO_SAVE;
    return true;
}

bool navigation::isMarkRecording()            { return _markState == MarkState::RECORDING; }
navigation::MarkState navigation::markState() { return _markState; }

uint32_t navigation::markElapsedSeconds() {
    if (_markState == MarkState::IDLE)
        { return 0; }
    if (_markState == MarkState::READY_TO_SAVE)
        { return markDurationSeconds(); }
    return (millis() - _markSnapshot.startedAtMillis) / 1000;
}

uint32_t navigation::markDurationSeconds() {
    if (!_markSnapshot.hasEnd) { return 0; }
    return (_markSnapshot.stoppedAtMillis - _markSnapshot.startedAtMillis) / 1000;
}

navigation::Coordinate navigation::markStartPosition()     { return _markSnapshot.start; }
navigation::Coordinate navigation::markEndPosition()       { return _markSnapshot.end; }
const navigation::MarkSnapshot& navigation::markSnapshot() { return _markSnapshot; }

double navigation::markTotalDistanceKm() {
    if (!_markSnapshot.hasEnd)
        { return -1.0; }
    return distance::betweenKilometers(
        _markSnapshot.start.latitude, _markSnapshot.start.longitude,
        _markSnapshot.end.latitude,   _markSnapshot.end.longitude);
}

double navigation::markCurrentDistanceKm() {
    if (_markState == MarkState::IDLE)  { return -1.0; }
    if (_markState == MarkState::READY_TO_SAVE) {
        const double km = markTotalDistanceKm();
        return (km <= 0.0) ? -1.0 : km;
    }
    return distance::betweenKilometers(
        _current.latitude,            _current.longitude,
        _markSnapshot.start.latitude, _markSnapshot.start.longitude
    );
}

double navigation::markCurrentBearingDeg() {
    if (_markState == MarkState::IDLE) { return -1.0; }
    if (_markState == MarkState::READY_TO_SAVE) {
        const double km = markTotalDistanceKm();
        if (km <= 0.0) { return -1.0; }
        return distance::bearingDegrees(
            _markSnapshot.start.latitude, _markSnapshot.start.longitude,
            _markSnapshot.end.latitude,   _markSnapshot.end.longitude
        );
    }
    return distance::bearingDegrees(
        _current.latitude,            _current.longitude,
        _markSnapshot.start.latitude, _markSnapshot.start.longitude
    );
}

void navigation::getMarkStartLocator(char* buffer, size_t size) {
    if (_markState == MarkState::IDLE) {
        text::copy(buffer, size, "---");
        return;
    }
    if (!locator::fromCoordinates(_markSnapshot.start.latitude, _markSnapshot.start.longitude, buffer, size))
        { text::copy(buffer, size, "---"); }
}

void navigation::getMarkEndLocator(char* buffer, size_t size) {
    if (!_markSnapshot.hasEnd) {
        text::copy(buffer, size, "---");
        return;
    }
    if (!locator::fromCoordinates(_markSnapshot.end.latitude, _markSnapshot.end.longitude, buffer, size))
        { text::copy(buffer, size, "---"); }
}

void navigation::updateGPSFix(bool fixValid) {
    _currentFixValid = fixValid;
    if (!fixValid) { return; }

    double latitude;
    double longitude;
    gps::getPosition(latitude, longitude);

    _current    = {latitude, longitude, gps::getAltitude()};
    _hasCurrent = true;

    if (_markState != MarkState::RECORDING) { return; }
    if (_current.altitude < _markSnapshot.minAltitude)
        { _markSnapshot.minAltitude = _current.altitude; }
    if (_current.altitude > _markSnapshot.maxAltitude)
        { _markSnapshot.maxAltitude = _current.altitude; }
}

bool navigation::hasCurrentPosition()                { return _hasCurrent; }
navigation::Coordinate navigation::currentPosition() { return _current; }

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
    if (!_hasCurrent || !_hasMark)
        { return -1.0; }
    return distance::betweenKilometers(
        _current.latitude, _current.longitude,
        _mark.latitude,    _mark.longitude
    );
}

double navigation::markBearingDeg() {
    if (!_hasCurrent || !_hasMark)
        { return -1.0; }
    return distance::bearingDegrees(
        _current.latitude, _current.longitude,
        _mark.latitude,    _mark.longitude
    );
}

void navigation::setSOTA(const char* code, double latitude, double longitude, int points, int altitude) {
    text::copy(_sota.code, sizeof(_sota.code), code);
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
    if (!_hasCurrent || !_hasSOTA)
        { return -1.0; }
    return distance::betweenKilometers(
        _current.latitude,         _current.longitude,
        _sota.coordinate.latitude, _sota.coordinate.longitude
    );
}

double navigation::sotaBearingDeg() {
    if (!_hasCurrent || !_hasSOTA)
        { return -1.0; }
    return distance::bearingDegrees(
        _current.latitude,         _current.longitude,
        _sota.coordinate.latitude, _sota.coordinate.longitude
    );
}

void navigation::getSOTACode(char* buffer, size_t size) { text::copy(buffer, size, _hasSOTA ? _sota.code : "--"); }
int navigation::getSOTAPoints()                         { return _hasSOTA ? _sota.points   : 0; }
int navigation::getSOTAAltitude()                       { return _hasSOTA ? _sota.altitude : 0; }
