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

#include <cstdio>
//#include <cstring>
#include "database/nvs.h"
#include "services/settings.h"

namespace services::settings {
    namespace {
        bool ready = false;
    }

    bool begin() {
        if (ready)
            { return true; }
        ready = database::nvs::begin();
        return ready;
    }

    bool getCallsign(char* buffer, unsigned int size) {
        if (!begin())
            { return false; }
        return database::nvs::getCallsign(buffer, size);
    }

    bool setCallsign(const char* callsign) {
        if (!begin())
            { return false; }
        return database::nvs::setCallsign(callsign);
    }

    bool resetCallsign() {
        if (!begin())
            { return false; }
        return database::nvs::resetCallsign();
    }

    CallsignSuffix getCallsignSuffix() {
        if (!begin())
            { return CallsignSuffix::NONE; }
        uint8_t value = database::nvs::getCallsignSuffix();
        switch (static_cast<CallsignSuffix>(value)) {
            case CallsignSuffix::NONE:
            case CallsignSuffix::P:
            case CallsignSuffix::M:
            case CallsignSuffix::MM:
            case CallsignSuffix::AM:
                return static_cast<CallsignSuffix>(value);
            default:
                return CallsignSuffix::NONE;
        }
    }

    bool setCallsignSuffix(CallsignSuffix callsignSuffix) {
        if (!begin())
            { return false; }
        return database::nvs::setCallsignSuffix(static_cast<uint8_t>(callsignSuffix));
    }

    bool resetCallsignSuffix() {
        if (!begin())
            { return false; }
        return database::nvs::resetCallsignSuffix();
    }

    bool getFullCallsign(char* buffer, unsigned int size) {
        if (buffer == nullptr || size == 0)
            { return false; }
        char callsign[32];
        if (!services::settings::getCallsign(callsign, sizeof(callsign)))
            { return false; }
        const auto suffix = services::settings::getCallsignSuffix();
        const char* suffixText = "";

        switch (suffix) {
            case services::settings::CallsignSuffix::NONE:
                suffixText = "";
                break;
            case services::settings::CallsignSuffix::P:
                suffixText = "/P";
                break;
            case services::settings::CallsignSuffix::M:
                suffixText = "/M";
                break;
            case services::settings::CallsignSuffix::MM:
                suffixText = "/MM";
                break;
            case services::settings::CallsignSuffix::AM:
                suffixText = "/AM";
                break;
        }
        snprintf(buffer, size, "%s%s", callsign, suffixText);
        return true;
    }

    Theme getTheme() {
        if (!begin())
            { return Theme::DEFAULTS; }
        uint8_t value = database::nvs::getTheme();
        switch (static_cast<Theme>(value)) {
            case Theme::DEFAULTS:
            case Theme::NIGHT:
            case Theme::HIGHS:
                return static_cast<Theme>(value);
            default:
                return Theme::DEFAULTS;
        }
    }

    bool setTheme(Theme theme) {
        if (!begin())
            { return false; }
        return database::nvs::setTheme(static_cast<uint8_t>(theme));
    }

    bool resetTheme() {
        if (!begin())
            { return false; }
        return database::nvs::resetTheme();
    }

    Units getUnits() {
        if (!begin())
            { return Units::METRIC; }
        uint8_t value = database::nvs::getUnits();
        switch (static_cast<Units>(value)) {
            case Units::METRIC:
            case Units::IMPERIAL:
                return static_cast<Units>(value);
            default:
                return Units::METRIC;
        }
    }

    bool setUnits(Units units) {
        if (!begin())
            { return false; }
        return database::nvs::setUnits(static_cast<uint8_t>(units));
    }

    bool resetUnits() {
        if (!begin())
            { return false; }
        return database::nvs::resetUnits();
    }

    bool resetAll() {
        if (!begin())
            { return false; }
        bool ok = true;
        ok = database::nvs::resetCallsign() && ok;
        ok = database::nvs::resetCallsignSuffix() && ok;
        ok = database::nvs::resetTheme() && ok;
        ok = database::nvs::resetUnits() && ok;
        return ok;
    }
}