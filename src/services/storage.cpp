/*
 * services/storage.cpp
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

#include "services/storage.h"

namespace services::storage {
    namespace {
        SDCard *_SD = nullptr;
        bool _READY = false;

        constexpr const char* DIR_ROOT   = "/RoverQTH";
        constexpr const char* DIR_LOGS   = "/RoverQTH/logs";
        constexpr const char* DIR_CONFIG = "/RoverQTH/config";
        constexpr const char* DIR_UPDATE = "/RoverQTH/update";

        constexpr const char* FILE_QTH    = "/RoverQTH/logs/QTH.jsonl";
        constexpr const char* FILE_QSO    = "/RoverQTH/logs/QSO.jsonl";
        constexpr const char* FILE_DEBUG  = "/RoverQTH/logs/debug.log";
        constexpr const char* FILE_CONFIG = "/RoverQTH/config/settings.json";

        bool available() {
            return _SD && _READY;
        }
    }

    bool begin(SPIClass &spi, uint32_t timeoutSec) {
        spi.begin(SD_CLK, SD_MISO, SD_MOSI);
        if (!_SD)
            { _SD = new SDCard(); }
        const uint32_t start = millis();
        do {
            if (_SD->initialize(spi, SD_CS)) {
                _READY = true;
                ensureTree();
                return true;
            }
            delay(250);
        } while ((millis() - start) < timeoutSec * 1000);
        _READY = false;
        return false;
    }

    bool isReady() {
        return available();
    }

    SDCard* card() {
        return _SD;
    }

    bool readCardInfos(uint8_t &type, uint64_t &size, uint64_t &totalBytes, uint64_t &usedBytes) {
        if (!available())
            { return false; }
        return _SD->cardInfos(type, size, totalBytes, usedBytes);
    }

    bool ensureTree() {
        if (!available())
            { return false; }
        bool ok = true;
        ok &= _SD->dirCreate(DIR_ROOT);
        ok &= _SD->dirCreate(DIR_LOGS);
        ok &= _SD->dirCreate(DIR_CONFIG);
        ok &= _SD->dirCreate(DIR_UPDATE);
        return ok;
    }

    bool fileExists(const char* path) {
        if (!available() || !path)
            { return false; }
        return _SD->fileExists(path);
    }

    size_t fileSize(const char* path) {
        if (!available() || !path)
            { return 0; }
        return _SD->fileSize(path);
    }

    bool writeFile(const char* path, const char* data) {
        if (!available() || !path || !data)
            { return false; }
        return _SD->fileWrite(path, data);
    }

    bool appendFile(const char* path, const char* data) {
        if (!available() || !path || !data)
            { return false; }
        return _SD->fileAppend(path, data);
    }

    bool writeOrAppendFile(const char* path, const char* data) {
        if (!available() || !path || !data)
            { return false; }
        return _SD->fileWriteOrAppend(path, data);
    }

    bool readFile(const char* path, char* buffer, size_t size) {
        if (!available() || !path || !buffer || size == 0)
            { return false; }
        buffer[0] = '\0';
        std::string result;
        if (!_SD->fileRead(path, result))
            { return false; }
        snprintf(buffer, size, "%s", result.c_str());
        return true;
    }

    bool deleteFile(const char* path) {
        if (!available() || !path)
            { return false; }
        return _SD->fileDelete(path);
    }

    bool renameFile(const char* from, const char* to) {
        if (!available() || !from || !to)
            { return false; }
        return _SD->fileRename(from, to);
    }

    const char* path(FileKind kind) {
        switch (kind) {
            case FileKind::QTH:
                return FILE_QTH;
            case FileKind::QSO:
                return FILE_QSO;
            case FileKind::DEBUG:
                return FILE_DEBUG;
            case FileKind::CONFIG:
                return FILE_CONFIG;
        }
        return DIR_ROOT;
    }

    bool appendQTHRecord(const char* data) {
        return writeOrAppendFile(FILE_QTH, data);
    }

    bool appendQSORecord(const char* data) {
        return writeOrAppendFile(FILE_QSO, data);
    }

    bool appendDebugRecord(const char* data) {
        return writeOrAppendFile(FILE_DEBUG, data);
    }
}