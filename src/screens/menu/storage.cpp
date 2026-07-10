/*
 * screens/menu/storage.cpp
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
#include "screens/mockup/grid.h"
#include "services/storage.h"

namespace screens::menu {
    const char* Storage::typeToText(uint8_t type) {
        switch (type) {
            case CARD_MMC:  return "MMC";
            case CARD_SD:   return "SDSC";
            case CARD_SDHC: return "SDHC/SDXC";
            default:        return "Unknown";
        }
    }

    void Storage::formatCapacity(uint64_t bytes, char* buffer, size_t size) {
        constexpr double GB = 1024.0 * 1024.0 * 1024.0;
        std::snprintf(buffer, size, "%.1f GB", static_cast<double>(bytes) / GB);
    }

    void Storage::formatUsage(uint64_t total, uint64_t used, char* buffer, size_t size) {
        const uint64_t percent = total > 0 ? (used * 100ULL) / total : 0;
        std::snprintf(buffer, size, "%llu %%", static_cast<unsigned long long>(percent));
    }

    void Storage::draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field<Action>* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        uint8_t type   = 0;
        uint64_t size  = 0;
        uint64_t total = 0;
        uint64_t used  = 0;

        if (services::storage::readCardInfos(type, size, total, used)) {
            std::snprintf(typeValue, sizeof(typeValue), "%s", typeToText(type));
            formatCapacity(total, capacityValue, sizeof(capacityValue));
            formatUsage(total, used, usageValue, sizeof(usageValue));
        } else {
            std::snprintf(typeValue,        sizeof(typeValue),      "Missing");
            std::snprintf(capacityValue,    sizeof(capacityValue),  "--");
            std::snprintf(usageValue,       sizeof(usageValue),     "--");
        }

        typeField.value     = typeValue;
        capacityField.value = capacityValue;
        usageField.value    = usageValue;
        
        drawTitle(tft, x, y, w, rowH, gap, "storage");
        for (Field<Action>* field : fields)
            { drawLine(tft, *field); }
    }
}
