#pragma once

#include "database/ota/parks.h"

namespace utilities::ota::parks {
    bool parseCsvRecord(
        const char* line,
        database::ota::parks::Park &park,
        bool &active
    );

    bool selectNearest(
        double latitude,
        double longitude,
        const database::ota::parks::Park &candidate,
        bool &found,
        database::ota::parks::Park &nearest,
        double &distanceKm
    );
}
