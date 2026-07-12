/*
 * include/services/storage.h
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <SPI.h>
#include <SDCard.h>

namespace services::storage {
    enum class FileKind {QTH, QSO, DEBUG, CONFIG};

    bool begin(SPIClass &spi, uint32_t timeoutSec = 10);
    bool isReady();

    SDCard* card();

    bool readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used);
    bool ensureTree();

    bool fileExists(const char* path);
    size_t fileSize(const char* path);

    bool writeFile        (const char* path, const char* data);
    bool appendFile       (const char* path, const char* data);
    bool writeOrAppendFile(const char* path, const char* data);

    bool readFile  (const char* path, char* buffer, size_t size);
    bool deleteFile(const char* path);
    bool renameFile(const char* from, const char* to);

    const char* path(FileKind kind);

    bool appendQTHRecord  (const char* data);
    bool appendQSORecord  (const char* data);
    bool appendDebugRecord(const char* data);
}