/*
 * src/screens/menu/storage.cpp
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
#include "screens/menu/storage.h"
#include "services/storage.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"

using screens::menu::Storage;
namespace storage  = services::storage;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;

const char* Storage::_typeToText(uint8_t type) {
    switch (type) {
        case CARD_MMC:  return "MMC";
        case CARD_SD:   return "SDSC";
        case CARD_SDHC: return "SDHC/SDXC";
        default:        return "Unknown";
    }
}

void Storage::_formatCapacity(uint64_t bytes, char* buffer, size_t size) {
    constexpr double GB = 1024.0 * 1024.0 * 1024.0;
    std::snprintf(buffer, size, "%.1f GB", static_cast<double>(bytes) / GB);
}

void Storage::_formatUsage(uint64_t total, uint64_t used, char* buffer, size_t size) {
    const uint64_t percent = total > 0 ? (used * 100ULL) / total : 0;
    std::snprintf(buffer, size, "%llu %%", static_cast<unsigned long long>(percent));
}

void Storage::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()     + (gap * 2);
    const int y     = grid::innerY()     + (gap * 2);
    const int w     = grid::innerWidth() - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    uint8_t type   = 0;
    uint64_t size  = 0;
    uint64_t total = 0;
    uint64_t used  = 0;

    if (storage::readCardInfos(type, size, total, used)) {
        std::snprintf(_typeValue, sizeof(_typeValue), "%s", _typeToText(type));
        _formatCapacity(total, _capacityValue, sizeof(_capacityValue));
        _formatUsage(total, used, _usageValue, sizeof(_usageValue));
    } else {
        std::snprintf(_typeValue,     sizeof(_typeValue),      "Missing");
        std::snprintf(_capacityValue, sizeof(_capacityValue),  "--");
        std::snprintf(_usageValue,    sizeof(_usageValue),     "--");
    }

    _typeField.value     = _typeValue;
    _capacityField.value = _capacityValue;
    _usageField.value    = _usageValue;
    
    _drawTitle(tft, x, y, w, rowH, gap, "storage");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}
