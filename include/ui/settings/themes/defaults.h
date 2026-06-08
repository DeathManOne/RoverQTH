/*
 * ui/settings/themes/defaults.h
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

namespace ui::settings::themes {
    namespace defaults {
        constexpr uint16_t BORDER = 0x3AEB;
        constexpr uint16_t NAV    = 0x33FF;
        constexpr uint16_t WHITE  = 0xFFFF;
        constexpr uint16_t CYAN   = 0x05BF;
        constexpr uint16_t GREEN  = 0x07E0;
        constexpr uint16_t ORANGE = 0xFD20;
        constexpr uint16_t YELLOW = 0xFFE0;
        constexpr uint16_t RED    = 0xF800;
        constexpr uint16_t GREY   = 0xBDF7;
        constexpr uint16_t BLACK  = 0x0000;
    }
}
