

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
}
