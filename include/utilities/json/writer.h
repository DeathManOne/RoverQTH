/*
 * include/utilities/json/writer.h
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

namespace utilities::json {
    /** @brief Allocation-free JSON object writer backed by a caller-owned buffer. */
    class Writer {
        private:
            static constexpr size_t MAX_DEPTH = 8U;
            struct Context { bool first; };

            char* _buffer;
            bool _valid;
            bool _hasRoot;
            size_t _size;
            size_t _length;
            size_t _depth;
            Context _contexts[MAX_DEPTH];

            bool _beforeValue    (const char* key);
            bool _push();
            bool _append         (char value);
            bool _append         (const char* value);
            bool _appendFormatted(const char* format, ...);
            bool _appendQuoted   (const char* value);
            bool _write          (const char* data, size_t length);
            void _invalidate();
        public:
            /**
             * Creates a JSON Writer using a caller-provided buffer.
             * The buffer must remain valid for the entire lifetime of the Writer.
             * No dynamic memory allocation is performed.
             * @param buffer Destination buffer for the generated JSON.
             * @param size   Total buffer capacity, including the null terminator.
             */
            Writer(char* buffer, size_t size);

            /**
             * Creates a JSON object.
             * @param key Optional object key when creating a nested object.
             * @return `true` when the object was started successfully, otherwise `false`.
             */
            bool beginObject(const char* key = nullptr);

            /**
             * Closes the current JSON object.
             * @return `true` when the object was closed successfully, otherwise `false`.
             */
            bool endObject();

            /**
             * Writes a JSON string value.
             * @param key   Property name.
             * @param value UTF-8 string to serialize.
             * @return `true` when the string was written successfully, otherwise `false`.
             */
            bool string(const char* key, const char* value);

            /**
             * Writes a JSON boolean value.
             * @param key   Property name.
             * @param value Boolean value.
             * @return `true` when the boolean was written successfully, otherwise `false`.
             */
            bool boolean(const char* key, bool value);

            /**
             * Writes an unsigned integer value.
             * @param key   Property name.
             * @param value Unsigned integer.
             * @return `true` when the unsigned integer was written successfully, otherwise `false`.
             */
            bool unsignedInteger(const char* key, uint64_t value);

            /**
             * Writes a floating-point value.
             * Only finite values are accepted.
             * @param key      Property name.
             * @param value    Floating-point value.
             * @param decimals Number of decimal places (0-9).
             * @return `true` when the number was written successfully, otherwise `false`.
             */
            bool number(const char* key, double value, uint8_t decimals);

            /**
             * Indicates whether the generated JSON is complete and valid.
             * @return `true` when the root object is complete and the Writer remains valid,
             * otherwise `false`.
             */
            bool complete() const;

            /**
             * Returns the length of the generated JSON.
             * @return Generated JSON length, or zero until complete() returns `true`.
             */
            size_t length() const;

            /**
             * Returns the generated JSON string.
             * @return Null-terminated generated JSON, or `nullptr` until complete() returns `true`.
             */
            const char* c_str() const;
    };
}
