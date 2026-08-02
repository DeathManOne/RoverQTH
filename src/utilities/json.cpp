/*
 * src/utilities/json.cpp
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

#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "utilities/json.h"

namespace json = utilities::json;

json::Writer::Writer(char* const buffer, const size_t size) :
    _buffer(buffer), _size(size), _length(0U), _depth(0U),
    _valid(buffer != nullptr && size > 0U), _hasRoot(false)
{
    if (_valid) { _buffer[0] = '\0'; }
}

bool json::Writer::beginObject(const char* const key) {
    if (!_valid || _depth >= MAX_DEPTH || !_beforeValue(key) || !_append('{')) { return false; }
    return _push();
}

bool json::Writer::endObject() {
    return _endObject();
}

bool json::Writer::string(const char* const key, const char* const value) {
    if (value == nullptr || !_beforeValue(key)) { return false; }
    return _appendQuoted(value);
}

bool json::Writer::boolean(const char* const key, const bool value) {
    if (!_beforeValue(key)) { return false; }
    return _append(value ? "true" : "false");
}

bool json::Writer::unsignedInteger(const char* const key, const uint64_t value) {
    if (!_beforeValue(key)) { return false; }
    return _appendFormatted("%llu", static_cast<unsigned long long>(value));
}

bool json::Writer::number(const char* const key, const double value, const uint8_t decimals) {
    if (!std::isfinite(value) || decimals > 9U || !_beforeValue(key)) { return false; }

    char format[8];
    const int written = std::snprintf(format, sizeof(format), "%%.%uf", static_cast<unsigned>(decimals));
    if (written <= 0 || static_cast<size_t>(written) >= sizeof(format)) {
        _invalidate();
        return false;
    }
    return _appendFormatted(format, value);
}

bool json::Writer::complete() const {
    return _valid && _hasRoot && _depth == 0U;
}

size_t json::Writer::length() const {
    return _length;
}

const char* json::Writer::c_str() const {
    return _valid && _buffer != nullptr ? _buffer : nullptr;
}

bool json::Writer::_beforeValue(const char* const key) {
    if (!_valid) { return false; }
    if (_depth == 0U) {
        if ( _hasRoot || key != nullptr) {
            _invalidate();
            return false;
        }
        _hasRoot = true;
        return true;
    }

    Context& context = _contexts[_depth - 1U];
    if (!context.first) {
        if (!_append(',')) { return false; }
    }

    context.first = false;
    if (key == nullptr) {
        _invalidate();
        return false;
    }

    return _appendQuoted(key) && _append(':');
}

bool json::Writer::_push() {
    if (!_valid || _depth >= MAX_DEPTH) {
        _invalidate();
        return false;
    }
    _contexts[_depth].first = true;
    ++_depth;
    return true;
}

bool json::Writer::_endObject() {
    if (!_valid || _depth == 0U) {
        _invalidate();
        return false;
    }

    if (!_append('}'))
        { return false; }
    --_depth;
    return true;
}

bool json::Writer::_append(const char value) {
    return _write(&value, 1U);
}

bool json::Writer::_append(const char* const value) {
    if (value == nullptr) {
        _invalidate();
        return false;
    }

    size_t length = 0U;
    while (value[length] != '\0') { ++length; }
    return _write(value, length);
}

bool json::Writer::_appendFormatted(const char* const format, ...) {
    if (!_valid || format == nullptr) {
        _invalidate();
        return false;
    }

    char formatted[64];
    va_list arguments;

    va_start(arguments, format);
    const int written = std::vsnprintf(formatted, sizeof(formatted), format, arguments);
    va_end(arguments);

    if (written < 0 || static_cast<size_t>(written) >= sizeof(formatted)) {
        _invalidate();
        return false;
    }
    return _write(formatted, static_cast<size_t>(written));
}

bool json::Writer::_appendQuoted(const char* const value) {
    if (value == nullptr || !_append('"')) { return false; }

    constexpr char HEX[] = "0123456789ABCDEF";
    for (size_t index = 0U; value[index] != '\0'; ++index) {
        const uint8_t character = static_cast<uint8_t>(value[index]);
        switch (character) {
            case '"':
                if (!_append("\\\"")) { return false; }
                break;
            case '\\':
                if (!_append("\\\\")) { return false; }
                break;
            case '\b':
                if (!_append("\\b")) { return false; }
                break;
            case '\f':
                if (!_append("\\f")) { return false; }
                break;
            case '\n':
                if (!_append("\\n")) { return false; }
                break;
            case '\r':
                if (!_append("\\r")) { return false; }
                break;
            case '\t':
                if (!_append("\\t")) { return false; }
                break;
            default:
                if (character < 0x20U) {
                    if (!_append("\\u00") || !_append(HEX[(character >> 4U) & 0x0FU]) || !_append(HEX[character & 0x0FU]))
                        { return false; }
                } else if (!_append(static_cast<char>(character)))
                    { return false; }
                break;
        }
    }
    return _append('"');
}

bool json::Writer::_write(const char* const data, const size_t length) {
    if (!_valid || data == nullptr || _buffer == nullptr || _size == 0U) {
        _invalidate();
        return false;
    }

    if (length == 0U) { return true; }
    if (_length + length >= _size) {
        _invalidate();
        return false;
    }

    for (size_t index = 0U; index < length; ++index)
        { _buffer[_length + index] = data[index]; }
    _length += length;
    _buffer[_length] = '\0';
    return true;
}

void json::Writer::_invalidate() {
    _valid = false;
    if (_buffer != nullptr && _size > 0U) {
        const size_t terminator = _length < _size ? _length : _size - 1U;
        _buffer[terminator] = '\0';
    }
}


json::Reader::Reader(const char* const content) : _begin(content), _end(content), _valid(false) {
    if (content == nullptr) { return; }
    while (*_end != '\0')   { ++_end; }

    const char* begin = _begin;
    const char* end   = _end;

    _skipSpaces(begin, end);
    while (end > begin && (end[-1] == ' '  || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
        { --end; }
    if (end - begin < 2 || *begin != '{' || end[-1] != '}')
        { return; }
    _begin = begin;
    _end   = end;
    _valid = true;
}

json::Reader::Reader(const char* const begin, const char* const end) :
    _begin(begin), _end(end),
    _valid(begin != nullptr && end != nullptr && end - begin >= 2 && *begin == '{' && end[-1] == '}')
{}

bool json::Reader::valid() const {
    return _valid;
}

void json::Reader::_skipSpaces(const char*& cursor, const char* const end) {
    while (cursor < end && (*cursor == ' '  || *cursor == '\t' || *cursor == '\r' || *cursor == '\n'))
        { ++cursor; }
}

bool json::Reader::_skipString(const char*& cursor, const char* const end) {
    if (cursor >= end || *cursor != '"') { return false; }

    ++cursor;
    while (cursor < end) {
        if (*cursor == '\\') {
            ++cursor;
            if (cursor >= end)
                { return false; }
            if (*cursor == 'u') {
                if (end - cursor < 5)
                    { return false; }
                for (size_t index = 1U; index <= 4U; ++index) {
                    if (!_isHexDigit(cursor[index]))
                        { return false; }
                }
                cursor += 5;
                continue;
            }

            switch (*cursor) {
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    ++cursor;
                    continue;
                default:
                    return false;
            }
        }

        if (*cursor == '"') {
            ++cursor;
            return true;
        }

        if (static_cast<uint8_t>(*cursor) < 0x20U)
            { return false; }
        ++cursor;
    }
    return false;
}

bool json::Reader::_skipValue(const char*& cursor, const char* const end, const uint8_t depth) {
    _skipSpaces(cursor, end);

    if (cursor >= end || depth > MAX_DEPTH) { return false; }
    if (*cursor == '"') { return _skipString(cursor, end); }
    if (*cursor == 't') { return _skipLiteral(cursor, end, "true"); }
    if (*cursor == 'f') { return _skipLiteral(cursor, end, "false"); }
    if (*cursor == 'n') { return _skipLiteral(cursor, end, "null"); }

    if (*cursor == '-' || _isDigit(*cursor)) { return _skipNumber(cursor, end); }
    if (*cursor == '{') { return _skipObject(cursor, end, depth + 1U); }
    if (*cursor == '[') { return _skipArray(cursor, end, depth + 1U); }

    return false;
}

bool json::Reader::_skipObject(const char*& cursor, const char* const end, const uint8_t depth) {
    if (cursor >= end || *cursor != '{' || depth > MAX_DEPTH) { return false; }

    ++cursor;
    _skipSpaces(cursor, end);

    if (cursor < end && *cursor == '}') {
        ++cursor;
        return true;
    }

    while (cursor < end) {
        if (!_skipString(cursor, end)) { return false; }

        _skipSpaces(cursor, end);
        if (cursor >= end || *cursor != ':') { return false; }

        ++cursor;
        if (!_skipValue(cursor, end, depth)) { return false; }

        _skipSpaces(cursor, end);
        if (cursor >= end) { return false; }
        if (*cursor == '}') {
            ++cursor;
            return true;
        }
        if (*cursor != ',') { return false; }

        ++cursor;
        _skipSpaces(cursor, end);
        if (cursor >= end || *cursor == '}') { return false; }
    }
    return false;
}

bool json::Reader::_skipArray(const char*& cursor, const char* const end, const uint8_t depth) {
    if (cursor >= end || *cursor != '[' || depth > MAX_DEPTH) { return false; }

    ++cursor;
    _skipSpaces(cursor, end);
    if (cursor < end && *cursor == ']') {
        ++cursor;
        return true;
    }

    while (cursor < end) {
        if (!_skipValue(cursor, end, depth)) { return false; }

        _skipSpaces(cursor, end);
        if (cursor >= end) { return false; }
        if (*cursor == ']') {
            ++cursor;
            return true;
        }
        if (*cursor != ',') { return false; }

        ++cursor;
        _skipSpaces(cursor, end);
        if (cursor >= end || *cursor == ']') { return false; }
    }
    return false;
}

bool json::Reader::_isDigit(const char value) {
    return value >= '0' && value <= '9';
}

bool json::Reader::_skipLiteral(const char*& cursor, const char* const end, const char* const literal) {
    if (cursor == nullptr || literal == nullptr) { return false; }

    const char* current = cursor;
    size_t index        = 0U;

    while (literal[index] != '\0') {
        if (current >= end || *current != literal[index])
            { return false; }
        ++current;
        ++index;
    }

    cursor = current;
    return true;
}

bool json::Reader::_skipNumber(const char*& cursor, const char* const end) {
    if (cursor == nullptr || cursor >= end) { return false; }

    const char* current = cursor;
    if (*current == '-') {
        ++current;
        if (current >= end)
            { return false; }
    }

    if (*current == '0') {
        ++current;
        if (current < end && _isDigit(*current))
            { return false; }
    } else {
        if (*current < '1' || *current > '9')
            { return false; }
        do { ++current; }
        while (current < end && _isDigit(*current));
    }

    if (current < end && *current == '.') {
        ++current;
        if (current >= end || !_isDigit(*current))
            { return false; }
        do { ++current; }
        while (current < end && _isDigit(*current));
    }

    if (current < end &&(*current == 'e' || *current == 'E')) {
        ++current;
        if (current < end && (*current == '+' || *current == '-'))
            { ++current; }
        if (current >= end || !_isDigit(*current))
            { return false; }
        do { ++current; }
        while (current < end && _isDigit(*current));
    }

    cursor = current;
    return true;
}

bool json::Reader::_matchString(const char*& cursor, const char* const end, const char* const expected) {
    if (cursor >= end || *cursor != '"' || expected == nullptr) { return false; }

    ++cursor;
    size_t index = 0U;

    while (cursor < end && *cursor != '"') {
        if (*cursor == '\\') { return false; }
        if (expected[index] == '\0' || *cursor != expected[index])
            { return false; }
        ++cursor;
        ++index;
    }

    if (cursor >= end || *cursor != '"' || expected[index] != '\0')
        { return false; }
    ++cursor;
    return true;
}

bool json::Reader::_findValue(const char* const key, const char*& begin, const char*& end) const {
    if (!_valid || key == nullptr || key[0] == '\0') { return false; }

    const char* cursor          = _begin + 1;
    const char* const objectEnd = _end - 1;
    const char* foundBegin = nullptr;
    const char* foundEnd   = nullptr;

    while (cursor < objectEnd) {
        _skipSpaces(cursor, objectEnd);
        if (cursor >= objectEnd) { break; }

        const char* keyCursor = cursor;
        const bool matches    = _matchString(keyCursor, objectEnd, key);

        if (!_skipString(cursor, objectEnd)) { return false; }
        _skipSpaces(cursor, objectEnd);

        if (cursor >= objectEnd || *cursor != ':') { return false; }
        ++cursor;
        _skipSpaces(cursor, objectEnd);

        const char* const valueBegin = cursor;
        if (!_skipValue(cursor, objectEnd, 0U)) { return false; }

        const char* valueEnd = cursor;
        while (valueEnd > valueBegin && (
            valueEnd[-1] == ' '  || valueEnd[-1] == '\t' ||
            valueEnd[-1] == '\r' || valueEnd[-1] == '\n'
        )) { --valueEnd; }

        if (matches) {
            if (foundBegin != nullptr) { return false; }
            foundBegin = valueBegin;
            foundEnd   = valueEnd;
        }

        _skipSpaces(cursor, objectEnd);
        if (cursor >= objectEnd) { break; }
        if (*cursor != ',')      { return false; }

        ++cursor;
        const char* next = cursor;
        _skipSpaces(next, objectEnd);

        if (next >= objectEnd) { return false; }
    }

    if (foundBegin == nullptr) { return false; }
    begin = foundBegin;
    end   = foundEnd;
    return true;
}

bool json::Reader::object(const char* const key, Reader& value) const {
    const char* begin = nullptr;
    const char* end   = nullptr;

    if (!_findValue(key, begin, end)) { return false; }
    Reader objectReader(begin, end);

    if (!objectReader.valid()) { return false; }
    value = objectReader;
    return true;
}

bool json::Reader::_decodeString(const char* const begin, const char* const end, char* const buffer, const size_t size) {
    if (begin == nullptr  || end == nullptr ||
        buffer == nullptr || size == 0U     ||
        end - begin < 2   || *begin != '"'  || end[-1] != '"'
    ) { return false; }

    size_t output                = 0U;
    const char* cursor           = begin + 1;
    const char* const contentEnd = end - 1;
    buffer[0] = '\0';

    while (cursor < contentEnd) {
        uint8_t character = static_cast<uint8_t>(*cursor++);
        if (character == '\\') {
            if (cursor >= contentEnd)
                { return false; }
            const char escaped = *cursor++;

            switch (escaped) {
                case '"':
                    character = '"';
                    break;
                case '\\':
                    character = '\\';
                    break;
                case '/':
                    character = '/';
                    break;
                case 'b':
                    character = '\b';
                    break;
                case 'f':
                    character = '\f';
                    break;
                case 'n':
                    character = '\n';
                    break;
                case 'r':
                    character = '\r';
                    break;
                case 't':
                    character = '\t';
                    break;
                case 'u': {
                    if (contentEnd - cursor < 4) { return false; }
                    for (size_t index = 0U; index < 4U; ++index) {
                        if (!_isHexDigit(cursor[index])) { return false; }
                    }
                    const uint16_t code =
                        static_cast<uint16_t>(_hexValue(cursor[0])) << 12U |
                        static_cast<uint16_t>(_hexValue(cursor[1])) << 8U  |
                        static_cast<uint16_t>(_hexValue(cursor[2])) << 4U  |
                        static_cast<uint16_t>(_hexValue(cursor[3]));
                    cursor += 4;
                    if (code > 0x00FFU) { return false; }
                    character = static_cast<uint8_t>(code);
                    break;
                }
                default:
                    return false;
            }
        } else if (character < 0x20U) { return false; }
        if (output + 1U >= size)      { return false; }
        buffer[output++] = static_cast<char>(character);
    }
    buffer[output] = '\0';
    return true;
}

bool json::Reader::string(const char* const key, char* const buffer, const size_t size) const {
    if (buffer == nullptr || size == 0U) { return false; }

    buffer[0]         = '\0';
    const char* begin = nullptr;
    const char* end   = nullptr;

    if (!_findValue(key, begin, end)) { return false; }
    return _decodeString(begin, end, buffer, size);
}

bool json::Reader::boolean(const char* const key, bool& value) const {
    const char* begin = nullptr;
    const char* end   = nullptr;

    if (!_findValue(key, begin, end)) { return false; }
    if (end - begin == 4 && begin[0] == 't' && begin[1] == 'r' && begin[2] == 'u' && begin[3] == 'e') {
        value = true;
        return true;
    }

    if (end - begin == 5 && begin[0] == 'f' && begin[1] == 'a' && begin[2] == 'l' && begin[3] == 's' && begin[4] == 'e') {
        value = false;
        return true;
    }

    return false;
}

bool json::Reader::unsignedInteger(const char* const key, uint64_t& value) const {
    const char* begin = nullptr;
    const char* end   = nullptr;

    if (!_findValue(key, begin, end)) { return false; }
    const size_t length = static_cast<size_t>(end - begin);

    if (length == 0U || length >= 32U) { return false; }

    for (size_t index = 0U; index < length; ++index)
        { if (begin[index] < '0' || begin[index] > '9') { return false; } }
    char buffer[32];

    for (size_t index = 0U; index < length; ++index)
        { buffer[index] = begin[index]; }
    buffer[length] = '\0';

    errno = 0;
    char* parseEnd = nullptr;
    const unsigned long long parsed = std::strtoull(buffer, &parseEnd, 10);

    if (parseEnd == buffer || *parseEnd != '\0' || errno == ERANGE)
        { return false; }
    value = static_cast<uint64_t>(parsed);
    return true;
}

bool json::Reader::number(const char* const key, double& value) const {
    const char* begin = nullptr;
    const char* end   = nullptr;

    if (!_findValue(key, begin, end)) { return false; }
    const size_t length = static_cast<size_t>(end - begin);

    if (length == 0U || length >= 64U) { return false; }
    char buffer[64];

    for (size_t index = 0U; index < length; ++index)
        { buffer[index] = begin[index]; }
    buffer[length] = '\0';

    errno = 0;
    char* parseEnd = nullptr;
    const double parsed = std::strtod(buffer, &parseEnd);

    if (parseEnd == buffer || *parseEnd != '\0' || errno == ERANGE || !std::isfinite(parsed))
        { return false; }
    value = parsed;
    return true;
}

bool json::Reader::_isHexDigit(const char value) {
    return
        (value >= '0' && value <= '9') ||
        (value >= 'A' && value <= 'F') ||
        (value >= 'a' && value <= 'f');
}

uint8_t json::Reader::_hexValue(const char value) {
    if (value >= '0' && value <= '9') { return static_cast<uint8_t>(value - '0'); }
    if (value >= 'A' && value <= 'F') { return static_cast<uint8_t>(value - 'A' + 10); }
    return static_cast<uint8_t>(value - 'a' + 10);
}
