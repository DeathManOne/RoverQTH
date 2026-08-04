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

    /**
     * Checks whether a string is a valid SHA-256 hash.
     * @param value SHA-256 hash as a null-terminated hexadecimal string.
     * @return true if the string is a valid SHA-256 hash, false otherwise.
     */
    bool isSha256Text(const char* value);

    /**
     * Normalizes a SHA-256 hash string to lowercase.
     * @param input  Source SHA-256 hash.
     * @param output Destination buffer.
     * @param size   Size of the destination buffer in bytes.
     * @return true if the normalized hash was successfully written, false otherwise.
     */
    bool normalizeSha256Text(const char* input, char* output, size_t size);

    class Sha256 final {
        public:
            /**
             * Creates a new SHA-256 calculator.
             */
            Sha256();

            /**
             * Releases the SHA-256 calculator.
             */
            ~Sha256();

            /**
             * Copy constructor is disabled.
             */
            Sha256(const Sha256&) = delete;

            /**
             * Copy assignment operator is disabled.
             */
            Sha256& operator = (const Sha256&) = delete;

            /**
             * Move constructor is disabled.
             */
            Sha256(Sha256&&) = delete;

            /**
             * Move assignment operator is disabled.
             */
            Sha256& operator = (Sha256&&) = delete;

            /**
             * Starts a new incremental SHA-256 computation.
             * @return true if the context was successfully initialized, false otherwise.
             */
            bool begin();

            /**
             * Adds data to the current SHA-256 computation.
             * @param data Pointer to the input data.
             * @param size Size of the input data in bytes.
             * @return true if the data was successfully processed, false otherwise.
             */
            bool update(const uint8_t* data, size_t size);

            /**
             * Finalizes the current SHA-256 computation.
             * @param output Destination buffer.
             * @param length Size of the destination buffer in bytes.
             * @return true if the hash was successfully written, false otherwise.
             */
            bool finish(char* output, size_t size);
        private:
            mbedtls_sha256_context _context;
            bool _active = false;
    };
}
