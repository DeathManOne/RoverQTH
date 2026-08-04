/*
 * include/utilities/json.h
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
    class Reader {
        private:
            static constexpr uint8_t MAX_DEPTH = 8U;

            const char* _begin;
            const char* _end;
            bool _valid;

            Reader(const char* begin, const char* end);
            bool _findValue(const char* key, const char*& begin, const char*& end) const;
            static void _skipSpaces  (const char*& cursor, const char* end);
            static bool _skipString  (const char*& cursor, const char* end);
            static bool _skipNumber  (const char*& cursor, const char* end);
            static bool _skipObject  (const char*& cursor, const char* end, uint8_t depth);
            static bool _skipArray   (const char*& cursor, const char* end, uint8_t depth);
            static bool _skipValue   (const char*& cursor, const char* end, uint8_t depth);
            static bool _skipLiteral (const char*& cursor, const char* end, const char* literal);
            static bool _matchString (const char*& cursor, const char* end, const char* expected);
            static bool _decodeString(const char* begin,   const char* end, char* buffer, size_t size);
            static bool _isDigit    (char value);
            static bool _isHexDigit (char value);
            static uint8_t _hexValue(char value);
        public:
            /**
             * Creates a JSON Reader from a null-terminated object string.
             * The source string must remain valid for the entire lifetime
             * of the Reader. The complete JSON object is validated during
             * construction. No dynamic memory allocation is performed.
             *
             * @param content Null-terminated JSON object to read.
             */
            explicit Reader(const char* content);

            /**
             * Indicates whether the complete JSON object is structurally valid.
             * @return True if the object is valid, otherwise false.
             */
            bool valid() const;

            /**
             * Reads a nested JSON object.
             * The output Reader is modified only when the property exists
             * and contains a valid object.
             * @param key   Property name.
             * @param value Reader receiving the nested object.
             * @return True on success, otherwise false.
             */
            bool object(const char* key, Reader& value) const;

            /**
             * Reads a finite floating-point value.
             * The output value is modified only after successful validation.
             * @param key   Property name.
             * @param value Variable receiving the number.
             * @return True on success, otherwise false.
             */
            bool number(const char* key, double& value) const;

            /**
             * Reads a JSON boolean value.
             * The output value is modified only when the property contains
             * exactly true or false.
             * @param key   Property name.
             * @param value Variable receiving the boolean.
             * @return True on success, otherwise false.
             */
            bool boolean(const char* key, bool& value) const;

            /**
             * Reads and decodes a JSON string.
             * The destination buffer is cleared before reading and remains
             * empty if the operation fails. The buffer size includes the
             * null terminator.
             * @param key    Property name.
             * @param buffer Destination buffer.
             * @param size   Total destination buffer capacity.
             * @return True on success, otherwise false.
             */
            bool string(const char* key, char* buffer, size_t size) const;

            /**
             * Reads an unsigned integer value.
             * Negative, fractional and out-of-range values are rejected.
             * The output value is modified only after successful validation.
             * @param key   Property name.
             * @param value Variable receiving the unsigned integer.
             * @return True on success, otherwise false.
             */
            bool unsignedInteger(const char* key, uint64_t& value) const;
    };
    class Writer {
        private:
            static constexpr size_t MAX_DEPTH = 8U;
            struct Context {bool first;};

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
             * @return True on success, false if the Writer becomes invalid.
             */
            bool beginObject(const char* key = nullptr);

            /**
             * Closes the current JSON object.
             * @return True on success, false on invalid nesting.
             */
            bool endObject();

            /**
             * Writes a JSON string value.
             * @param key   Property name.
             * @param value UTF-8 string to serialize.
             * @return True on success, false if the Writer becomes invalid.
             */
            bool string(const char* key, const char* value);

            /**
             * Writes a JSON boolean value.
             * @param key   Property name.
             * @param value Boolean value.
             * @return True on success, false if the Writer becomes invalid.
             */
            bool boolean(const char* key, bool value);

            /**
             * Writes an unsigned integer value.
             * @param key   Property name.
             * @param value Unsigned integer.
             * @return True on success, false if the Writer becomes invalid.
             */
            bool unsignedInteger(const char* key, uint64_t value);

            /**
             * Writes a floating-point value.
             * Only finite values are accepted.
             * @param key      Property name.
             * @param value    Floating-point value.
             * @param decimals Number of decimal places (0-9).
             * @return True on success, false if the Writer becomes invalid.
             */
            bool number(const char* key, double value, uint8_t decimals);

            /**
             * Indicates whether the generated JSON is complete and valid.
             */
            bool complete() const;

            /**
             * Returns the length of the generated JSON.
             * Returns zero until complete() is true.
             */
            size_t length() const;

            /**
             * Returns the generated JSON string.
             * Returns nullptr until complete() is true.
             */
            const char* c_str() const;
    };
}
