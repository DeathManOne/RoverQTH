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
    bool begin(SPIClass &spi, uint32_t timeoutSec = 10);
    bool isReady();

    bool readCardInfos(uint8_t &type, uint64_t &size, uint64_t &total, uint64_t &used);

    bool appendLogRecord  (const char* data);
    bool appendErrorRecord(const char* data);
    bool appendQTHRecord  (const char* data);
}