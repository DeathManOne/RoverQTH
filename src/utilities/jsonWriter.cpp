/*
 * src/utilities/jsonWriter.cpp
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

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include "utilities/json.h"

using utilities::json::Writer;

bool Writer::_beforeValue(const char* const key) {
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

bool Writer::_push() {
    if (!_valid || _depth >= MAX_DEPTH) {
        _invalidate();
        return false;
    }
    _contexts[_depth].first = true;
    ++_depth;
    return true;
}

bool Writer::_append(const char value) {
    return _write(&value, 1U);
}

bool Writer::_append(const char* const value) {
    if (value == nullptr) {
        _invalidate();
        return false;
    }

    size_t length = 0U;
    while (value[length] != '\0') { ++length; }
    return _write(value, length);
}

bool Writer::_appendFormatted(const char* const format, ...) {
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

bool Writer::_appendQuoted(const char* const value) {
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

bool Writer::_write(const char* const data, const size_t length) {
    if (!_valid || data == nullptr || _buffer == nullptr || _size == 0U || _length >= _size ) {
        _invalidate();
        return false;
    }

    if (length == 0U) {return true; }
    const size_t available = _size - _length - 1U;

    if (length > available) {
        _invalidate();
        return false;
    }

    for (size_t index = 0U; index < length; ++index)
        { _buffer[_length + index] = data[index]; }
    _length += length;
    _buffer[_length] = '\0';
    return true;
}

void Writer::_invalidate() {
    _valid = false;
    if (_buffer != nullptr && _size > 0U) {
        const size_t terminator = _length < _size ? _length : _size - 1U;
        _buffer[terminator] = '\0';
    }
}

Writer::Writer(char* const buffer, const size_t size) :
    _buffer(buffer), _valid(buffer != nullptr && size > 0U),
    _hasRoot(false), _size(size), _length(0U), _depth(0U), _contexts {}
{ if (_valid) { _buffer[0] = '\0'; } }

bool Writer::beginObject(const char* const key) {
    if (!_valid)
        { return false; }
    if (_depth >= MAX_DEPTH) {
        _invalidate();
        return false;
    }
    if (!_beforeValue(key) || !_append('{'))
        { return false; }
    return _push();
}

bool Writer::endObject() {
    if (!_valid || _depth == 0U) {
        _invalidate();
        return false;
    }
    if (!_append('}'))
        { return false; }
    --_depth;
    return true;
}

bool Writer::string(const char* const key, const char* const value) {
    if (!_valid)
        { return false; }
    if (value == nullptr) {
        _invalidate();
        return false;
    }
    if (!_beforeValue(key))
        { return false; }
    return _appendQuoted(value);
}

bool Writer::boolean(const char* const key, const bool value) {
    if (!_beforeValue(key)) { return false; }
    return _append(value ? "true" : "false");
}

bool Writer::unsignedInteger(const char* const key, const uint64_t value) {
    if (!_beforeValue(key)) { return false; }
    return _appendFormatted("%llu", static_cast<unsigned long long>(value));
}

bool Writer::number(const char* const key, const double value, const uint8_t decimals) {
    if (!_valid)
        { return false; }
    if (!std::isfinite(value) || decimals > 9U) {
        _invalidate();
        return false;
    }

    if (!_beforeValue(key))
        { return false; }
    char format[8];

    const int written = std::snprintf(format, sizeof(format), "%%.%uf", static_cast<unsigned>(decimals));
    if (written <= 0 || static_cast<size_t>(written) >= sizeof(format)) {
        _invalidate();
        return false;
    }
    return _appendFormatted(format, value);
}

bool Writer::complete() const {
    return _valid && _hasRoot && _depth == 0U;
}

size_t Writer::length() const {
    return complete() ? _length : 0U;
}

const char* Writer::c_str() const {
    return complete() && _buffer != nullptr ? _buffer : nullptr;
}
