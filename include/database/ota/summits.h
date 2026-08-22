#pragma once

#include <cstddef>
#include <cstdint>

#include "database/ota.h"

namespace database::ota::summits {
    constexpr size_t CODE_SIZE    = database::ota::CODE_SIZE;
    constexpr size_t ETAG_SIZE    = database::ota::ETAG_SIZE;
    constexpr size_t AREA_SIZE    = 64U;
    constexpr size_t VERSION_SIZE = 11U;

    using ProgressCallback = database::ota::ProgressCallback;

    struct Summit {
        char code[CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        int16_t altitude = 0;
        uint8_t points   = 0U;
        uint8_t bonus    = 0U;
    };

    struct Info {
        uint32_t records    = 0U;
        uint64_t sourceSize = 0U;
        char version[VERSION_SIZE] {};
        char etag[database::ota::ETAG_SIZE] {};
    };

    bool info(Info &value);

    bool buildCandidate(
        const char* csvPath,
        const char* version,
        const char* etag,
        uint64_t sourceSize,
        database::ota::ProgressCallback callback = nullptr,
        void* userData = nullptr
    );

    bool installCandidate();
    void discardCandidate();

    bool findNearest(
        double latitude,
        double longitude,
        Summit &summit,
        double &distanceKm,
        double &bearing
    );

    bool findByCode(
        const char* code,
        Summit &summit
    );

    database::ota::SearchStatus findByPrefix(
        double latitude,
        double longitude,
        const char* normalizedPrefix,
        database::ota::SearchResults &results,
        database::ota::CancelCallback cancelCallback = nullptr,
        void* cancelUserData = nullptr
    );

    database::ota::SearchStatus findNearby(
        double latitude,
        double longitude,
        double radiusKm,
        database::ota::SearchResults &results,
        database::ota::CancelCallback cancelCallback = nullptr,
        void* cancelUserData = nullptr
    );
}
