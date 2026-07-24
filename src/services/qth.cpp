/*
 * src/services/qth.cpp
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
#include <cstdio>
#include <ctime>

#include "services/navigation.h"
#include "services/qth.h"
#include "services/storage.h"
#include "utilities/locator.h"
#include "utilities/units.h"

namespace navigation = services::navigation;
namespace qth        = services::qth;
namespace storage    = services::storage;
namespace locator    = utilities::locator;
namespace units      = utilities::units;

namespace {
    bool _formatUTC(uint32_t epoch, char* buffer, size_t size) {
        if (!buffer || size < 21 || epoch == 0) { return false; }

        const time_t rawTime = static_cast<time_t>(epoch);
        struct tm utcTime {};

        if (!gmtime_r(&rawTime, &utcTime)) { return false; }
        const int written = std::snprintf(
            buffer, size,
            "%04d-%02d-%02dT%02d:%02d:%02dZ",
            utcTime.tm_year + 1900, utcTime.tm_mon + 1, utcTime.tm_mday,
            utcTime.tm_hour, utcTime.tm_min, utcTime.tm_sec
        );
        return written == 20;
    }

    bool _formatDuration(uint32_t seconds, char* buffer, size_t size) {
        if (!buffer || size == 0) { return false; }

        const uint32_t hours    = seconds / 3600;
        const uint32_t minutes  = (seconds % 3600) / 60;
        const uint32_t rSeconds = seconds % 60;
        const int written = std::snprintf(
            buffer, size,
            "%02lu:%02lu:%02lu",
            static_cast<unsigned long>(hours),
            static_cast<unsigned long>(minutes),
            static_cast<unsigned long>(rSeconds)
        );
        return (written > 0 && static_cast<size_t>(written) < size);
    }
}

bool qth::isCurrentRecordLongEnough() {
    if (navigation::markState() != navigation::MarkState::READY_TO_SAVE)
        { return false; }
    return (navigation::markDurationSeconds() >= MIN_DURATION_SECONDS);
}

bool qth::buildCurrentRecord(QTHRecord& record) {
    if (navigation::markState() != navigation::MarkState::READY_TO_SAVE)
        { return false; }
    const navigation::MarkSnapshot& snapshot = navigation::markSnapshot();

    if (!snapshot.hasEnd) { return false; }
    record = {};
    record.start.time      = snapshot.startUTC;
    record.start.latitude  = snapshot.start.latitude;
    record.start.longitude = snapshot.start.longitude;
    record.start.altitude  = snapshot.start.altitude;

    if (!_formatUTC(record.start.time, record.start.utc, sizeof(record.start.utc)))
        { return false; }
    if (!locator::fromCoordinates(record.start.latitude, record.start.longitude, record.start.locator, sizeof(record.start.locator)))
        { return false; }
    record.stop.time      = snapshot.stopUTC;
    record.stop.latitude  = snapshot.end.latitude;
    record.stop.longitude = snapshot.end.longitude;
    record.stop.altitude  = snapshot.end.altitude;

    if (!_formatUTC(record.stop.time, record.stop.utc, sizeof(record.stop.utc)))
        { return false; }
    if (!locator::fromCoordinates(record.stop.latitude, record.stop.longitude, record.stop.locator, sizeof(record.stop.locator)))
        { return false; }
    record.duration.seconds = navigation::markDurationSeconds();

    if (!_formatDuration(record.duration.seconds, record.duration.hms, sizeof(record.duration.hms)))
        { return false; }
    record.distance.airLineKm      = navigation::markTotalDistanceKm();
    record.distance.airLineMeters  = static_cast<uint32_t>(std::lround(units::kilometersToMeters(record.distance.airLineKm)));
    record.minimumAltitude         = snapshot.minAltitude;
    record.maximumAltitude         = snapshot.maxAltitude;
    return true;
}

bool qth::serializeJSONL(const QTHRecord& record, char* buffer, size_t size) {
    if (!buffer || size == 0) { return false; }

    const int written = std::snprintf(
        buffer, size,
        "{"
            "\"start\":{"
                "\"time\":%lu,"
                "\"utc\":\"%s\","
                "\"position\":{"
                    "\"latitude\":%.7f,"
                    "\"longitude\":%.7f,"
                    "\"altitude\":%.1f,"
                    "\"locator\":\"%s\""
                "}"
            "},"
            "\"stop\":{"
                "\"time\":%lu,"
                "\"utc\":\"%s\","
                "\"position\":{"
                    "\"latitude\":%.7f,"
                    "\"longitude\":%.7f,"
                    "\"altitude\":%.1f,"
                    "\"locator\":\"%s\""
                "}"
            "},"
            "\"duration\":{"
                "\"seconds\":%lu,"
                "\"hms\":\"%s\""
            "},"
            "\"distance\":{"
                "\"airLine\":{"
                    "\"meters\":%lu,"
                    "\"km\":%.3f"
                "}"
            "},"
            "\"altitude\":{"
                "\"minimum\":%.1f,"
                "\"maximum\":%.1f"
            "}"
        "}",
        static_cast<unsigned long>(record.start.time),
        record.start.utc,
        record.start.latitude,
        record.start.longitude,
        record.start.altitude,
        record.start.locator,
        static_cast<unsigned long>(record.stop.time),
        record.stop.utc,
        record.stop.latitude,
        record.stop.longitude,
        record.stop.altitude,
        record.stop.locator,
        static_cast<unsigned long>(record.duration.seconds),
        record.duration.hms,
        static_cast<unsigned long>(record.distance.airLineMeters),
        record.distance.airLineKm,
        record.minimumAltitude,
        record.maximumAltitude
    );
    return (written > 0 && static_cast<size_t>(written) < size);
}

bool qth::saveCurrentRecord() {
    QTHRecord record {};

    if (!buildCurrentRecord(record)) {
        storage::appendErrorRecord("QTH_BUILD_FAILED");
        return false;
    }

    if (record.duration.seconds < MIN_DURATION_SECONDS) { return false; }
    char json[1024];

    if (!serializeJSONL(record, json, sizeof(json))) {
        storage::appendErrorRecord("QTH_SERIALIZE_FAILED");
        return false;
    }
    return storage::appendQTHRecord(json);
}
