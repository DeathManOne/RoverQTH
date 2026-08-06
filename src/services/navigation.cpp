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

#include "services/dtc.h"
#include "services/navigation.h"
#include "utilities/distance.h"

namespace dtc        = services::dtc;
namespace navigation = services::navigation;
namespace distance   = utilities::distance;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr size_t TRACE_QUEUE_SIZE         = 8U;
    constexpr uint32_t TRACE_INTERVAL_SECONDS = 1U;

    struct NavigationData {
        navigation::Coordinate current;
        navigation::MarkSnapshot mark;
        navigation::MarkState state;
        bool hasCurrent;
        bool currentFixValid;
    };

    navigation::Coordinate _current {};
    navigation::MarkSnapshot _markSnapshot {};
    navigation::TracePoint _traceQueue[TRACE_QUEUE_SIZE] {};

    size_t _traceQueueRead           = 0U;
    size_t _traceQueueWrite          = 0U;
    size_t _traceQueueCount          = 0U;
    uint32_t _lastTraceUTC           = 0U;
    bool _hasCurrent                 = false;
    bool _currentFixValid            = false;
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

    void _clearTraceQueue() {
        _traceQueueRead  = 0U;
        _traceQueueWrite = 0U;
        _traceQueueCount = 0U;
    }

    bool _pushTracePoint(const navigation::TracePoint& point) {
        if (_traceQueueCount >= TRACE_QUEUE_SIZE) { return false; }

        _traceQueue[_traceQueueWrite] = point;
        _traceQueueWrite = (_traceQueueWrite + 1U) % TRACE_QUEUE_SIZE;

        ++_traceQueueCount;
        return true;
    }

    bool _isValidCoordinate(const navigation::Coordinate& coordinate) {
        return
            std::isfinite(coordinate.latitude)  &&
            std::isfinite(coordinate.longitude) &&
            std::isfinite(coordinate.altitude)  &&
            coordinate.latitude  >= -90.0       && coordinate.latitude  <= 90.0  &&
            coordinate.longitude >= -180.0      && coordinate.longitude <= 180.0;
    }
}

void navigation::begin() {
    portENTER_CRITICAL(&_lock);
    _current            = {};
    _markSnapshot       = {};
    _markState          = MarkState::IDLE;
    _hasCurrent         = false;
    _currentFixValid    = false;
    _clearTraceQueue();
    _lastTraceUTC       = 0U;
    portEXIT_CRITICAL(&_lock);
}

void navigation::updateGPSFix(const Coordinate& coordinate, const bool fixValid) {
    const bool coordinateValid = fixValid && _isValidCoordinate(coordinate);

    portENTER_CRITICAL(&_lock);
    _currentFixValid = coordinateValid;

    if (!coordinateValid) {
        portEXIT_CRITICAL(&_lock);
        return;
    }

    _current    = coordinate;
    _hasCurrent = true;

    if (_markState == MarkState::RECORDING) {
        const uint32_t utc = dtc::now();
        if (utc != 0U && (_lastTraceUTC == 0U || utc - _lastTraceUTC >= TRACE_INTERVAL_SECONDS)) {
            TracePoint point {};
            point.coordinate = coordinate;
            point.utc        = utc;
            if (_pushTracePoint(point))
                { _lastTraceUTC = utc; }
        }
    }
    portEXIT_CRITICAL(&_lock);
}

bool navigation::peekPendingTracePoint(TracePoint& point) {
    portENTER_CRITICAL(&_lock);

    if (_traceQueueCount == 0U) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    point = _traceQueue[_traceQueueRead];

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool navigation::discardPendingTracePoint() {
    portENTER_CRITICAL(&_lock);

    if (_traceQueueCount == 0U) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _traceQueueRead = (_traceQueueRead + 1U) % TRACE_QUEUE_SIZE;
    --_traceQueueCount;

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool navigation::startMark() {
    if (!dtc::isSynced()) { return false; }

    const uint32_t utc = dtc::now();
    if (utc == 0U) { return false; }

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
    _clearTraceQueue();

    TracePoint firstPoint {};
    firstPoint.coordinate = _current;
    firstPoint.utc        = utc;

    if (!_pushTracePoint(firstPoint)) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _lastTraceUTC = utc;
    _markState    = MarkState::RECORDING;

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool navigation::restoreMark(const Coordinate& start, const uint32_t startUTC) {
    if (!_isValidCoordinate(start))         { return false; }
    if (!dtc::isSynced() || startUTC == 0U) { return false; }

    const uint32_t currentUTC = dtc::now();
    if (currentUTC == 0U || currentUTC < startUTC) { return false; }

    const uint32_t elapsedSeconds = currentUTC - startUTC;
    if (elapsedSeconds > UINT32_MAX / 1000U) { return false; }

    const uint32_t restoredStartedAt = millis() - elapsedSeconds * 1000U;

    portENTER_CRITICAL(&_lock);

    if (_markState != MarkState::IDLE) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }

    _markSnapshot                 = {};
    _markSnapshot.start           = start;
    _markSnapshot.startUTC        = startUTC;
    _markSnapshot.startedAtMillis = restoredStartedAt;
    _clearTraceQueue();
    _lastTraceUTC                 = currentUTC;
    _markState                    = MarkState::RECORDING;

    portEXIT_CRITICAL(&_lock);
    return true;
}

bool navigation::stopMark() {
    if (!dtc::isSynced()) { return false; }

    const uint32_t utc = dtc::now();
    if (utc == 0U) { return false; }

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
    _markSnapshot      = {};
    _markState         = MarkState::IDLE;
    _clearTraceQueue();
    _lastTraceUTC      = 0U;
    portEXIT_CRITICAL(&_lock);
}

navigation::MarkState navigation::markState() {
    portENTER_CRITICAL(&_lock);
    const MarkState state = _markState;
    portEXIT_CRITICAL(&_lock);
    return state;
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

bool navigation::getMarkDisplaySnapshot(MarkDisplaySnapshot& snapshot) {
    NavigationData data {};

    _copyNavigationData(data);
    snapshot = {};

    if (data.state == MarkState::IDLE) { return false; }
    snapshot.state = data.state;
    snapshot.start = data.mark.start;

    if (data.state == MarkState::READY_TO_SAVE && data.mark.hasEnd) {
        snapshot.elapsedSeconds = (
            data.mark.stoppedAtMillis - data.mark.startedAtMillis
        ) / 1000U;

        const double distanceKm = distance::betweenKilometers(
            data.mark.end.latitude,   data.mark.end.longitude,
            data.mark.start.latitude, data.mark.start.longitude
        );

        snapshot.distanceKm = distanceKm;

        if (distanceKm > 0.0) {
            snapshot.bearingDeg = distance::bearingDegrees(
                data.mark.end.latitude,   data.mark.end.longitude,
                data.mark.start.latitude, data.mark.start.longitude
            );
        }
        return true;
    }

    snapshot.elapsedSeconds = (
        millis() - data.mark.startedAtMillis
    ) / 1000U;

    if (!data.hasCurrent || !data.currentFixValid) { return true; }
    const double distanceKm = distance::betweenKilometers(
        data.current.latitude,    data.current.longitude,
        data.mark.start.latitude, data.mark.start.longitude
    );

    snapshot.distanceKm = distanceKm;

    if (distanceKm > 0.0) {
        snapshot.bearingDeg = distance::bearingDegrees(
            data.current.latitude,    data.current.longitude,
            data.mark.start.latitude, data.mark.start.longitude
        );
    }

    return true;
}
