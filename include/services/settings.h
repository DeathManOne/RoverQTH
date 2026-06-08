/*
 * services/settings.h
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

namespace services::settings {
    enum class Theme {DEFAULTS, NIGHT, HIGHS};
    enum class Units {METRIC, IMPERIAL};

    bool begin();

    bool getCallsign(char* buffer, unsigned int size);
    bool setCallsign(const char* callsign);
    bool resetCallsign();

    Theme theme();
    bool setTheme(Theme theme);
    bool resetTheme();

    Units units();
    bool setUnits(Units units);
    bool resetUnits();

    bool resetAll();
}