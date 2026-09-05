/*
 * include/ui/settings/mockup.h
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

namespace ui::settings::mockup {
    constexpr int GAP                       = 4;   /**< Standard spacing between UI elements. */
    constexpr int RADIUS                    = 10;  /**< Standard rounded-corner radius. */
    constexpr int HEADER_HEIGHT             = 36;  /**< Header height in pixels. */
    constexpr int GRID_WIDTH                = 320; /**< Main grid width in pixels. */
    constexpr int RIGHT_ROW_COUNT           = 6;   /**< Number of rows in the right panel. */
    constexpr int GRID_REF_ROW_COUNT        = 7;   /**< Reference row count used by the main grid. */
    constexpr int BUTTON_HEIGHT             = 34;  /**< Persistent button height in pixels. */
    constexpr int STATUS_TOP_BOTTOM_HEIGHT  = 20;  /**< Height of a top or bottom status strip. */
}
