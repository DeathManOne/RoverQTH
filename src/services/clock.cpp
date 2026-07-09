/*
 * services/clock.cpp
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

namespace services::clock {
    namespace {
        bool synced         = false;
        uint32_t syncMillis = 0;
        uint32_t syncEpoch  = 0;
    }

    void sync(uint32_t utcEpoch) {
        syncEpoch   = utcEpoch;
        syncMillis  = millis();
        synced      = true;
    }

    bool isSynced() {
        return synced;
    }

    uint32_t now() {
        if (!synced)
            { return 0; }
        return syncEpoch + ((millis() - syncMillis) / 1000);
    }

    void reset() {
        synced      = false;
        syncMillis  = 0;
        syncEpoch   = 0;
    }
}