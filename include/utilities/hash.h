/*
 * include/utilities/hash.h
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

#include <mbedtls/sha256.h>

namespace utilities::hash {
    constexpr size_t SHA256_DIGEST_SIZE = 32;
    constexpr size_t SHA256_TEXT_SIZE   = 65;

    bool isSha256Text(const char* value);
    bool normalizeSha256Text(const char* input, char* output, size_t size);

    class Sha256 final {
        public:
            Sha256();
            ~Sha256();

            Sha256(const Sha256&) = delete;
            Sha256& operator = (const Sha256&) = delete;

            Sha256(Sha256&&) = delete;
            Sha256& operator = (Sha256&&) = delete;

            bool begin();
            bool update(const uint8_t* data, size_t size);
            bool finish(char* output, size_t size);
        private:
            mbedtls_sha256_context _context;
            bool _active = false;
    };
}