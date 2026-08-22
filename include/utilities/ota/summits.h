#pragma once

#include <cstddef>

#include "database/ota/summits.h"

namespace utilities::ota::summits {
    bool parseListVersion(
        const char* line,
        char* version,
        size_t size
    );

    bool parseCsvRecord(
        const char* line,
        database::ota::summits::Summit &summit
    );

    bool isVersionValid(const char* version);

    bool isVersionNewer(
        const char* candidate,
        const char* installed
    );

    bool selectNearest(
        double latitude,
        double longitude,
        const database::ota::summits::Summit &candidate,
        bool &found,
        database::ota::summits::Summit &nearest,
        double &distanceKm
    );
}
