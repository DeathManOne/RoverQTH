/*
 * screens/menu/storage.h
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
#include "screens/menu/page.h"

namespace screens::menu {
    class Storage final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class Action {NONE};

            char typeValue[16]      = "";
            char capacityValue[16]  = "";
            char usageValue[16]     = "";

            Field<Action> typeField     = makeField("Type",     Action::NONE,   ui::settings::themes::defaults::WHITE);
            Field<Action> capacityField = makeField("Capacity", Action::NONE,   ui::settings::themes::defaults::WHITE);
            Field<Action> usageField    = makeField("Usage",    Action::NONE,   ui::settings::themes::defaults::WHITE);
            
            Field<Action>* fields[3] = {
                &typeField,
                &capacityField,
                &usageField
            };

            static const char* typeToText(uint8_t type);
            static void formatCapacity(uint64_t bytes, char* buffer, size_t size);
            static void formatUsage(uint64_t total, uint64_t used, char* buffer, size_t size);
    };
}