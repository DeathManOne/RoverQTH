/*
 * include/utilities/clock.h
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

namespace utilities::clock {
    uint32_t toEpochUTC(int year, int month, int day, int hour, int minute, int second);
    bool formatISO8601(uint32_t utcEpoch, char* buffer, size_t size);

    void sync(const uint32_t utcEpoch);
    bool isSynced();

    uint32_t now();
    bool formatTime(uint8_t hour, uint8_t minute, uint8_t second, bool valid, char* buffer, size_t size, bool withSecond = true);
    void getDate(char* buffer, size_t size);
    void getTime(char* buffer, size_t size, bool withSecond = true);

    void reset();
}