/*
 * include/screens/menu/updates.h
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

#include <cstddef>
#include <cstdint>

#include <MSP4021.h>

#include "screens/menu/page.h"
#include "services/update.h"
#include "ui/settings/themes/defaults.h"

namespace screens::menu {
    /** @brief Coordinates firmware, SOTA and POTA update actions in the menu. */
    class Updates final : public Page {
        public:
            /** @copydoc Page::draw() */
            void draw(ST7796S::MSP4021 &tft) override;

            /** @copydoc Page::update() */
            void update(ST7796S::MSP4021 &tft) override;

            /** @copydoc Page::handleTouch() */
            bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) override;
        private:
            enum class _Action {NONE, CHECK_FIRMWARE, DOWNLOAD_FIRMWARE, CHECK_SOTA, DOWNLOAD_SOTA, CHECK_POTA, DOWNLOAD_POTA};

            char _firmwareVersionValue[(services::update::FIRMWARE_VERSION_SIZE * 2U) + 3U] = "";
            char _firmwareStatusValue[services::update::ERROR_SIZE]                         = "";
            char _sotaVersionValue[(services::update::SOTA_VERSION_SIZE * 2U) + 3U]         = "";
            char _sotaStatusValue[services::update::ERROR_SIZE]                             = "";
            char _potaVersionValue[32]                                                      = "";
            char _potaStatusValue[services::update::ERROR_SIZE]                             = "";

            Field<_Action> _firmwareVersionField = _makeField("Firmware", _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _firmwareStatusField  = _makeField("",         _Action::NONE, ui::settings::themes::defaults::GREY);
            Field<_Action> _sotaVersionField     = _makeField("SOTA",     _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _sotaStatusField      = _makeField("",         _Action::NONE, ui::settings::themes::defaults::GREY);
            Field<_Action> _potaVersionField     = _makeField("POTA",     _Action::NONE, ui::settings::themes::defaults::WHITE);
            Field<_Action> _potaStatusField      = _makeField("",         _Action::NONE, ui::settings::themes::defaults::GREY);

            Field<_Action>* _fields[6] = {
                &_firmwareVersionField,
                &_firmwareStatusField,
                &_sotaVersionField,
                &_sotaStatusField,
                &_potaVersionField,
                &_potaStatusField
            };

            void _actionCheckFirmware(ST7796S::MSP4021 &tft);
            void _actionCheckSota(ST7796S::MSP4021 &tft);
            void _actionCheckPota(ST7796S::MSP4021 &tft);

            void _prepareFirmwareFields();
            void _prepareSotaFields();
            void _preparePotaFields();

            void _updateFirmwareFields(ST7796S::MSP4021 &tft);
            void _updatePotaFields(ST7796S::MSP4021 &tft);
            void _updateSotaFields(ST7796S::MSP4021 &tft);

            void _setProgressStatus(Field<_Action> &field, char* buffer, size_t size, const char* label, uint8_t progress, uint16_t color);
            void _setFirmwareVersion(const char* currentVersion, const char* latestVersion = nullptr);
            void _setSotaVersion(const char* installedVersion, const char* latestVersion = nullptr);

            void _setFirmwareStatus(const char* value, _Action action, uint16_t color);
            void _setSotaStatus(const char* value, _Action action, uint16_t color);
            void _setPotaStatus(const char* value, _Action action, uint16_t color);

            void _setPotaRecords(uint32_t records);

            void _actionDownloadFirmware(ST7796S::MSP4021 &tft);
            void _actionDownloadSota(ST7796S::MSP4021 &tft);
            void _actionDownloadPota(ST7796S::MSP4021 &tft);
    };
}
