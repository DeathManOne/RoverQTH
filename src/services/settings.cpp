/*
 * services/settings.cpp
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

#include "services/settings.h"
#include "services/nvs.h"

namespace services::settings {
    namespace {
        bool ready = false;

        services::nvs::Theme toNvsTheme(Theme theme) {
            switch (theme) {
                case Theme::DEFAULTS:
                    return services::nvs::Theme::DEFAULTS;
                case Theme::NIGHT:
                    return services::nvs::Theme::NIGHT;
                case Theme::HIGHS:
                    return services::nvs::Theme::HIGHS;
            }
            return services::nvs::Theme::DEFAULTS;
        }

        Theme fromNvsTheme(services::nvs::Theme theme) {
            switch (theme) {
                case services::nvs::Theme::DEFAULTS:
                    return Theme::DEFAULTS;
                case services::nvs::Theme::NIGHT:
                    return Theme::NIGHT;
                case services::nvs::Theme::HIGHS:
                    return Theme::HIGHS;
            }
            return Theme::DEFAULTS;
        }

        services::nvs::Units toNvsUnits(Units units) {
            switch (units) {
                case Units::METRIC:
                    return services::nvs::Units::METRIC;
                case Units::IMPERIAL:
                    return services::nvs::Units::IMPERIAL;
            }
            return services::nvs::Units::METRIC;
        }

        Units fromNvsUnits(services::nvs::Units units) {
            switch (units) {
                case services::nvs::Units::METRIC:
                    return Units::METRIC;
                case services::nvs::Units::IMPERIAL:
                    return Units::IMPERIAL;
            }

            return Units::METRIC;
        }
    }

    bool begin() {
        if (ready)
            { return true; }
        ready = services::nvs::begin();
        return ready;
    }

    bool getCallsign(char* buffer, unsigned int size) {
        if (!begin())
            { return false; }
        return services::nvs::getCallsign(buffer, size);
    }

    bool setCallsign(const char* callsign) {
        if (!begin())
            { return false; }
        return services::nvs::setCallsign(callsign);
    }

    bool resetCallsign() {
        if (!begin())
            { return false; }
        return services::nvs::resetCallsign();
    }

    Theme theme() {
        if (!begin())
            { return Theme::DEFAULTS; }
        return fromNvsTheme(services::nvs::getTheme());
    }

    bool setTheme(Theme theme) {
        if (!begin())
            { return false; }
        return services::nvs::setTheme(toNvsTheme(theme));
    }

    bool resetTheme() {
        if (!begin())
            { return false; }
        return services::nvs::resetTheme();
    }

    Units units() {
        if (!begin())
            { return Units::METRIC; }
        return fromNvsUnits(services::nvs::getUnits());
    }

    bool setUnits(Units units) {
        if (!begin())
            { return false; }
        return services::nvs::setUnits(toNvsUnits(units));
    }

    bool resetUnits() {
        if (!begin())
            { return false; }
        return services::nvs::resetUnits();
    }

    bool resetAll() {
        if (!begin())
            { return false; }
        bool ok = true;
        ok = services::nvs::resetCallsign() && ok;
        ok = services::nvs::resetTheme() && ok;
        ok = services::nvs::resetUnits() && ok;
        return ok;
    }
}