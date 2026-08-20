

#pragma once

#include <cstddef>
#include <cstdint>

namespace utilities::json {
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
