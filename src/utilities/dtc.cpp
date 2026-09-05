/*
 * src/utilities/dtc.cpp
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

#include "utilities/dtc.h"

#include <cstdio>
#include <ctime>

namespace dtc = utilities::dtc;

namespace {
    bool _isLeapYear(const int year) {
        return
            year % 4 == 0 &&
            (year % 100 != 0 || year % 400 == 0);
    }

    uint8_t _daysInMonth(const int year, const int month) {
        static constexpr uint8_t DAYS[] = {
            31U, 28U, 31U, 30U, 31U, 30U,
            31U, 31U, 30U, 31U, 30U, 31U
        };

        if (month < 1 || month > 12) {
            return 0U;
        }

        if (month == 2 && _isLeapYear(year)) {
            return 29U;
        }

        return DAYS[month - 1];
    }

    int64_t _daysFromCivil(
        int year,
        const unsigned month,
        const unsigned day
    ) {
        year -= month <= 2U;

        const int era =
            (year >= 0 ? year : year - 399) / 400;

        const unsigned yearOfEra =
            static_cast<unsigned>(year - era * 400);

        const unsigned adjustedMonth =
            month > 2U
                ? month - 3U
                : month + 9U;

        const unsigned dayOfYear =
            (153U * adjustedMonth + 2U) / 5U + day - 1U;

        const unsigned dayOfEra =
            yearOfEra * 365U +
            yearOfEra / 4U -
            yearOfEra / 100U +
            dayOfYear;

        return
            static_cast<int64_t>(era) * 146097LL +
            static_cast<int64_t>(dayOfEra) -
            719468LL;
    }

    bool _getUTC(const uint32_t utcEpoch, tm& utc) {
        if (utcEpoch == 0U) {
            return false;
        }

        const time_t rawTime = static_cast<time_t>(utcEpoch);
        return gmtime_r(&rawTime, &utc) != nullptr;
    }
}

uint32_t dtc::toEpochUTC(
    const int year,
    const int month,
    const int day,
    const int hour,
    const int minute,
    const int second
) {
    if (
        year < 1970 ||
        month < 1 ||
        month > 12 ||
        hour < 0 ||
        hour > 23 ||
        minute < 0 ||
        minute > 59 ||
        second < 0 ||
        second > 59
    ) {
        return 0U;
    }

    const uint8_t daysInMonth = _daysInMonth(year, month);

    if (day < 1 || day > static_cast<int>(daysInMonth)) {
        return 0U;
    }

    const int64_t days = _daysFromCivil(
        year,
        static_cast<unsigned>(month),
        static_cast<unsigned>(day)
    );

    const int64_t epoch =
        days * 86400LL +
        hour * 3600LL +
        minute * 60LL +
        second;

    if (epoch <= 0LL || epoch > UINT32_MAX) {
        return 0U;
    }

    return static_cast<uint32_t>(epoch);
}

bool dtc::formatISO8601(
    const uint32_t utcEpoch,
    char* const buffer,
    const size_t size
) {
    if (buffer == nullptr || size == 0U) {
        return false;
    }

    buffer[0] = '\0';

    if (size < 21U) {
        return false;
    }

    tm utc {};

    if (!_getUTC(utcEpoch, utc)) {
        return false;
    }

    const int written = std::snprintf(
        buffer,
        size,
        "%04d-%02d-%02dT%02d:%02d:%02dZ",
        utc.tm_year + 1900,
        utc.tm_mon + 1,
        utc.tm_mday,
        utc.tm_hour,
        utc.tm_min,
        utc.tm_sec
    );

    if (written != 20) {
        buffer[0] = '\0';
        return false;
    }

    return true;
}

bool dtc::formatDate(
    const uint32_t utcEpoch,
    char* const buffer,
    const size_t size
) {
    if (buffer == nullptr || size == 0U) {
        return false;
    }

    buffer[0] = '\0';

    tm utc {};

    if (!_getUTC(utcEpoch, utc)) {
        const int written = std::snprintf(
            buffer,
            size,
            "---- -- --"
        );

        return
            written >= 0 &&
            static_cast<size_t>(written) < size;
    }

    const int written = std::snprintf(
        buffer,
        size,
        "%04d %02d %02d",
        utc.tm_year + 1900,
        utc.tm_mon + 1,
        utc.tm_mday
    );

    if (
        written < 0 ||
        static_cast<size_t>(written) >= size
    ) {
        buffer[0] = '\0';
        return false;
    }

    return true;
}

bool dtc::formatTime(
    const uint8_t hour,
    const uint8_t minute,
    const uint8_t second,
    const bool valid,
    char* const buffer,
    const size_t size,
    const bool withSecond
) {
    if (buffer == nullptr || size == 0U) {
        return false;
    }

    if (
        !valid ||
        hour > 23U ||
        minute > 59U ||
        second > 59U
    ) {
        const int written = std::snprintf(
            buffer,
            size,
            withSecond ? "-- : -- : --" : "-- : --"
        );

        return
            written >= 0 &&
            static_cast<size_t>(written) < size;
    }

    const int written = withSecond
        ? std::snprintf(
            buffer,
            size,
            "%02u : %02u : %02u",
            static_cast<unsigned>(hour),
            static_cast<unsigned>(minute),
            static_cast<unsigned>(second)
        )
        : std::snprintf(
            buffer,
            size,
            "%02u : %02u",
            static_cast<unsigned>(hour),
            static_cast<unsigned>(minute)
        );

    return
        written >= 0 &&
        static_cast<size_t>(written) < size;
}

bool dtc::formatTime(
    const uint32_t utcEpoch,
    char* const buffer,
    const size_t size,
    const bool withSecond
) {
    tm utc {};
    const bool valid = _getUTC(utcEpoch, utc);

    return formatTime(
        valid ? static_cast<uint8_t>(utc.tm_hour) : 0U,
        valid ? static_cast<uint8_t>(utc.tm_min) : 0U,
        valid ? static_cast<uint8_t>(utc.tm_sec) : 0U,
        valid,
        buffer,
        size,
        withSecond
    );
}
