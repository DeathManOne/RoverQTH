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
    /**
     * Callback invoked for each line read from a text file.
     * @param line     Null-terminated line.
     * @param userData User-defined context pointer.
     * @return true to continue reading, false to stop.
     */
    using LineCallback = bool (*)(const char* line, void* userData);

    /**
     * Initializes the storage service.
     * @param spi        SPI bus used to access the SD card.
     * @param timeoutSec Maximum initialization time in seconds.
     * @return true if the storage service was successfully initialized, false otherwise.
     */
    bool begin(SPIClass &spi, uint32_t timeoutSec = 10);

    /**
     * Indicates whether the storage service is ready.
     * @return true if the storage service is ready, false otherwise.
     */
    bool isReady();

    /**
     * Reads SD card information.
     * @param type  Receives the SD card type.
     * @param size  Receives the SD card capacity in bytes.
     * @param total Receives the total filesystem size in bytes.
     * @param used  Receives the used filesystem size in bytes.
     * @return true if the information was successfully retrieved, false otherwise.
     */
    bool readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used);

    /**
     * Reads a text file line by line.
     * @param path      File path.
     * @param callback  Callback invoked for each line.
     * @param userData  User-defined context passed to the callback.
     * @return true if the file was successfully processed, false otherwise.
     */
    bool readFileLines(const char* path, LineCallback callback, void* userData = nullptr);

    /**
     * Indicates whether a file exists.
     * @param path File path.
     * @return true if the file exists, false otherwise.
     */
    bool fileExists(const char* path);

    /**
     * Reads the content of a file.
     * @param path   File path.
     * @param buffer Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return true if the file was successfully read, false otherwise.
     */
    bool readFile(const char* path, char* buffer, size_t size);

    /**
     * Writes a file.
     * @param path File path.
     * @param data Null-terminated data to write.
     * @return true if the file was successfully written, false otherwise.
     */
    bool writeFile(const char* path, const char* data);

    /**
     * Appends data to a file.
     * @param path File path.
     * @param data Null-terminated data to append.
     * @return true if the data was successfully appended, false otherwise.
     */
    bool appendFile(const char* path, const char* data);

    /**
     * Renames a file.
     * @param source      Current file path.
     * @param destination New file path.
     * @return true if the file was successfully renamed, false otherwise.
     */
    bool renameFile(const char* source, const char* destination);

    /**
     * Deletes a file.
     * @param path File path.
     * @return true if the file was successfully deleted, false otherwise.
     */
    bool deleteFile(const char* path);

    /**
     * Appends a record to the system log.
     * @param data Null-terminated log record.
     * @return true if the record was successfully appended, false otherwise.
     */
    bool appendLogRecord(const char* data);

    /**
     * Appends a record to the error log.
     * @param data Null-terminated error record.
     * @return true if the record was successfully appended, false otherwise.
     */
    bool appendErrorRecord(const char* data);
}
