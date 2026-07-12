/*
 * include/ui/settings/gps.h
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
#include <cstdint>

namespace ui::settings::gps {
    constexpr double MIN_HEADING_SPEED_KMH  = 2.0;
    constexpr double HDOP_EXCELLENT         = 1.0;
    constexpr double HDOP_GOOD              = 2.0;
    constexpr double HDOP_FAIR              = 5.0;
    constexpr double HDOP_POOR              = 10.0;
    constexpr int SAT_MIN_FIX               = 4;
    constexpr int SAT_GOOD                  = 7;
    constexpr int SAT_EXCELLENT             = 10;
}
