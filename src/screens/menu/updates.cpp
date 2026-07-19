/*
 * src/screens/menu/updates.cpp
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
#include <cstring>

#include "screens/menu/updates.h"
#include "services/update.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

using screens::menu::Updates;
namespace update   = services::update;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;
namespace theme    = ui::settings::themes::defaults;

void Updates::_prepareFirmwareFields() {
    char latestVersion[16] = "";
    char error[48]         = "";

    const char* currentVersion  = update::currentVersion();
    const bool hasLatestVersion = update::getLatestVersion(latestVersion, sizeof(latestVersion));

    switch (update::status()) {
        case update::Status::IDLE:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus("Check", _Action::CHECK_FIRMWARE, theme::CYAN);
            break;
        case update::Status::CHECKING:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus("Checking...", _Action::NONE, theme::GREY);
            break;
        case update::Status::UP_TO_DATE:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus(
                "Up to date", _Action::CHECK_FIRMWARE, theme::GREEN);
            break;
        case update::Status::AVAILABLE:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            _setFirmwareStatus("Download", _Action::DOWNLOAD_FIRMWARE, theme::CYAN);
            break;
        case update::Status::DOWNLOADING:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            std::snprintf(_firmwareStatusValue, sizeof(_firmwareStatusValue), "Downloading %d%%", update::progress());
            _firmwareStatusField.value  = _firmwareStatusValue;
            _firmwareStatusField.action = _Action::NONE;
            _firmwareStatusField.color  = theme::YELLOW;
            break;
        case update::Status::VERIFYING:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            _setFirmwareStatus("Verifying...", _Action::NONE, theme::YELLOW);
            break;
        case update::Status::INSTALLING:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            _setFirmwareStatus("Installing...", _Action::NONE, theme::ORANGE);
            break;
        case update::Status::SUCCESS:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            _setFirmwareStatus("Restarting...", _Action::NONE, theme::GREEN);
            break;
        case update::Status::ERROR:
            _setFirmwareVersion(currentVersion, hasLatestVersion ? latestVersion : nullptr);
            if (!update::getError(error, sizeof(error)))
                { std::snprintf(error, sizeof(error), "%s", "Update failed"); }
            _setFirmwareStatus(error, _Action::CHECK_FIRMWARE, theme::RED);
            break;
        default:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus("Unknown state", _Action::NONE, theme::RED);
            break;
    }
}

void Updates::_updateFirmwareFields(ST7796S::MSP4021 &tft) {
    _prepareFirmwareFields();
    _updateField(tft, _firmwareVersionField);
    _updateField(tft, _firmwareStatusField);
}

void Updates::_setFirmwareStatus(const char* value, _Action action, uint16_t color) {
    if (value == nullptr) { value = ""; }
    std::snprintf(_firmwareStatusValue, sizeof(_firmwareStatusValue), "%s", value);

    _firmwareStatusField.value  = _firmwareStatusValue;
    _firmwareStatusField.action = action;
    _firmwareStatusField.color  = color;
}

void Updates::_setFirmwareVersion(const char* currentVersion, const char* latestVersion) {
    if (currentVersion == nullptr) { currentVersion = ""; }
    if (latestVersion != nullptr && latestVersion[0] != '\0' && std::strcmp(currentVersion, latestVersion) != 0)
        { std::snprintf( _firmwareVersionValue, sizeof(_firmwareVersionValue), "%s -> %s", currentVersion, latestVersion); }
    else { std::snprintf(_firmwareVersionValue, sizeof(_firmwareVersionValue), "%s", currentVersion); }
    _firmwareVersionField.value = _firmwareVersionValue;
}

void Updates::_actionCheckFirmware(ST7796S::MSP4021 &tft) {
    if (update::isBusy()) { return; }
    update::checkUpdate();
    _updateFirmwareFields(tft);
}

void Updates::_actionDownloadFirmware(ST7796S::MSP4021 &tft) {
    if (update::isBusy() || !update::isAvailable()) { return; }
    if (update::startUpdate()) {
        _setFirmwareStatus("Starting...", _Action::NONE, theme::YELLOW);
        _updateField(tft, _firmwareStatusField);
        return;
    }
    _updateFirmwareFields(tft);
}

void Updates::update(ST7796S::MSP4021 &tft) {
    _updateFirmwareFields(tft);
}

void Updates::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap   = uiMockup::GAP;
    const int x     = grid::innerX()       + (gap * 2);
    const int y     = grid::innerY()       + (gap * 2);
    const int w     = grid::innerWidth()   - (gap * 4);
    const int rowH  = 28;

    int rowY = y + rowH + (gap * 3);
    for (Field<_Action>* field : _fields) {
        _makeFieldArea(*field, x, rowY, w, rowH);
        rowY += rowH;
    }

    _prepareFirmwareFields();
    std::snprintf(_sotaVersionValue,     sizeof(_sotaVersionValue),     "%s", "Not installed");
    std::snprintf(_sotaStatusValue,      sizeof(_sotaStatusValue),      "%s", "Not checked");
    std::snprintf(_potaVersionValue,     sizeof(_potaVersionValue),     "%s", "Not installed");
    std::snprintf(_potaStatusValue,      sizeof(_potaStatusValue),      "%s", "Not checked");

    _sotaVersionField.value     = _sotaVersionValue;
    _sotaStatusField.value      = _sotaStatusValue;
    _potaVersionField.value     = _potaVersionValue;
    _potaStatusField.value      = _potaStatusValue;

    _drawTitle(tft, x, y, w, rowH, gap, "updates");
    for (Field<_Action>* field : _fields)
        { _drawLine(tft, *field); }
}

bool Updates::handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
    for (Field<_Action>* field : _fields) {
        if (!_isPressed(*field, x, y)) { continue; }
        switch (field->action) {
            case _Action::CHECK_FIRMWARE:
                _actionCheckFirmware(tft);
                return true;
            case _Action::DOWNLOAD_FIRMWARE:
                _actionDownloadFirmware(tft);
                return true;
            case _Action::CHECK_SOTA:
            case _Action::DOWNLOAD_SOTA:
            case _Action::CHECK_POTA:
            case _Action::DOWNLOAD_POTA:
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}
