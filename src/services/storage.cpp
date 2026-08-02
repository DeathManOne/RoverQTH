/*
 * src/services/storage.cpp
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

#include <cstdio>
#include "services/storage.h"
#include "utilities/clock.h"
#include "utilities/text.h"

namespace storage = services::storage;
namespace uClock  = utilities::clock;
namespace text    = utilities::text;

namespace {
    constexpr size_t MAX_LOGS_WAITING   = 20;
    constexpr size_t MAX_LOGS_LENGTH    = 256;

    constexpr const char* DIR_ROOT             = "/RoverQTH";

    constexpr const char* DIR_DATABASE         = "/RoverQTH/database";
    constexpr const char* FILE_DATABASE_POTA   = "/RoverQTH/database/pota.bin";
    constexpr const char* FILE_DATABASE_SOTA   = "/RoverQTH/database/sota.bin";

    constexpr const char* DIR_EXPORT           = "/RoverQTH/export";
    constexpr const char* FILE_EXPORT_QTH      = "/RoverQTH/export/QTH.json";
    constexpr const char* FILE_EXPORT_SETTINGS = "/RoverQTH/export/settings.json";

    constexpr const char* DIR_LOGS             = "/RoverQTH/logs";
    constexpr const char* FILE_LOGS_ERROR      = "/RoverQTH/logs/error.log";
    constexpr const char* FILE_LOGS_SYSTEM     = "/RoverQTH/logs/system.log";

    constexpr const char* DIR_QTH              = "/RoverQTH/qth";
    constexpr const char* DIR_TMP              = "/RoverQTH/tmp";
    constexpr const char* FILE_TMP_FIRMWARE    = "/RoverQTH/tmp/firmware.tmp";
    constexpr const char* FILE_TMP_POTA        = "/RoverQTH/tmp/pota.tmp";
    constexpr const char* FILE_TMP_SOTA        = "/RoverQTH/tmp/sota.tmp";

    struct WaitingLog {
        const char* path;
        char data[MAX_LOGS_LENGTH];
    };

    struct FileReadContext {
        char* buffer;
        size_t size;
        size_t length;
        bool valid;
    };

    SDCard *_sd              = nullptr;
    bool _ready              = false;
    size_t _waitingLogsCount = 0;
    WaitingLog _waitingLogs[MAX_LOGS_WAITING] {};

    bool _formatRecord(const char* data, char* buffer, size_t size);
    bool _queueLogs(const char* path, const char* data);
    bool _flushWaitingLogs();
    bool _ensureTree();
    bool _readFileChunk(const uint8_t* const data, const size_t length, void* const userData);

    bool _formatRecord(const char* data, char* buffer, size_t size) {
        if (!data || data[0] == '\0' || !buffer || size == 0) { return false; }

        int written = 0;
        if (uClock::isSynced()) {
            char timestamp[21];
            if (!uClock::formatISO8601(uClock::now(), timestamp, sizeof(timestamp)))
                { return false; }
            written = std::snprintf(buffer, size, "%s | %s\n", timestamp, data);
        } else {
            written = std::snprintf(buffer, size,
                "BOOT+%010lums | %s\n",
                static_cast<unsigned long>(millis()), data
            );
        }
        return (written > 0 && static_cast<size_t>(written) < size);
    }

    bool _queueLogs(const char* path, const char* data) {
        if (!path || !data || data[0] == '\0')     { return false; }
        if (_waitingLogsCount >= MAX_LOGS_WAITING) { return false; }

        WaitingLog& waiting = _waitingLogs[_waitingLogsCount];
        waiting.path        = path;

        if (!text::copy(waiting.data, sizeof(waiting.data), data)) {
            waiting.path = nullptr;
            return false;
        }
        ++_waitingLogsCount;
        return true;
    }

    bool _flushWaitingLogs() {
        if (!storage::isReady()) { return false; }

        size_t writtenCount = 0U;
        while (writtenCount < _waitingLogsCount) {
            const WaitingLog& waiting = _waitingLogs[writtenCount];
            if (waiting.path == nullptr || waiting.data[0] == '\0') {
                ++writtenCount;
                continue;
            }
            if (!_sd->fileWriteOrAppend(waiting.path, waiting.data))
                { break; }
            ++writtenCount;
        }

        if (writtenCount > 0U) {
            const size_t remaining = _waitingLogsCount - writtenCount;
            for (size_t index = 0U; index < remaining; ++index)
                { _waitingLogs[index] = _waitingLogs[writtenCount + index]; }
            for (size_t index = remaining; index < _waitingLogsCount; ++index) {
                _waitingLogs[index].path    = nullptr;
                _waitingLogs[index].data[0] = '\0';
            }
            _waitingLogsCount = remaining;
        }
        return _waitingLogsCount == 0U;
    }

    bool _ensureTree() {
        if (!storage::isReady()) { return false; }
        bool ok = true;
        ok &= _sd->dirCreate(DIR_ROOT);
        ok &= _sd->dirCreate(DIR_DATABASE);
        ok &= _sd->dirCreate(DIR_EXPORT);
        ok &= _sd->dirCreate(DIR_LOGS);
        ok &= _sd->dirCreate(DIR_QTH);
        ok &= _sd->dirCreate(DIR_TMP);
        return ok;
    }

    bool _readFileChunk(const uint8_t* const data, const size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        FileReadContext* const context =static_cast<FileReadContext*>(userData);
        if (!context->valid || context->buffer == nullptr || context->size == 0U) { return false; }
        if (context->length + length >= context->size) {
            context->valid = false;
            return false;
        }

        for (size_t index = 0U; index < length; ++index)
            { context->buffer[context->length + index] = static_cast<char>(data[index]); }
        context->length += length;
        context->buffer[context->length] = '\0';
        return true;
    }
}

bool storage::begin(SPIClass &spi, uint32_t timeoutSec) {
    spi.begin(SD_CLK, SD_MISO, SD_MOSI);
    if (!_sd) { _sd = new SDCard(); }

    const uint32_t start = millis();
    do {
        if (_sd->initialize(spi, SD_CS)) {
            _ready = true;
            if (!_ensureTree()) { _ready = false; }
            else { _flushWaitingLogs(); }
            return _ready;
        }
        delay(250);
    } while ((millis() - start) < timeoutSec * 1000);

    _ready = false;
    return _ready;
}

bool storage::isReady() {
    return _sd && _ready;
}

bool storage::readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used) {
    if (!isReady()) { return false; }
    return _sd->cardInfos(type, size, total, used);
}

bool storage::fileExists(const char* const path) {
    if (!isReady() || path == nullptr || path[0] == '\0') { return false; }
    return _sd->fileExists(path);
}

bool storage::readFile(const char* const path, char* const buffer, const size_t size) {
    if (!isReady() || path == nullptr || path[0] == '\0' || buffer == nullptr || size == 0U)
        { return false; }
    buffer[0] = '\0';

    FileReadContext context {buffer, size, 0U, true};
    const bool read = _sd->fileRead(path, _readFileChunk, &context);
    if (!read || !context.valid) {
        buffer[0] = '\0';
        return false;
    }
    return true;
}

bool storage::readFileLines(const char* const path, const LineCallback callback, void* const userData) {
    if (!isReady() || path == nullptr || path[0] == '\0' || callback == nullptr) { return false; }
    if (!_sd->fileReadLines( path, callback, userData))                          { return false; }
    return true;
}

bool storage::writeFile(const char* const path, const char* const data) {
    if (!isReady() || path == nullptr || path[0] == '\0' || data == nullptr) { return false; }
    return _sd->fileWrite(path, data);
}

bool storage::appendFile(const char* const path, const char* const data) {
    if (!isReady() || path == nullptr || path[0] == '\0' || data == nullptr) { return false; }
    return _sd->fileWriteOrAppend(path, data);
}

bool storage::renameFile(const char* const source, const char* const destination) {
    if (!isReady()             ||
        source == nullptr      || source[0] == '\0'      ||
        destination == nullptr || destination[0] == '\0'
    ) { return false; }
    return _sd->fileRename(source, destination);
}

bool storage::deleteFile(const char* const path) {
    if (!isReady() || path == nullptr || path[0] == '\0') { return false; }
    return _sd->fileDelete(path);
}

bool storage::appendErrorRecord(const char* const data) {
    if (data == nullptr || data[0] == '\0') { return false; }

    char line[MAX_LOGS_LENGTH];
    if (!_formatRecord(data, line, sizeof(line))) { return false; }

    if (!isReady())                                     { return _queueLogs(FILE_LOGS_ERROR, line); }
    if (_waitingLogsCount > 0U && !_flushWaitingLogs()) { return _queueLogs(FILE_LOGS_ERROR, line); }
    if (!_sd->fileWriteOrAppend(FILE_LOGS_ERROR, line)) { return _queueLogs(FILE_LOGS_ERROR, line); }

    return true;
}

bool storage::appendLogRecord(const char* const data) {
    if (data == nullptr || data[0] == '\0') { return false; }

    char line[MAX_LOGS_LENGTH];
    if (!_formatRecord(data, line, sizeof(line))) { return false; }

    if (!isReady())                                      { return _queueLogs(FILE_LOGS_SYSTEM, line); }
    if (_waitingLogsCount > 0U && !_flushWaitingLogs())  { return _queueLogs(FILE_LOGS_SYSTEM, line); }
    if (!_sd->fileWriteOrAppend(FILE_LOGS_SYSTEM, line)) { return _queueLogs(FILE_LOGS_SYSTEM, line); }

    return true;
}
