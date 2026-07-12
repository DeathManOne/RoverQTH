/*
 * include/screens/menu/storage.h
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

#include <MSP4021.h>
#include "screens/menu/page.h"

namespace screens::menu {
    class Storage final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class _Action {NONE};

            char _typeValue[16]      = "";
            char _capacityValue[16]  = "";
            char _usageValue[16]     = "";

            Field<_Action> _typeField     = _makeField("Type",     _Action::NONE,   ui::settings::themes::defaults::WHITE);
            Field<_Action> _capacityField = _makeField("Capacity", _Action::NONE,   ui::settings::themes::defaults::WHITE);
            Field<_Action> _usageField    = _makeField("Usage",    _Action::NONE,   ui::settings::themes::defaults::WHITE);
            
            Field<_Action>* _fields[3] = {
                &_typeField,
                &_capacityField,
                &_usageField
            };

            static const char* _typeToText(uint8_t type);
            static void _formatCapacity(uint64_t bytes, char* buffer, size_t size);
            static void _formatUsage(uint64_t total, uint64_t used, char* buffer, size_t size);
    };
}