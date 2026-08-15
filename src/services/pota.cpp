/*
 * src/services/pota.cpp
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

#include "database/pota.h"
#include "services/pota.h"
#include "services/storage.h"
#include "services/update.h"
#include "utilities/distance.h"

namespace potaDB   = database::pota;
namespace pota     = services::pota;
namespace storage  = services::storage;
namespace update   = services::update;
namespace distance = utilities::distance;

namespace {
    portMUX_TYPE _lock   = portMUX_INITIALIZER_UNLOCKED;
    pota::Status _status = pota::Status::UNAVAILABLE;

    constexpr double REFRESH_DISTANCE_KM = 1.0;
    constexpr uint32_t TASK_STACK_SIZE   = 8192U;

    utilities::pota::Park _park {};
    double _distanceKm         = 0.0;
    double _bearingDeg         = 0.0;
    double _requestedLatitude  = 0.0;
    double _requestedLongitude = 0.0;
    double _searchedLatitude   = 0.0;
    double _searchedLongitude  = 0.0;
    bool _hasSearchedPosition  = false;
    bool _taskRunning          = false;

    bool _validPosition(const double latitude, const double longitude);
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

        utilities::pota::Park park {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
        const bool found = potaDB::findNearest(
            latitude,
            longitude,
            park,
            distanceKm,
            bearingDeg
        );

        if (!found) { storage::appendErrorRecord("POTA_SEARCH_FAILED"); }
        portENTER_CRITICAL(&_lock);

        if (found) {
            _park                = park;
            _distanceKm          = distanceKm;
            _bearingDeg          = bearingDeg;
            _searchedLatitude    = latitude;
            _searchedLongitude   = longitude;
            _hasSearchedPosition = true;
            _status              = pota::Status::READY;
        } else {
            _park       = utilities::pota::Park {};
            _distanceKm = 0.0;
            _bearingDeg = 0.0;
            _status     = pota::Status::ERROR;
        }
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);

        vTaskDelete(nullptr);
    }
}

bool pota::begin() {
    potaDB::Info info {};
    const bool available = potaDB::info(info);

    portENTER_CRITICAL(&_lock);
    _status              = available ? Status::IDLE : Status::UNAVAILABLE;
    _park                = utilities::pota::Park {};
    _distanceKm          = 0.0;
    _bearingDeg          = 0.0;
    _hasSearchedPosition = false;
    _taskRunning         = false;
    portEXIT_CRITICAL(&_lock);

    return true;
}

void pota::invalidate() {
    portENTER_CRITICAL(&_lock);
    _status              = Status::IDLE;
    _park                = utilities::pota::Park {};
    _distanceKm          = 0.0;
    _bearingDeg          = 0.0;
    _hasSearchedPosition = false;
    portEXIT_CRITICAL(&_lock);
}

bool pota::requestNearest(const double latitude, const double longitude) {
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

    if (status == Status::UNAVAILABLE) { return false; }
    if (hasSearchedPosition && status == Status::READY) {
        const double movedKm = distance::betweenKilometers(
            searchedLatitude, searchedLongitude,
            latitude,         longitude
        );
        if (std::isfinite(movedKm) && movedKm < REFRESH_DISTANCE_KM) { return true; }
    }

    portENTER_CRITICAL(&_lock);
    if (_taskRunning || update::isBusy()) {
        portEXIT_CRITICAL(&_lock);
        return false;
    }
    _requestedLatitude   = latitude;
    _requestedLongitude  = longitude;
    _status              = Status::SEARCHING;
    _park                = utilities::pota::Park {};
    _distanceKm          = 0.0;
    _bearingDeg          = 0.0;
    _hasSearchedPosition = false;
    _taskRunning         = true;
    portEXIT_CRITICAL(&_lock);

    const BaseType_t created = xTaskCreate(
        _searchTask,
        "POTA nearest",
        TASK_STACK_SIZE,
        nullptr,
        1,
        nullptr
    );
    if (created == pdPASS) { return true; }

    portENTER_CRITICAL(&_lock);
    _taskRunning = false;
    _status      = Status::ERROR;
    portEXIT_CRITICAL(&_lock);

    storage::appendErrorRecord("POTA_SEARCH_TASK_CREATE_FAILED");
    return false;
}

pota::Snapshot pota::snapshot() {
    Snapshot value;

    portENTER_CRITICAL(&_lock);
    value.status     = _status;
    value.park       = _park;
    value.distanceKm = _distanceKm;
    value.bearingDeg = _bearingDeg;
    portEXIT_CRITICAL(&_lock);

    return value;
}

bool pota::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);

    return busy;
}
