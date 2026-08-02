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
    class Writer {
        public:
            Writer(char* buffer, size_t size);
            bool beginObject(const char* key = nullptr);
            bool endObject();
            bool string (const char* key, const char* value);
            bool boolean(const char* key, bool value);
            bool unsignedInteger(const char* key, uint64_t value);
            bool number(const char* key, double value, uint8_t decimals);
            bool complete() const;
            size_t length() const;
            const char* c_str() const;
        private:
            static constexpr size_t MAX_DEPTH = 8U;
            struct Context {
                bool first;
            };

            char* _buffer;
            bool _valid;
            bool _hasRoot;
            size_t _size;
            size_t _length;
            size_t _depth;
            Context _contexts[MAX_DEPTH];

            bool _beforeValue(const char* key);
            bool _push();
            bool _endObject();
            bool _append(char value);
            bool _append(const char* value);
            bool _appendFormatted(const char* format, ...);
            bool _appendQuoted(const char* value);
            bool _write(const char* data, size_t length);
            void _invalidate();
    };
    class Reader {
        public:
            explicit Reader(const char* content);
            bool valid() const;
            bool object (const char* key, Reader& value) const;
            bool number (const char* key, double& value) const;
            bool boolean(const char* key, bool& value) const;
            bool string (const char* key, char* buffer, size_t size) const;
            bool unsignedInteger(const char* key, uint64_t& value) const;

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
    };
}