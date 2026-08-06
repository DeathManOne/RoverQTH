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
#include <limits>

#include "services/navigation.h"
#include "services/qth.h"
#include "services/settings.h"
#include "services/storage.h"
#include "utilities/distance.h"
#include "utilities/json.h"
#include "utilities/text.h"

namespace navigation = services::navigation;
namespace qth        = services::qth;
namespace settings   = services::settings;
namespace storage    = services::storage;
namespace distance   = utilities::distance;
namespace json       = utilities::json;
namespace text       = utilities::text;

namespace {
    enum class TraceRestoreStatus : uint8_t {RESTORED, MISSING_OR_EMPTY, ERROR};
    constexpr size_t ID_SIZE                       = 30U;
    constexpr size_t MODE_SIZE                     = 4U;
    constexpr size_t USER_ID_SIZE                  = 32U;
    constexpr size_t FILE_PATH_SIZE                = 64U;
    constexpr size_t TRACE_JSON_SIZE               = 160U;
    constexpr size_t RECORD_JSON_SIZE              = 768U;
    constexpr double TRACE_MIN_DISTANCE_METERS     = 1.0;
    constexpr double TRACE_MIN_ALTITUDE_CHANGE     = 0.5;
    constexpr uint32_t MIN_DURATION_SECONDS        = 5U;
    constexpr const char* DIR_QTH                  = "/RoverQTH/qth/";
    constexpr const char* FILE_EXTENSION           = ".json";
    constexpr const char* FILE_QTH_TRACE           = "/RoverQTH/qth/QTH.trace.tmp";
    constexpr const char* FILE_QTH_TEMPORARY       = "/RoverQTH/qth/QTH.tmp";
    constexpr const char* FILE_QTH_FINAL_TEMPORARY = "/RoverQTH/qth/QTH.final.tmp";

    struct TraceMergeContext {
        uint32_t lastUTC = 0U;
        bool first       = true;
        bool hasPoint    = false;
        bool valid       = true;
    };

    struct LastTraceContext {
        qth::TracePoint point;
        bool hasPoint = false;
        bool valid    = true;
    };

    struct PositionData {
        uint32_t utc     = 0U;
        double latitude  = 0.0;
        double longitude = 0.0;
        double altitude  = 0.0;
    };

    struct RecordData {
        char id[ID_SIZE]          {};
        char userId[USER_ID_SIZE] {};
        char mode[MODE_SIZE]      {};

        bool finished = false;
        PositionData start;
        PositionData end;
    };

    bool _formatUser(char* const userId, const size_t userIdSize, char* const mode, const size_t modeSize);
    bool _serializePosition(json::Writer& writer, const char* const key, const PositionData& position);
    bool _deserializePosition(const json::Reader& parent, const char* const key, PositionData& position);
    bool _validatePosition(const PositionData& position, bool requireUTC);
    bool _validateRecord(const RecordData& record);
    bool _writeRecord(const RecordData& record);
    bool _buildFilePath(const char* const id, char* const path, const size_t size);
    bool _mergeTraceLine(const char* const line, void* const userData);
    bool _writeFinalPrefix(const RecordData& record);
    bool _writeFinalRecord(const RecordData& record);
    bool _finalizeRecord(const RecordData& record, bool allowExistingFinal);
    void _clearRestoredRecord();
    void _clearLastTracePoint();
    bool _validateTracePoint(const qth::TracePoint& point);
    bool _shouldAppendTracePoint(const qth::TracePoint& point);
    bool _deserializeTracePoint(const char* const content, qth::TracePoint& point);
    bool _readLastTracePoint(const char* const line, void* const userData);
    TraceRestoreStatus _restoreLastTracePoint();
    bool _restoreOrCreateTrace(const RecordData& record);
    bool _writeTracePoint(const qth::TracePoint& point);
    bool _isSameTracePoint(const qth::TracePoint& first, const qth::TracePoint& second);
    bool _createId(uint32_t startUTC, uint32_t startedAtMillis, char* buffer, size_t size);
    bool _buildCurrentRecord(RecordData& record);
    bool _serialize(json::Writer& writer, const RecordData& record);
    bool _deserialize(const char* content, RecordData& record);
    bool _loadTemporaryRecord(RecordData& record);
    bool _hasTemporaryRecord();

    RecordData _restoredRecord     {};
    qth::TracePoint _lastTracePoint {};
    bool _hasLastTracePoint = false;
    bool _hasRestoredRecord = false;

    bool _formatUser(char* const userId, const size_t userIdSize, char* const mode, const size_t modeSize) {
        const settings::General configuration = settings::general();

        if (configuration.callsign[0] == '\0')                       { return false; }
        if (!text::copy(userId, userIdSize, configuration.callsign)) { return false; }

        const char* const suffix = settings::callsignSuffixText(configuration.suffix);
        return text::copy(mode, modeSize, suffix);
    }

    bool _serializePosition(json::Writer& writer, const char* const key, const PositionData& position) {
        return
            writer.beginObject(key)                            &&
            writer.unsignedInteger("utc", position.utc)        &&
            writer.number("latitude", position.latitude, 7U)   &&
            writer.number("longitude", position.longitude, 7U) &&
            writer.number("altitude", position.altitude, 1U)   &&
            writer.endObject();
    }

    bool _deserializePosition(const json::Reader& parent, const char* const key, PositionData& position) {
        json::Reader object(nullptr);
        if (!parent.object(key, object)) { return false; }

        uint64_t utc = 0U;
        if (!object.unsignedInteger( "utc", utc) ||
            utc > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) ||
            !object.number("latitude",  position.latitude)  ||
            !object.number("longitude", position.longitude) ||
            !object.number("altitude",  position.altitude)
        ) { return false; }

        position.utc = static_cast<uint32_t>(utc);
        return true;
    }

    bool _validatePosition(const PositionData& position, const bool requireUTC) {
        if (requireUTC && position.utc == 0U) { return false; }
        if (!std::isfinite(position.latitude)  ||
            !std::isfinite(position.longitude) ||
            !std::isfinite(position.altitude)
        ) { return false; }
        return
            position.latitude >= -90.0   && position.latitude <= 90.0   &&
            position.longitude >= -180.0 && position.longitude <= 180.0;
    }

    bool _validateRecord(const RecordData& record) {
        if (record.id[0] == '\0' || record.userId[0] == '\0') { return false; }
        if (!_validatePosition(record.start, true)) { return false; }
        if (!record.finished) { return record.end.utc == 0U; }
        if (!_validatePosition(record.end, true)) { return false; }
        return record.end.utc >= record.start.utc;
    }

    bool _writeRecord(const RecordData& record) {
        char buffer[RECORD_JSON_SIZE];

        json::Writer writer(buffer, sizeof(buffer));
        if (!_serialize(writer, record)) { return false; }

        const char* const serialized = writer.c_str();
        if (serialized == nullptr) { return false; }
        return storage::writeFile( FILE_QTH_TEMPORARY, serialized);
    }

    bool _buildFilePath(const char* const id, char* const path, const size_t size) {
        if (id == nullptr || id[0] == '\0' || path == nullptr || size == 0U) { return false; }
        const int written = std::snprintf(path, size, "%s%s%s", DIR_QTH, id, FILE_EXTENSION);
        return written > 0 && static_cast<size_t>(written) < size;
    }

    bool _mergeTraceLine(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        TraceMergeContext* const context = static_cast<TraceMergeContext*>(userData);
        if (!context->valid) { return false; }
        if (line[0] == '\0') { return true; }

        qth::TracePoint point {};
        if (!_deserializeTracePoint(line, point)) {
            context->valid = false;
            return false;
        }

        if (context->hasPoint && point.utc < context->lastUTC) {
            context->valid = false;
            return false;
        }

        if (!context->first && !storage::appendFile(FILE_QTH_FINAL_TEMPORARY, ",")) {
            context->valid = false;
            return false;
        }

        if (!storage::appendFile(FILE_QTH_FINAL_TEMPORARY, line)) {
            context->valid = false;
            return false;
        }

        context->lastUTC  = point.utc;
        context->first    = false;
        context->hasPoint = true;
        return true;
    }

    bool _writeFinalPrefix(const RecordData& record) {
        char buffer[RECORD_JSON_SIZE];

        json::Writer writer(buffer, sizeof(buffer));
        if (!_serialize(writer, record)) { return false; }

        const char* const serialized = writer.c_str();
        const size_t length          = writer.length();

        if (serialized == nullptr || length == 0U || serialized[length - 1U] != '}')
            { return false; }
        buffer[length - 1U] = '\0';

        constexpr const char* TRACE_PREFIX = ",\"trace\":[";
        const int written = std::snprintf(
            buffer + length - 1U,
            sizeof(buffer) - (length - 1U),
            "%s", TRACE_PREFIX
        );

        if (written <= 0 || static_cast<size_t>(written) >= sizeof(buffer) - (length - 1U))
            { return false; }
        return storage::writeFile(FILE_QTH_FINAL_TEMPORARY, buffer);
    }

    bool _writeFinalRecord(const RecordData& record) {
        if (!_writeFinalPrefix(record)) { return false; }

        TraceMergeContext context {};
        const bool read = storage::readFileLines(FILE_QTH_TRACE, _mergeTraceLine, &context);

        if (!read || !context.valid) {
            storage::deleteFile(FILE_QTH_FINAL_TEMPORARY);
            return false;
        }

        if (!storage::appendFile(FILE_QTH_FINAL_TEMPORARY, "]}")) {
            storage::deleteFile(FILE_QTH_FINAL_TEMPORARY);
            return false;
        }
        return true;
    }

    bool _finalizeRecord(const RecordData& record, const bool allowExistingFinal) {
        if (!record.finished) { return false; }

        char finalPath[FILE_PATH_SIZE];
        if (!_buildFilePath(record.id, finalPath, sizeof(finalPath))) { return false; }

        const bool finalExists = storage::fileExists(finalPath);
        if (finalExists && allowExistingFinal) {
            if (storage::fileExists(FILE_QTH_FINAL_TEMPORARY) && !storage::deleteFile(FILE_QTH_FINAL_TEMPORARY))
                {storage::appendErrorRecord("QTH_FINAL_TEMP_DELETE_FAILED"); }
        } else {
            if (finalExists) { return false; }
            if (!_writeFinalRecord(record)) { return false; }
            if (!storage::renameFile(FILE_QTH_FINAL_TEMPORARY, finalPath)) { return false; }
        }

        if (storage::fileExists(FILE_QTH_TEMPORARY) && !storage::deleteFile(FILE_QTH_TEMPORARY))
            { storage::appendErrorRecord("QTH_TEMP_DELETE_FAILED"); }
        if (storage::fileExists(FILE_QTH_TRACE) && !storage::deleteFile(FILE_QTH_TRACE))
            { storage::appendErrorRecord("QTH_TRACE_DELETE_FAILED"); }
        _clearRestoredRecord();
        _clearLastTracePoint();
        return true;
    }

    void _clearRestoredRecord() {
        _restoredRecord    = {};
        _hasRestoredRecord = false;
    }

    void _clearLastTracePoint() {
        _lastTracePoint    = {};
        _hasLastTracePoint = false;
    }

    bool _validateTracePoint(const qth::TracePoint& point) {
        if (point.utc == 0U) { return false; }
        if (!std::isfinite(point.latitude)  ||
            !std::isfinite(point.longitude) ||
            !std::isfinite(point.altitude)
        ) { return false; }
        return
            point.latitude >= -90.0   && point.latitude <= 90.0   &&
            point.longitude >= -180.0 && point.longitude <= 180.0;
    }

    bool _shouldAppendTracePoint(const qth::TracePoint& point) {
        if (!_validateTracePoint(point))      { return false; }
        if (!_hasLastTracePoint)              { return true; }
        if (point.utc <= _lastTracePoint.utc) { return false; }

        const double horizontalMeters = distance::betweenKilometers(
            _lastTracePoint.latitude, _lastTracePoint.longitude,
            point.latitude,           point.longitude
        ) * 1000.0;
        const double altitudeChange = std::fabs(point.altitude - _lastTracePoint.altitude);

        return
            horizontalMeters >= TRACE_MIN_DISTANCE_METERS ||
            altitudeChange >= TRACE_MIN_ALTITUDE_CHANGE;
    }

    bool _deserializeTracePoint(const char* const content, qth::TracePoint& point) {
        if (content == nullptr) { return false; }

        json::Reader reader(content);
        if (!reader.valid()) { return false; }

        uint64_t utc = 0U;
        qth::TracePoint restored {};
        if (!reader.unsignedInteger("utc", utc) || utc == 0U ||
            utc > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) ||
            !reader.number("latitude", restored.latitude)                     ||
            !reader.number("longitude", restored.longitude)                   ||
            !reader.number("altitude", restored.altitude)
        ) { return false; }

        restored.utc = static_cast<uint32_t>(utc);
        if (!_validateTracePoint(restored))
            { return false; }
        point = restored;
        return true;
    }

    bool _readLastTracePoint(const char* const line, void* const userData) {
        if (line == nullptr || userData == nullptr) { return false; }

        LastTraceContext* const context = static_cast<LastTraceContext*>(userData);
        if (line[0] == '\0') { return true; }

        qth::TracePoint point {};
        if (!_deserializeTracePoint(line, point)) {
            context->valid = false;
            return false;
        }

        if (context->hasPoint && point.utc < context->point.utc) {
            context->valid = false;
            return false;
        }

        context->point    = point;
        context->hasPoint = true;
        return true;
    }

    TraceRestoreStatus _restoreLastTracePoint() {
        _clearLastTracePoint();
        if (!storage::fileExists(FILE_QTH_TRACE))
            { return TraceRestoreStatus::MISSING_OR_EMPTY; }
        LastTraceContext context {};

        const bool read = storage::readFileLines(FILE_QTH_TRACE, _readLastTracePoint, &context);
        if (!read || !context.valid) { return TraceRestoreStatus::ERROR; }
        if (!context.hasPoint) { return TraceRestoreStatus::MISSING_OR_EMPTY; }

        _lastTracePoint    = context.point;
        _hasLastTracePoint = true;
        return TraceRestoreStatus::RESTORED;
    }

    bool _restoreOrCreateTrace(const RecordData& record) {
        const TraceRestoreStatus status = _restoreLastTracePoint();
        if (status == TraceRestoreStatus::RESTORED) { return true; }
        if (status == TraceRestoreStatus::ERROR)    { return false; }

        qth::TracePoint startPoint {};
        startPoint.utc       = record.start.utc;
        startPoint.latitude  = record.start.latitude;
        startPoint.longitude = record.start.longitude;
        startPoint.altitude  = record.start.altitude;

        if (!storage::writeFile(FILE_QTH_TRACE, "")) { return false; }
        _clearLastTracePoint();
        return _writeTracePoint(startPoint);
    }

    bool _writeTracePoint(const qth::TracePoint& point) {
        if (!_validateTracePoint(point))                           { return false; }
        if (_hasLastTracePoint && point.utc < _lastTracePoint.utc) { return false; }

        char jsonBuffer[TRACE_JSON_SIZE];
        json::Writer writer(jsonBuffer, sizeof(jsonBuffer));

        if (!writer.beginObject() ||
            !writer.unsignedInteger("utc", point.utc)           ||
            !writer.number("latitude",     point.latitude, 7U)  ||
            !writer.number("longitude",    point.longitude, 7U) ||
            !writer.number("altitude",     point.altitude, 1U)  ||
            !writer.endObject() ||
            !writer.complete()
        ) { return false; }

        const char* const serialized = writer.c_str();
        if (serialized == nullptr) { return false; }

        char line[TRACE_JSON_SIZE + 2U];
        const int written = std::snprintf(line, sizeof(line), "%s\n", serialized);

        if (written <= 0 || static_cast<size_t>(written) >= sizeof(line)) { return false; }
        if (!storage::appendFile(FILE_QTH_TRACE, line))                   { return false; }

        _lastTracePoint    = point;
        _hasLastTracePoint = true;
        return true;
    }

    bool _isSameTracePoint(const qth::TracePoint& first, const qth::TracePoint& second) {
        return
            first.utc       == second.utc       &&
            first.latitude  == second.latitude  &&
            first.longitude == second.longitude &&
            first.altitude  == second.altitude;
    }

    bool _createId(const uint32_t startUTC, const uint32_t startedAtMillis, char* const buffer, const size_t size) {
        if (startUTC == 0U || buffer == nullptr || size < ID_SIZE) { return false; }

        const time_t value = static_cast<time_t>(startUTC);
        tm utc {};

        if (gmtime_r(&value, &utc) == nullptr) { return false; }
        const int written = std::snprintf(buffer, size,
            "%04d%02d%02d-%02d%02d%02d-%010lu",
            utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
            utc.tm_hour,        utc.tm_min,     utc.tm_sec,
            static_cast<unsigned long>(startedAtMillis)
        );
        return written == 26 && static_cast<size_t>(written) < size;
    }

    bool _buildCurrentRecord(RecordData& record) {
        navigation::MarkSnapshot snapshot {};
        if (!navigation::getMarkSnapshot(snapshot)) { return false; }

        record = {};
        if (_hasRestoredRecord) {
            if (!text::copy(record.id,     sizeof(record.id),     _restoredRecord.id)     ||
                !text::copy(record.userId, sizeof(record.userId), _restoredRecord.userId) ||
                !text::copy(record.mode,   sizeof(record.mode),   _restoredRecord.mode)
            ) { return false; }
        } else {
            if (!_createId(snapshot.startUTC, snapshot.startedAtMillis, record.id, sizeof(record.id)))
                { return false; }
            if (!_formatUser(record.userId, sizeof(record.userId), record.mode, sizeof(record.mode)))
                { return false; }
        }

        record.finished         = snapshot.hasEnd;
        record.start.utc        = snapshot.startUTC;
        record.start.latitude   = snapshot.start.latitude;
        record.start.longitude  = snapshot.start.longitude;
        record.start.altitude   = snapshot.start.altitude;

        if (record.finished) {
            record.end.utc       = snapshot.stopUTC;
            record.end.latitude  = snapshot.end.latitude;
            record.end.longitude = snapshot.end.longitude;
            record.end.altitude  = snapshot.end.altitude;
        }
        return true;
    }
    
    bool _serialize(json::Writer& writer, const RecordData& record) {
        if (!_validateRecord(record)) { return false; }
        if (!writer.beginObject()                              ||
            !writer.string("id",        record.id)             ||
            !writer.string("userId",    record.userId)         ||
            !writer.string("mode",      record.mode)           ||
            !writer.boolean("finished", record.finished)       ||
            !_serializePosition(writer, "start", record.start) ||
            !_serializePosition(writer, "end", record.end)     ||
            !writer.endObject()
        ) { return false; }
        return writer.complete();
    }

    bool _deserialize(const char* const content, RecordData& record) {
        if (content == nullptr) { return false; }

        json::Reader root(content);
        if (!root.valid()) { return false; }

        RecordData restored {};
        if (!root.string("id",        restored.id,     sizeof(restored.id))     ||
            !root.string("userId",    restored.userId, sizeof(restored.userId)) ||
            !root.string("mode",      restored.mode,   sizeof(restored.mode))   ||
            !root.boolean("finished", restored.finished)                        ||
            !_deserializePosition(root, "start", restored.start)                ||
            !_deserializePosition(root, "end",   restored.end)
        ) { return false; }

        if (!_validateRecord(restored)) { return false; }
        record = restored;
        return true;
    }

    bool _loadTemporaryRecord(RecordData& record) {
        if (!_hasTemporaryRecord()) { return false; }

        char buffer[RECORD_JSON_SIZE];
        if (!storage::readFile(FILE_QTH_TEMPORARY, buffer, sizeof(buffer))) { return false; }
        return _deserialize(buffer, record);
    }

    bool _hasTemporaryRecord() {
        return storage::fileExists(FILE_QTH_TEMPORARY);
    }
}

bool qth::isCurrentRecordLongEnough() {
    return navigation::markDurationSeconds() >= MIN_DURATION_SECONDS;
}

qth::RecoveryStatus qth::recoverTemporaryRecord() {
    if (!_hasTemporaryRecord()) { return RecoveryStatus::NONE; }
    if (_hasRestoredRecord)     { return RecoveryStatus::ERROR; }

    RecordData record {};
    if (!_loadTemporaryRecord(record)) { return RecoveryStatus::ERROR; }
    if (record.finished) {
        if (!_finalizeRecord(record, true)) { return RecoveryStatus::ERROR; }
        return RecoveryStatus::RECORD_FINALIZED;
    }

    if (!_restoreOrCreateTrace(record)) { return RecoveryStatus::ERROR; }

    navigation::Coordinate start {};
    start.latitude  = record.start.latitude;
    start.longitude = record.start.longitude;
    start.altitude  = record.start.altitude;

    if (!navigation::restoreMark(start, record.start.utc))
        { return RecoveryStatus::ERROR; }
    _restoredRecord = record;
    _hasRestoredRecord = true;
    return RecoveryStatus::RECORDING_RESTORED;
}

bool qth::appendTracePoint(const TracePoint& point) {
    if (!_validateTracePoint(point))     { return false; }
    if (!_shouldAppendTracePoint(point)) { return true; }
    return _writeTracePoint(point);
}

bool qth::appendFinalTracePoint(const TracePoint& point) {
    if (!_validateTracePoint(point))                                     { return false; }
    if (_hasLastTracePoint && _isSameTracePoint(_lastTracePoint, point)) { return true; }
    return _writeTracePoint(point);
}

bool qth::resetTrace() {
    _clearLastTracePoint();
    return storage::writeFile(FILE_QTH_TRACE, "");
}

bool qth::saveTemporaryRecord() {
    _clearRestoredRecord();
    RecordData record {};
    if (!_buildCurrentRecord(record)) { return false; }
    if (record.finished)             { return false; }
    return _writeRecord(record);
}

bool qth::discardTemporaryRecord() {
    const bool deleted = storage::deleteFile(FILE_QTH_TEMPORARY);
    _clearRestoredRecord();
    return deleted;
}

bool qth::discardTemporaryTrace() {
    const bool deleted = storage::deleteFile(FILE_QTH_TRACE);
    _clearLastTracePoint();
    return deleted;
}

bool qth::saveCurrentRecord() {
    RecordData record {};
    if (!_buildCurrentRecord(record)) { return false; }
    if (!record.finished)            { return false; }
    if (!_writeRecord(record))       { return false; }
    return _finalizeRecord(record, false);
}
