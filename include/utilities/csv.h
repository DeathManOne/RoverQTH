/*
 * include/utilities/csv.h
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

namespace utilities::csv {
    /**
     * @brief Copies one CSV field into a fixed-size string buffer.
     * @param source Start of the field content.
     * @param length Number of source characters to copy.
     * @param destination Destination buffer.
     * @param destinationSize Total destination capacity.
     * @return `true` when the complete field was copied, otherwise `false`.
     */
    bool copyField(const char* source, size_t length, char* destination, size_t destinationSize);

    /**
     * @brief Reads the next field from a CSV record and advances the cursor.
     * @param cursor Input cursor, updated to the next unread character.
     * @param destination Destination buffer for the decoded field.
     * @param destinationSize Total destination capacity.
     * @param delimiter Receives whether a field delimiter was consumed.
     * @return `true` when the field is valid and fits in the destination buffer,
     * otherwise `false`.
     */
    bool readField(const char* &cursor, char* destination, size_t destinationSize, bool &delimiter);
}
