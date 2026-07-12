/*
 * src/services/clock.cpp
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
#include "services/clock.h"

namespace sclock = services::clock;

namespace {
    bool _synced         = false;
    uint32_t _syncMillis = 0;
    uint32_t _syncEpoch  = 0;
}

bool sclock::isSynced() { return _synced; }

void sclock::sync(uint32_t utcEpoch) {
    _syncEpoch   = utcEpoch;
    _syncMillis  = millis();
    _synced      = true;
}

uint32_t sclock::now() {
    if (!_synced) { return 0; }
    return _syncEpoch + ((millis() - _syncMillis) / 1000);
}

void sclock::reset() {
    _synced      = false;
    _syncMillis  = 0;
    _syncEpoch   = 0;
}
