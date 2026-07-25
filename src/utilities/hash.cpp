/*
 * src/utilities/hash.cpp
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

#include <cctype>
#include <cstring>

#include "utilities/hash.h"

namespace hash = utilities::hash;

namespace {
    constexpr char HEX_CHARACTERS[] = "0123456789abcdef";

    bool _digestToText(const uint8_t* const digest, char* const output, const size_t size) {
        if (digest == nullptr || output == nullptr || size < hash::SHA256_TEXT_SIZE) { return false; }

        for (size_t index = 0; index < hash::SHA256_DIGEST_SIZE; ++index) {
            const uint8_t value = digest[index];

            output[index * 2]     = HEX_CHARACTERS[static_cast<size_t>(value >> 4)];
            output[index * 2 + 1] = HEX_CHARACTERS[static_cast<size_t>(value & 0x0FU)];
        }

        output[hash::SHA256_TEXT_SIZE - 1] = '\0';
        return true;
    }
}

bool hash::isSha256Text(const char* const value) {
    if (value == nullptr || std::strlen(value) != SHA256_TEXT_SIZE - 1) { return false; }

    for (size_t index = 0; index < SHA256_TEXT_SIZE - 1; ++index) {
        const unsigned char character = static_cast<unsigned char>(value[index]);
        if (!std::isxdigit(character)) { return false; }
    }
    return true;
}

bool hash::normalizeSha256Text(const char* const input, char* const output, const size_t size) {
    if (!isSha256Text(input) || output == nullptr || size < SHA256_TEXT_SIZE) { return false; }

    for (size_t index = 0; index < SHA256_TEXT_SIZE - 1; ++index)
        { output[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(input[index]))); }
    output[SHA256_TEXT_SIZE - 1] = '\0';
    return true;
}

hash::Sha256::Sha256() {
    mbedtls_sha256_init(&_context);
}

hash::Sha256::~Sha256() {
    mbedtls_sha256_free(&_context);
}

bool hash::Sha256::begin() {
    _active = mbedtls_sha256_starts_ret(&_context, 0) == 0;
    return _active;
}

bool hash::Sha256::update(const uint8_t* const data, const size_t size) {
    if (!_active || (data == nullptr && size > 0)) { return false; }
    if (size == 0) { return true; }

    if (mbedtls_sha256_update_ret(&_context, data, size) != 0) {
        _active = false;
        return false;
    }
    return true;
}

bool hash::Sha256::finish(char* const output, const size_t size) {
    if (!_active || output == nullptr || size < SHA256_TEXT_SIZE) { return false; }

    uint8_t digest[SHA256_DIGEST_SIZE];
    const bool success = mbedtls_sha256_finish_ret(&_context, digest) == 0;

    _active = false;
    if (!success) {
        output[0] = '\0';
        return false;
    }
    return _digestToText(digest, output, size);
}