/*
 * include/utilities/format.h
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

namespace utilities::format {
    bool durationCompact(uint32_t seconds, char* buffer, size_t size);
    bool durationHMS    (uint32_t seconds, char* buffer, size_t size);

    bool speed   (double kilometersPerHour, bool imperial, char* buffer, size_t size);
    bool altitude(double meters,            bool imperial, char* buffer, size_t size);
    bool distance(double kilometers,        bool imperial, char* buffer, size_t size);

    bool bearing(double degrees, char* buffer, size_t size);
    bool heading(double degrees, char* buffer, size_t size);

    bool percentage(uint64_t value, char* buffer, size_t size);
    bool voltage   (double volts,   char* buffer, size_t size);

    bool capacityMilliAmpHours(uint32_t milliAmpHours, char* buffer, size_t size);

    bool storageCapacity(uint64_t bytes, char* buffer, size_t size);
}