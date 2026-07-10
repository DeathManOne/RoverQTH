/*
 * screens/menu/updates.h
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
    class Updates final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class Action {NONE};

            char sotaVersionValue[16] = "";
            char sotaUpdateValue[16]  = "";
            char potaVersionValue[16] = "";
            char potaUpdateValue[16]  = "";

            Field<Action> sotaVersionField = makeField("SOTA version", Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> sotaUpdateField  = makeField("SOTA update",  Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> potaVersionField = makeField("POTA version", Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> potaUpdateField  = makeField("POTA update",  Action::NONE, ui::settings::themes::defaults::WHITE);

            Field<Action>* fields[4] = {
                &sotaVersionField,
                &sotaUpdateField,
                &potaVersionField,
                &potaUpdateField
            };
    };
}
