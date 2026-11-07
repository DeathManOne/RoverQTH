/*
 * src/services/dtc.cpp
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
#include <sys/time.h>

#include "services/dtc.h"
#include "utilities/dtc.h"

namespace dtc  = services::dtc;
namespace uDtc = utilities::dtc;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    bool _synced         = false;
    uint32_t _syncMillis = 0;
    uint32_t _syncEpoch  = 0;

    uint32_t _currentEpoch(uint32_t syncEpoch, uint32_t syncMillis, uint32_t currentMillis);

    uint32_t _currentEpoch(const uint32_t syncEpoch, const uint32_t syncMillis, const uint32_t currentMillis) {
        const uint32_t elapsedSeconds = (currentMillis - syncMillis) / 1000U;
        const uint64_t currentEpoch   = static_cast<uint64_t>(syncEpoch) + static_cast<uint64_t>(elapsedSeconds);

        if (currentEpoch > UINT32_MAX)
            { return UINT32_MAX; }
        return static_cast<uint32_t>(currentEpoch);
    }
}


void dtc::sync(const uint32_t utcEpoch) {
    if (utcEpoch == 0U) { return; }

    portENTER_CRITICAL(&_lock);
    const uint32_t currentMillis = millis();

    if (_synced) {
        const uint32_t currentEpoch = _currentEpoch(_syncEpoch, _syncMillis, currentMillis);
        if (utcEpoch < currentEpoch) {
            portEXIT_CRITICAL(&_lock);
            return;
        }
    }

    _syncEpoch  = utcEpoch;
    _syncMillis = currentMillis;
    _synced     = true;
    portEXIT_CRITICAL(&_lock);

    timeval systemTime {};
    systemTime.tv_sec = static_cast<time_t>(utcEpoch);
    settimeofday(&systemTime, nullptr);
}

bool dtc::isSynced() {
    portENTER_CRITICAL(&_lock);
    const bool synced = _synced;
    portEXIT_CRITICAL(&_lock);
    return synced;
}

uint32_t dtc::now() {
    portENTER_CRITICAL(&_lock);
    const bool synced         = _synced;
    const uint32_t syncMillis = _syncMillis;
    const uint32_t syncEpoch  = _syncEpoch;
    portEXIT_CRITICAL(&_lock);

    if (!synced)
        { return 0U; }
    return _currentEpoch(syncEpoch, syncMillis, millis());
}

bool dtc::getDate(char* const buffer, const size_t size) {
    return uDtc::formatDate(now(), buffer, size);
}

bool dtc::getTime(char* const buffer, const size_t size, const bool withSecond) {
    return uDtc::formatTime(now(), buffer, size, withSecond);
}

void dtc::reset() {
    portENTER_CRITICAL(&_lock);
    _synced     = false;
    _syncMillis = 0;
    _syncEpoch  = 0;
    portEXIT_CRITICAL(&_lock);
}
