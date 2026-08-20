#include <cmath>

#include "database/ota.h"

namespace ota = database::ota;

namespace {
    bool _validCode(const char* const code) {
        if (code == nullptr || code[0] == '\0') {
            return false;
        }

        for (size_t index = 0U; index < ota::CODE_SIZE; ++index) {
            if (code[index] == '\0') {
                return true;
            }
        }

        return false;
    }
}

void ota::clear(SearchResults &results) {
    results = {};
}

bool ota::retainNearest(
    SearchResults &results,
    const SearchResult &candidate
) {
    if (results.count > RESULT_CAPACITY) {
        return false;
    }

    if (
        !_validCode(candidate.code) ||
        !std::isfinite(candidate.distanceKm) ||
        candidate.distanceKm < 0.0
    ) {
        return false;
    }

    size_t insertionIndex = 0U;

    while (
        insertionIndex < results.count &&
        results.items[insertionIndex].distanceKm <= candidate.distanceKm
    ) {
        ++insertionIndex;
    }

    if (
        results.count == RESULT_CAPACITY &&
        insertionIndex == RESULT_CAPACITY
    ) {
        return false;
    }

    const size_t lastIndex =
        results.count < RESULT_CAPACITY
            ? results.count
            : RESULT_CAPACITY - 1U;

    for (
        size_t index = lastIndex;
        index > insertionIndex;
        --index
    ) {
        results.items[index] = results.items[index - 1U];
    }

    results.items[insertionIndex] = candidate;

    if (results.count < RESULT_CAPACITY) {
        ++results.count;
    }

    return true;
}
