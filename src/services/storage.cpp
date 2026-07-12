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

#include <cstring>
#include "services/storage.h"

namespace storage = services::storage;

namespace {
    SDCard *_sd = nullptr;
    bool _ready = false;

    constexpr const char* DIR_ROOT   = "/RoverQTH";
    constexpr const char* DIR_LOGS   = "/RoverQTH/logs";
    constexpr const char* DIR_CONFIG = "/RoverQTH/config";
    constexpr const char* DIR_UPDATE = "/RoverQTH/update";

    constexpr const char* FILE_QTH    = "/RoverQTH/logs/QTH.jsonl";
    constexpr const char* FILE_QSO    = "/RoverQTH/logs/QSO.jsonl";
    constexpr const char* FILE_DEBUG  = "/RoverQTH/logs/debug.log";
    constexpr const char* FILE_CONFIG = "/RoverQTH/config/settings.json";

    bool _available() { return _sd && _ready; }
}

bool storage::begin(SPIClass &spi, uint32_t timeoutSec) {
    spi.begin(SD_CLK, SD_MISO, SD_MOSI);
    if (!_sd) { _sd = new SDCard(); }

    const uint32_t start = millis();
    do {
        if (_sd->initialize(spi, SD_CS)) {
            _ready = true;
            ensureTree();
            return true;
        }
        delay(250);
    } while ((millis() - start) < timeoutSec * 1000);

    _ready = false;
    return false;
}

bool storage::isReady() { return _available(); }
SDCard* storage::card() { return _sd; }

bool storage::readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used) {
    if (!_available()) { return false; }
    return _sd->cardInfos(type, size, total, used);
}

bool storage::ensureTree() {
    if (!_available()) { return false; }

    bool ok = true;
    ok &= _sd->dirCreate(DIR_ROOT);
    ok &= _sd->dirCreate(DIR_LOGS);
    ok &= _sd->dirCreate(DIR_CONFIG);
    ok &= _sd->dirCreate(DIR_UPDATE);
    return ok;
}

bool storage::fileExists(const char* path) {
    if (!_available() || !path) { return false; }
    return _sd->fileExists(path);
}

size_t storage::fileSize(const char* path) {
    if (!_available() || !path) { return 0; }
    return _sd->fileSize(path);
}

bool storage::writeFile(const char* path, const char* data) {
    if (!_available() || !path || !data) { return false; }
    return _sd->fileWrite(path, data);
}

bool storage::appendFile(const char* path, const char* data) {
    if (!_available() || !path || !data) { return false; }
    return _sd->fileAppend(path, data);
}

bool storage::writeOrAppendFile(const char* path, const char* data) {
    if (!_available() || !path || !data) { return false; }
    return _sd->fileWriteOrAppend(path, data);
}

bool storage::readFile(const char* path, char* buffer, size_t size) {
    if (!_available() || !path || !buffer || size == 0) { return false; }

    struct Context {
        char* buffer;
        size_t size;
        size_t used;
    };

    Context ctx = {buffer, size, 0};
    buffer[0]   = '\0';

    const bool ok = _sd->fileRead(
        path, [](const uint8_t* chunk, size_t length, void* userData) -> bool {
            Context* ctx = static_cast<Context*>(userData);
            if (ctx->used >= ctx->size - 1) { return false; }

            const size_t remaining = (ctx->size - 1) - ctx->used;
            const size_t toCopy    = length < remaining ? length : remaining;

            memcpy(ctx->buffer + ctx->used, chunk, toCopy);
            ctx->used += toCopy;
            ctx->buffer[ctx->used] = '\0';
            return toCopy == length;
        }, &ctx
    );
    return ok && ctx.used > 0;
}

bool storage::deleteFile(const char* path) {
    if (!_available() || !path) { return false; }
    return _sd->fileDelete(path);
}

bool storage::renameFile(const char* from, const char* to) {
    if (!_available() || !from || !to) { return false; }
    return _sd->fileRename(from, to);
}

const char* storage::path(FileKind kind) {
    switch (kind) {
        case FileKind::QTH:    return FILE_QTH;
        case FileKind::QSO:    return FILE_QSO;
        case FileKind::DEBUG:  return FILE_DEBUG;
        case FileKind::CONFIG: return FILE_CONFIG;
        default:               return DIR_ROOT;
    }
}

bool storage::appendQTHRecord  (const char* data) { return writeOrAppendFile(FILE_QTH,   data); }
bool storage::appendQSORecord  (const char* data) { return writeOrAppendFile(FILE_QSO,   data); }
bool storage::appendDebugRecord(const char* data) { return writeOrAppendFile(FILE_DEBUG, data); }
