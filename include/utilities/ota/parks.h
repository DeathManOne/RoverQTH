/*
 * include/utilities/ota/parks.h
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

#include "database/ota/parks.h"

namespace utilities::ota::parks {
    /**
     * @brief Converts one POTA CSV record into its binary park representation.
     * @param line Null-terminated CSV record to parse.
     * @param park Receives the parsed park when the record is active and usable.
     * @param active Receives whether the source park is active and usable.
     * @return `true` when the CSV record was parsed successfully.
     */
    bool parseCsvRecord(const char* line, database::ota::parks::Park &park, bool &active);

    /**
     * @brief Updates the nearest-park selection with a valid candidate.
     * @param latitude Reference latitude in decimal degrees.
     * @param longitude Reference longitude in decimal degrees.
     * @param candidate Park candidate to evaluate.
     * @param found Indicates whether a previous nearest park exists and is updated when selected.
     * @param nearest Receives the candidate when it becomes the nearest park.
     * @param distanceKm Current nearest distance, updated in kilometres when the candidate is closer.
     * @return `true` when the candidate distance was evaluated successfully.
     */
    bool selectNearest(
        double latitude,
        double longitude,
        const database::ota::parks::Park &candidate,
        bool &found,
        database::ota::parks::Park &nearest,
        double &distanceKm
    );
}
