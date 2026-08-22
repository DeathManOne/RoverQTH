#pragma once

#include <cstddef>
#include <cstdint>

#include "database/ota.h"

namespace services::ota {
    constexpr size_t AREA_SIZE = 96U;

    enum class Type : uint8_t {
        SUMMITS,
        PARKS
    };

    enum class Status : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        ERROR
    };

    enum class NearbyStatus : uint8_t {
        UNAVAILABLE,
        IDLE,
        SEARCHING,
        READY,
        EMPTY,
        ERROR
    };

    struct Reference {
        char code[database::ota::CODE_SIZE] {};
        char area[AREA_SIZE] {};
        double latitude  = 0.0;
        double longitude = 0.0;
        int16_t altitude = 0;
        uint8_t points   = 0U;
        uint8_t bonus    = 0U;
    };

    struct Snapshot {
        Status status = Status::UNAVAILABLE;
        Type type     = Type::SUMMITS;
        Reference reference {};
        double distanceKm = 0.0;
        double bearingDeg = 0.0;
    };

    struct NearbySnapshot {
        NearbyStatus status = NearbyStatus::UNAVAILABLE;
        size_t count        = 0U;
    };

    void begin();
    void invalidate(Type type);

    bool requestNearest(
        Type type,
        double latitude,
        double longitude
    );

    bool requestByCode(
        Type type,
        const char* code,
        double latitude,
        double longitude
    );

    bool requestByPrefix(
        Type type,
        const char* prefix,
        double latitude,
        double longitude
    );

    bool requestNearby(
        Type type,
        double latitude,
        double longitude,
        double radiusKm
    );

    bool cancel();

    Snapshot snapshot(Type type);
    NearbySnapshot nearbySnapshot(Type type);

    bool nearbyResult(
        Type type,
        size_t index,
        database::ota::SearchResult &result
    );

    bool isBusy();
}
