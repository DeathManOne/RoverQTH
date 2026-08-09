/*
 * src/services/sota.cpp
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "database/sota.h"
#include "services/sota.h"
#include "services/update.h"
#include "utilities/distance.h"

namespace sota     = services::sota;
namespace sotaDB   = database::sota;
namespace update   = services::update;
namespace distance = utilities::distance;

namespace {
    portMUX_TYPE _lock   = portMUX_INITIALIZER_UNLOCKED;
    sota::Status _status = sota::Status::UNAVAILABLE;

    constexpr double REFRESH_DISTANCE_KM = 1.0;
    constexpr uint32_t TASK_STACK_SIZE   = 8192U;

    utilities::sota::Summit _summit {};
    double _distanceKm         = 0.0;
    double _bearingDeg         = 0.0;
    double _requestedLatitude  = 0.0;
    double _requestedLongitude = 0.0;
    double _searchedLatitude   = 0.0;
    double _searchedLongitude  = 0.0;
    bool _hasSearchedPosition  = false;
    bool _taskRunning          = false;

    bool _validPosition(double latitude, double longitude);
    void _searchTask(void*);

    bool _validPosition(const double latitude, const double longitude) {
        return std::isfinite(latitude) && std::isfinite(longitude) &&
            latitude >= -90.0          && latitude <= 90.0 &&
            longitude >= -180.0        && longitude <= 180.0;
    }

    void _searchTask(void*) {
        double latitude;
        double longitude;

        portENTER_CRITICAL(&_lock);
        latitude  = _requestedLatitude;
        longitude = _requestedLongitude;
        portEXIT_CRITICAL(&_lock);

        utilities::sota::Summit summit {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
        const bool found = sotaDB::findNearest(
            latitude,
            longitude,
            summit,
            distanceKm,
            bearingDeg
        );

        portENTER_CRITICAL(&_lock);
        if (found) {
            _summit              = summit;
            _distanceKm          = distanceKm;
            _bearingDeg          = bearingDeg;
            _searchedLatitude    = latitude;
            _searchedLongitude   = longitude;
            _hasSearchedPosition = true;
            _status              = sota::Status::READY;
        } else {
            _summit      = utilities::sota::Summit {};
            _distanceKm  = 0.0;
            _bearingDeg  = 0.0;
            _status      = sota::Status::ERROR;
        }
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);

        vTaskDelete(nullptr);
    }
}

bool sota::begin() {
    sotaDB::Info info {};
    const bool available  = sotaDB::info(info);

    portENTER_CRITICAL(&_lock);
    _status               = available ? Status::IDLE : Status::UNAVAILABLE;
    _summit               = utilities::sota::Summit {};
    _distanceKm           = 0.0;
    _bearingDeg           = 0.0;
    _hasSearchedPosition  = false;
    _taskRunning          = false;
    portEXIT_CRITICAL(&_lock);
    return true;
}

void sota::invalidate() {
    portENTER_CRITICAL(&_lock);
    _status              = Status::IDLE;
    _summit              = utilities::sota::Summit {};
    _distanceKm          = 0.0;
    _bearingDeg          = 0.0;
    _hasSearchedPosition = false;
    portEXIT_CRITICAL(&_lock);
}

bool sota::requestNearest(const double latitude, const double longitude) {
    if (!_validPosition(latitude, longitude) || update::isBusy()) { return false; }

    bool hasSearchedPosition;
    Status status;
    double searchedLatitude;
    double searchedLongitude;

    portENTER_CRITICAL(&_lock);
    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }
    hasSearchedPosition = _hasSearchedPosition;
    status              = _status;
    searchedLatitude    = _searchedLatitude;
    searchedLongitude   = _searchedLongitude;
    portEXIT_CRITICAL(&_lock);

    if (status == Status::UNAVAILABLE || status == Status::ERROR) { return false; }
    if (hasSearchedPosition && status == Status::READY) {
        const double movedKm = distance::betweenKilometers(
            searchedLatitude, searchedLongitude,
            latitude,         longitude
        );
        if (std::isfinite(movedKm) && movedKm < REFRESH_DISTANCE_KM)
            { return true; }
    }

    portENTER_CRITICAL(&_lock);
    if (_taskRunning || update::isBusy()) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }
    _requestedLatitude  = latitude;
    _requestedLongitude = longitude;
    _status             = Status::SEARCHING;
    _taskRunning        = true;
    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(_searchTask, "SOTA nearest", TASK_STACK_SIZE, nullptr, 1, nullptr);
    if (created == pdPASS) { return true; }

    portENTER_CRITICAL(&_lock);
    _taskRunning = false;
    _status      = Status::ERROR;
    portEXIT_CRITICAL(&_lock);
    return false;
}

sota::Snapshot sota::snapshot() {
    Snapshot value;
    portENTER_CRITICAL(&_lock);
    value.status     = _status;
    value.summit     = _summit;
    value.distanceKm = _distanceKm;
    value.bearingDeg = _bearingDeg;
    portEXIT_CRITICAL(&_lock);
    return value;
}

bool sota::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);
    return busy;
}
