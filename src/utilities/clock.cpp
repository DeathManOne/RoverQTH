/*
 * src/utilities/clock.cpp
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
#include <cstdio>
#include <ctime>
#include "utilities/clock.h"

namespace uClock = utilities::clock;

namespace {
    bool _synced         = false;
    uint32_t _syncMillis = 0;
    uint32_t _syncEpoch  = 0;

    int64_t _daysFromCivil(int year, unsigned month, unsigned day) {
        year -= month <= 2;

        const int era            = (year >= 0 ? year : year - 399) / 400;
        const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
        const unsigned dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const unsigned dayOfEra  = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;

        return (static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(dayOfEra) - 719468);
    }

    bool _getUTC(uint32_t utcEpoch, tm &utc) {
        if (utcEpoch == 0)
            { return false; }
        const time_t rawTime = static_cast<time_t>(utcEpoch);
        return gmtime_r(&rawTime, &utc) != nullptr;
    }
}

bool uClock::isSynced() { return _synced; }

uint32_t uClock::toEpochUTC(int year, int month, int day, int hour, int minute, int second) {
    if (year   < 1970 ||
        month  < 1    || month > 12  ||
        day    < 1    || day   > 31  ||
        hour   < 0    || hour  > 23  ||
        minute < 0    || minute > 59 ||
        second < 0    || second > 60
    ) { return 0; }

    const int64_t days  = _daysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
    const int64_t epoch = days * 86400LL + hour * 3600LL + minute * 60LL + second;

    if (epoch <= 0 || epoch > UINT32_MAX)
        { return 0; }
    return static_cast<uint32_t>(epoch);
}

void uClock::sync(const uint32_t utcEpoch) {
    if (utcEpoch == 0)                { return; }
    if (_synced && utcEpoch <= now()) { return; }
    _syncEpoch  = utcEpoch;
    _syncMillis = millis();
    _synced     = true;
}

uint32_t uClock::now() {
    if (!_synced) { return 0; }
    return _syncEpoch + ((millis() - _syncMillis) / 1000);
}

void uClock::getDate(char* buffer, size_t size) {
    if (buffer == nullptr || size == 0) { return; }

    tm utc = {};
    if (!_getUTC(now(), utc)) {
        snprintf(buffer, size, "---- -- --");
        return;
    }

    snprintf(buffer, size, "%04d %02d %02d", utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday);
}

void uClock::getTime(char* buffer, size_t size, bool withSecond) {
    if (buffer == nullptr || size == 0) { return; }

    tm utc = {};
    if (!_getUTC(now(), utc)) {
        if (withSecond) { snprintf(buffer, size, "-- : -- : --"); }
        else { snprintf(buffer, size, "-- : --"); }
        return;
    }

    if (withSecond) { snprintf(buffer, size, "%02d : %02d : %02d", utc.tm_hour, utc.tm_min, utc.tm_sec); }
    else { snprintf(buffer, size, "%02d : %02d", utc.tm_hour, utc.tm_min); }
}

void uClock::reset() {
    _synced      = false;
    _syncMillis  = 0;
    _syncEpoch   = 0;
}
