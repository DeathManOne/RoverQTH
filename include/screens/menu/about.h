/*
 * screens/menu/about.h
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
    class About final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class Action {NONE};
            Field<Action> nameField     = makeField("Name",     Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> versionField  = makeField("Version",  Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> authorField   = makeField("Author",   Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> licenseField  = makeField("License",  Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<Action> linkField     = makeField("Link",     Action::NONE, ui::settings::themes::defaults::WHITE);

            Field<Action>* fields[5] = {
                &nameField,
                &versionField,
                &authorField,
                &licenseField,
                &linkField
            };
    };
}