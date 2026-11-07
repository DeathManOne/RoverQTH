/*
 * include/screens/menu.h
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

#include <MSP4021.h>

#include "screens/menu/about.h"
#include "screens/menu/battery.h"
#include "screens/menu/displayer.h"
#include "screens/menu/general.h"
#include "screens/menu/item.h"
#include "screens/menu/page.h"
#include "screens/menu/storage.h"
#include "screens/menu/updates.h"
#include "screens/menu/wifi.h"
#include "screens/screen.h"

namespace screens {
    /** @brief Owns the settings pages and coordinates menu navigation. */
    class Menu final : public Screen {
        private:
            menu::Item _currentItem = menu::Item::GENERAL;

            menu::General   _general;
            menu::Displayer _displayer;
            menu::Wifi      _wifi;
            menu::Updates   _updates;
            menu::Storage   _storage;
            menu::Battery   _battery;
            menu::About     _about;

            menu::Page &_pageFromItem(menu::Item item);
            menu::Page &_currentPage();

            const menu::Page &_currentPage() const;
            void select(menu::Item item);
        public:
            /** @brief Restores the menu and its current page to their initial state. */
            void reset();

            /** @copydoc Screen::preload() */
            void preload() override;

            /** @copydoc Screen::draw() */
            void draw(ST7796S::MSP4021 &tft) override;

            /** @copydoc Screen::update() */
            void update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) override;

            /** @copydoc Screen::handleTouch() */
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;

            /** @copydoc Screen::isEditing() */
            bool isEditing() const override;

            /**
             * @brief Consumes the pending display reconfiguration request.
             * @return Pending request before it is cleared, or `Request::NONE`.
             */
            menu::Displayer::Request takeDisplayRequest();
    };
}
