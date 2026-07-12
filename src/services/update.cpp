/*
 * src/services/update.cpp
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

#include "services/update.h"

namespace update = services::update;

namespace {
    bool _updating = false;
    int _progress  = 0;
}

bool update::isUpdating() { return _updating; }
int update::progress   () { return _progress; }

bool update::begin() {
    _updating = false;
    _progress = 0;
    return true;
}

bool update::checkUpdate() {
    // TODO
    return false;
}

bool update::startUpdate() {
    // TODO
    _updating = true;
    _progress = 0;
    return false;
}
