/*
 * src/core/power.cpp
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

#include <esp_sleep.h>
#include "core/power.h"
#include "services/battery.h"

namespace power   = core::power;
namespace battery = services::battery;

bool power::shouldShutdown() { return battery::isCritical(); }

void power::shutdown() {
    // futur :
    // storage save
    // flush logs
    // etc

    esp_deep_sleep_start();
}