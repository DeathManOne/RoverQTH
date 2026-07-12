/*
 * include/screens/menu/about.h
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
    class About final : public Page {
        public:
            void draw(ST7796S::MSP4021 &tft) override;
        private:
            enum class _Action {NONE};
            Field<_Action> _nameField     = _makeField("Name",     _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _versionField  = _makeField("Version",  _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _authorField   = _makeField("Author",   _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _licenseField  = _makeField("License",  _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _linkField     = _makeField("Link",     _Action::NONE, ui::settings::themes::defaults::WHITE);

            Field<_Action>* _fields[5] = {
                &_nameField,
                &_versionField,
                &_authorField,
                &_licenseField,
                &_linkField
            };
    };
}