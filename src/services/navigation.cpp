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

#include "services/navigation.h"
#include "utilities/clock.h"
#include "utilities/distance.h"
#include "utilities/locator.h"
#include "utilities/text.h"

namespace navigation = services::navigation;
namespace uClock     = utilities::clock;
namespace distance   = utilities::distance;
namespace locator    = utilities::locator;
namespace text       = utilities::text;

namespace {
    struct NavigationData {
        navigation::Coordinate current;
        navigation::MarkSnapshot mark;
        navigation::MarkState state;
        bool hasCurrent;
        bool currentFixValid;
    };

    navigation::Coordinate _current {};
    navigation::MarkSnapshot _markSnapshot {};

    bool _hasCurrent                 = false;
    bool _currentFixValid            = false;
    portMUX_TYPE _lock               = portMUX_INITIALIZER_UNLOCKED;
    navigation::MarkState _markState = navigation::MarkState::IDLE;

    void _copyNavigationData(NavigationData& data) {
        portENTER_CRITICAL(&_lock);
        data.current         = _current;
        data.mark            = _markSnapshot;
        data.state           = _markState;
        data.hasCurrent      = _hasCurrent;
        data.currentFixValid = _currentFixValid;
        portEXIT_CRITICAL(&_lock);
    }
}

void navigation::begin() {
    portENTER_CRITICAL(&_lock);
    _current         = {};
    _markSnapshot    = {};
    _markState       = MarkState::IDLE;
    _hasCurrent      = false;
    _currentFixValid = false;
    portEXIT_CRITICAL(&_lock);
}

void navigation::updateGPSFix(const Coordinate& coordinate, const bool fixValid) {
    portENTER_CRITICAL(&_lock);
    _currentFixValid = fixValid;

    if (!fixValid) {
        portEXIT_CRITICAL(&_lock);
        return;
    }

    _current    = coordinate;
    _hasCurrent = true;

    if (_markState == MarkState::RECORDING) {
        if (coordinate.altitude < _markSnapshot.minAltitude)
            { _markSnapshot.minAltitude = coordinate.altitude; }
        if (coordinate.altitude > _markSnapshot.maxAltitude)
            { _markSnapshot.maxAltitude = coordinate.altitude; }
    }
    portEXIT_CRITICAL(&_lock);
}

bool navigation::startMark() {
    if (!uClock::isSynced()) { return false; }

    const uint32_t utc       = uClock::now();
    const uint32_t startedAt = millis();

    portENTER_CRITICAL(&_lock);

    if (!_hasCurrent || !_currentFixValid || _markState != MarkState::IDLE) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _markSnapshot                 = {};
    _markSnapshot.start           = _current;
    _markSnapshot.startUTC        = utc;
    _markSnapshot.startedAtMillis = startedAt;
    _markSnapshot.minAltitude     = _current.altitude;
    _markSnapshot.maxAltitude     = _current.altitude;
    _markState = MarkState::RECORDING;

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool navigation::stopMark() {
    if (!uClock::isSynced()) { return false; }

    const uint32_t utc       = uClock::now();
    const uint32_t stoppedAt = millis();

    portENTER_CRITICAL(&_lock);

    if (_markState != MarkState::RECORDING || !_hasCurrent || !_currentFixValid) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _markSnapshot.end             = _current;
    _markSnapshot.stopUTC         = utc;
    _markSnapshot.stoppedAtMillis = stoppedAt;
    _markSnapshot.hasEnd          = true;
    _markState                    = MarkState::READY_TO_SAVE;

    portEXIT_CRITICAL(&_lock);
    return true;
}

void navigation::clearMark() {
    portENTER_CRITICAL(&_lock);
    _markSnapshot = {};
    _markState    = MarkState::IDLE;
    portEXIT_CRITICAL(&_lock);
}

bool navigation::hasMark() {
    portENTER_CRITICAL(&_lock);
    const bool hasMark = _markState != MarkState::IDLE;
    portEXIT_CRITICAL(&_lock);
    return hasMark;
}

navigation::MarkState navigation::markState() {
    portENTER_CRITICAL(&_lock);
    const MarkState state = _markState;
    portEXIT_CRITICAL(&_lock);
    return state;
}

uint32_t navigation::markElapsedSeconds() {
    NavigationData data {};
    _copyNavigationData(data);
    if (data.state == MarkState::IDLE) { return 0; }

    if (data.state == MarkState::READY_TO_SAVE && data.mark.hasEnd)
        { return (data.mark.stoppedAtMillis - data.mark.startedAtMillis) / 1000U; }
    return (millis() - data.mark.startedAtMillis) / 1000U;
}

uint32_t navigation::markDurationSeconds() {
    MarkSnapshot snapshot {};

    if (!getMarkSnapshot(snapshot)) { return 0; }
    if (!snapshot.hasEnd)           { return 0; }
    return (snapshot.stoppedAtMillis - snapshot.startedAtMillis) / 1000U;
}

bool navigation::getMarkSnapshot(MarkSnapshot& snapshot) {
    portENTER_CRITICAL(&_lock);
    const bool available = _markState != MarkState::IDLE;
    snapshot             = _markSnapshot;
    portEXIT_CRITICAL(&_lock);
    return available;
}

double navigation::markTotalDistanceKm() {
    MarkSnapshot snapshot {};

    if (!getMarkSnapshot(snapshot) || !snapshot.hasEnd) { return -1.0; }
    return distance::betweenKilometers(
        snapshot.start.latitude, snapshot.start.longitude,
        snapshot.end.latitude,   snapshot.end.longitude
    );
}

double navigation::markCurrentDistanceKm() {
    NavigationData data {};
    _copyNavigationData(data);
    if (data.state == MarkState::IDLE) { return -1.0; }

    if (data.state == MarkState::READY_TO_SAVE) {
        if (!data.mark.hasEnd) { return -1.0; }
        const double distanceKm = distance::betweenKilometers(
            data.mark.start.latitude, data.mark.start.longitude,
            data.mark.end.latitude,   data.mark.end.longitude
        );
        return distanceKm > 0.0 ? distanceKm : -1.0;
    }

    if (!data.hasCurrent || !data.currentFixValid) { return -1.0; }
    return distance::betweenKilometers(
        data.current.latitude,    data.current.longitude,
        data.mark.start.latitude, data.mark.start.longitude
    );
}

double navigation::markCurrentBearingDeg() {
    NavigationData data {};
    _copyNavigationData(data);
    if (data.state == MarkState::IDLE) { return -1.0; }

    if (data.state == MarkState::READY_TO_SAVE) {
        if (!data.mark.hasEnd) { return -1.0; }
        const double distanceKm = distance::betweenKilometers(
            data.mark.start.latitude, data.mark.start.longitude,
            data.mark.end.latitude,   data.mark.end.longitude
        );

        if (distanceKm <= 0.0) { return -1.0; }
        return distance::bearingDegrees(
            data.mark.start.latitude, data.mark.start.longitude,
            data.mark.end.latitude,   data.mark.end.longitude
        );
    }

    if (!data.hasCurrent || !data.currentFixValid) { return -1.0; }
    return distance::bearingDegrees(
        data.current.latitude,    data.current.longitude,
        data.mark.start.latitude, data.mark.start.longitude
    );
}

void navigation::getMarkStartLocator(char* const buffer, const size_t size) {
    MarkSnapshot snapshot {};

    if (!getMarkSnapshot(snapshot)) {
        text::copy(buffer, size, "---");
        return;
    }
    if (!locator::fromCoordinates(snapshot.start.latitude, snapshot.start.longitude, buffer, size))
        { text::copy(buffer, size, "---"); }
}
