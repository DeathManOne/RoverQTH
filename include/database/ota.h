

#pragma once

#include <cstddef>
#include <cstdint>

namespace database::ota {
    constexpr size_t CODE_SIZE       = 16U;
    constexpr size_t ETAG_SIZE       = 96U;
    constexpr size_t RESULT_CAPACITY = 64U;

    enum class SearchStatus : uint8_t {
        SUCCESS,
        CANCELLED,
        ERROR
    };

    struct SearchResult {
        char code[CODE_SIZE] {};
        double distanceKm = 0.0;
        uint8_t points    = 0U;
        uint8_t bonus     = 0U;
    };

    struct SearchResults {
        SearchResult items[RESULT_CAPACITY] {};
        size_t count = 0U;
    };

    using ProgressCallback =
        void (*)(uint8_t progress, void* userData);

    using CancelCallback =
        bool (*)(void* userData);

    void clear(SearchResults &results);

    bool retainNearest(
        SearchResults &results,
        const SearchResult &candidate
    );
}
