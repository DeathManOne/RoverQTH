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
#include <ctime>
#include "services/storage.h"
#include "utilities/clock.h"

namespace storage = services::storage;
namespace uClock   = utilities::clock;

namespace {
    constexpr size_t MAX_LOGS_WAITING   = 20;
    constexpr size_t MAX_LOGS_LENGTH    = 256;

    struct WaitingLog {
        const char* path;
        char data[MAX_LOGS_LENGTH];
    };

    SDCard *_sd              = nullptr;
    bool _ready              = false;
    size_t _waitingLogsCount = 0;
    WaitingLog _waitingLogs[MAX_LOGS_WAITING] {};

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
    constexpr const char* FILE_QTH             = "/RoverQTH/qth/QTH.jsonl";

    constexpr const char* DIR_TMP              = "/RoverQTH/tmp";
    constexpr const char* FILE_TMP_FIRMWARE    = "/RoverQTH/tmp/firmware.tmp";
    constexpr const char* FILE_TMP_POTA        = "/RoverQTH/tmp/pota.tmp";
    constexpr const char* FILE_TMP_SOTA        = "/RoverQTH/tmp/sota.tmp";

    bool _formatRecord(const char* data, char* buffer, size_t size);
    bool _queueLogs(const char* path, const char* data);
    void _clearWaitingLogs();
    void _flushWaitingLogs();
    bool _ensureTree();

    bool _formatRecord(const char* data, char* buffer, size_t size) {
        if (!data || data[0] == '\0' || !buffer || size == 0) { return false; }

        int written = 0;
        if (uClock::isSynced()) {
            const time_t rawTime = static_cast<time_t>(uClock::now());

            struct tm utcTime {};
            if (!gmtime_r(&rawTime, &utcTime)) { return false; }

            written = std::snprintf(
                buffer, size,
                "%04d-%02d-%02dT%02d:%02d:%02dZ | %s\n",
                utcTime.tm_year + 1900, utcTime.tm_mon + 1, utcTime.tm_mday,
                utcTime.tm_hour, utcTime.tm_min, utcTime.tm_sec, data
            );
        } else {
            written = std::snprintf(
                buffer, size,
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

        std::snprintf(waiting.data, MAX_LOGS_LENGTH, "%s", data);
        ++_waitingLogsCount;
        return true;
    }

    void _clearWaitingLogs() {
        for (size_t index = 0; index < _waitingLogsCount; ++index) {
            _waitingLogs[index].path    = nullptr;
            _waitingLogs[index].data[0] = '\0';
        }
        _waitingLogsCount = 0;
    }

    void _flushWaitingLogs() {
        if (!storage::isReady()) { return; }
        for (size_t index = 0; index < _waitingLogsCount; ++index) {
            const WaitingLog& waiting = _waitingLogs[index];
            if (!waiting.path || waiting.data[0] == '\0') { continue; }
            if (!_sd->fileWriteOrAppend(waiting.path, waiting.data)) {
                _ready = false;
                _clearWaitingLogs();
                return;
            }
        }
        _clearWaitingLogs();
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
}

bool storage::begin(SPIClass &spi, uint32_t timeoutSec) {
    spi.begin(SD_CLK, SD_MISO, SD_MOSI);
    if (!_sd) { _sd = new SDCard(); }

    const uint32_t start = millis();
    do {
        if (_sd->initialize(spi, SD_CS)) {
            _ready = true;
            if (!_ensureTree()) {
                _ready = false;
                _clearWaitingLogs();
            } else { _flushWaitingLogs(); }
            return _ready;
        }
        delay(250);
    } while ((millis() - start) < timeoutSec * 1000);

    _clearWaitingLogs();
    _ready = false;
    return _ready;
}

bool storage::isReady() { return _sd && _ready; }

bool storage::readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used) {
    if (!isReady()) { return false; }
    return _sd->cardInfos(type, size, total, used);
}

bool storage::appendErrorRecord(const char* data) {
    if (!data || data[0] == '\0') {return false; }

    char line[MAX_LOGS_LENGTH];
    if (!_formatRecord(data, line, sizeof(line))) { return false; }
    if (!_sd) {return _queueLogs(FILE_LOGS_ERROR, line); }
    if (!isReady()) { return false; }
    if (!_sd->fileWriteOrAppend(FILE_LOGS_ERROR, line)) {
        _ready = false;
        _clearWaitingLogs();
        return false;
    }
    return true;
}

bool storage::appendLogRecord(const char* data) {
    if (!data || data[0] == '\0') {return false; }

    char line[MAX_LOGS_LENGTH];
    if (!_formatRecord(data, line, sizeof(line))) { return false; }
    if (!_sd) {return _queueLogs(FILE_LOGS_SYSTEM, line); }
    if (!isReady()) { return false; }
    if (!_sd->fileWriteOrAppend(FILE_LOGS_SYSTEM, line)) {
        _ready = false;
        _clearWaitingLogs();
        return false;
    }
    return true;
}

bool storage::appendQTHRecord(const char* data) {
    if (!data || data[0] == '\0' || !isReady()) { return false; }

    char line[1032];
    const int written = std::snprintf(line, sizeof(line), "%s\n", data);

    if (written <= 0 || static_cast<size_t>(written) >= sizeof(line))
        { return false; }
    if (!_sd->fileWriteOrAppend(FILE_QTH, line)) {
        _ready = false;
        _clearWaitingLogs();
        return false;
    }
    return true;
}
