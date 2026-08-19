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

#include "screens/menu/updates.h"
#include "services/update.h"
#include "ui/mockup/grid.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"
#include "utilities/text.h"

using screens::menu::Updates;
namespace update   = services::update;
namespace grid     = ui::mockup::grid;
namespace uiMockup = ui::settings::mockup;
namespace theme    = ui::settings::themes::defaults;
namespace text     = utilities::text;

void Updates::_prepareFirmwareFields() {
    const update::FirmwareSnapshot snapshot  = update::firmwareSnapshot();
    const char* const currentVersion = update::firmwareVersion();
    const char* const latestVersion  = snapshot.latestVersion[0] != '\0' ? snapshot.latestVersion : nullptr;

    switch (snapshot.status) {
        case update::Status::IDLE:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus("Check", _Action::CHECK_FIRMWARE, theme::CYAN);
            break;
        case update::Status::CHECKING:
            _setFirmwareVersion(currentVersion);
            std::snprintf(
                _firmwareStatusValue, sizeof(_firmwareStatusValue),
                "Checking %u%%",      static_cast<unsigned int>(snapshot.progress)
            );
            _firmwareStatusField.value  = _firmwareStatusValue;
            _firmwareStatusField.action = _Action::NONE;
            _firmwareStatusField.color  = theme::GREY;
            break;
        case update::Status::UP_TO_DATE:
            _setFirmwareVersion(currentVersion);
            _setFirmwareStatus("Up to date", _Action::CHECK_FIRMWARE, theme::GREEN);
            break;
        case update::Status::AVAILABLE:
            _setFirmwareVersion(currentVersion, latestVersion);
            _setFirmwareStatus("Download", _Action::DOWNLOAD_FIRMWARE, theme::CYAN);
            break;
        case update::Status::DOWNLOADING:
            _setFirmwareVersion(currentVersion, latestVersion);
            std::snprintf(
                _firmwareStatusValue, sizeof(_firmwareStatusValue),
                "Downloading %u%%",   static_cast<unsigned int>(snapshot.progress)
            );
            _firmwareStatusField.value  = _firmwareStatusValue;
            _firmwareStatusField.action = _Action::NONE;
            _firmwareStatusField.color  = theme::YELLOW;
            break;
        case update::Status::VERIFYING:
            _setFirmwareVersion(currentVersion, latestVersion);
            _setFirmwareStatus("Verifying...", _Action::NONE, theme::YELLOW );
            break;
        case update::Status::INSTALLING:
            _setFirmwareVersion(currentVersion, latestVersion);
            _setFirmwareStatus("Installing...", _Action::NONE, theme::ORANGE);
            break;
        case update::Status::SUCCESS:
            _setFirmwareVersion(currentVersion, latestVersion);
            _setFirmwareStatus("Restarting...", _Action::NONE, theme::GREEN);
            break;
        case update::Status::ERROR:
            _setFirmwareVersion(currentVersion, latestVersion);
            _setFirmwareStatus(snapshot.error[0] != '\0' ? snapshot.error : "Update failed", _Action::CHECK_FIRMWARE, theme::RED);
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
    text::copy(_firmwareStatusValue, sizeof(_firmwareStatusValue), value);

    _firmwareStatusField.value  = _firmwareStatusValue;
    _firmwareStatusField.action = action;
    _firmwareStatusField.color  = color;
}

void Updates::_setFirmwareVersion(const char* currentVersion, const char* latestVersion) {
    if (currentVersion == nullptr) { currentVersion = ""; }
    if (latestVersion != nullptr && latestVersion[0] != '\0' && !text::equals(currentVersion, latestVersion))
        { std::snprintf( _firmwareVersionValue, sizeof(_firmwareVersionValue), "%s -> %s", currentVersion, latestVersion); }
    else { text::copy(_firmwareVersionValue, sizeof(_firmwareVersionValue), currentVersion); }
    _firmwareVersionField.value = _firmwareVersionValue;
}

void Updates::_actionCheckFirmware(ST7796S::MSP4021 &tft) {
    update::checkFirmwareUpdate();
    _updateFirmwareFields(tft);
}

void Updates::_actionDownloadFirmware(ST7796S::MSP4021 &tft) {
    if (update::startFirmwareUpdate()) {
        _setFirmwareStatus("Starting...", _Action::NONE, theme::YELLOW);
        _updateField(tft, _firmwareStatusField);
        return;
    }
    _updateFirmwareFields(tft);
}

void Updates::_prepareSotaFields() {
    const update::SotaSnapshot snapshot = update::sotaSnapshot();
    const char* const installedVersion  = snapshot.installedVersion[0] != '\0'
        ? snapshot.installedVersion
        : nullptr;
    const char* const latestVersion     = snapshot.latestVersion[0]    != '\0'
        ? snapshot.latestVersion
        : nullptr;

    switch (snapshot.status) {
        case update::Status::NOT_INSTALLED:
            _setSotaVersion(nullptr);
            _setSotaStatus("Check", _Action::CHECK_SOTA, theme::CYAN);
            break;
        case update::Status::IDLE:
            _setSotaVersion(installedVersion);
            _setSotaStatus("Check", _Action::CHECK_SOTA, theme::CYAN);
            break;
        case update::Status::CHECKING:
            _setSotaVersion(installedVersion);
            std::snprintf(
                _sotaStatusValue, sizeof(_sotaStatusValue),
                "Checking %u%%",  static_cast<unsigned int>(snapshot.progress)
            );
            _sotaStatusField.value  = _sotaStatusValue;
            _sotaStatusField.action = _Action::NONE;
            _sotaStatusField.color  = theme::GREY;
            break;
        case update::Status::UP_TO_DATE:
            _setSotaVersion(installedVersion);
            _setSotaStatus("Up to date", _Action::CHECK_SOTA, theme::GREEN);
            break;
        case update::Status::AVAILABLE:
            _setSotaVersion(installedVersion, latestVersion);
            _setSotaStatus("Download", _Action::DOWNLOAD_SOTA, theme::CYAN);
            break;
        case update::Status::DOWNLOADING:
            _setSotaVersion(installedVersion, latestVersion);
            std::snprintf(
                _sotaStatusValue,   sizeof(_sotaStatusValue),
                "Downloading %u%%", static_cast<unsigned int>(snapshot.progress)
            );
            _sotaStatusField.value  = _sotaStatusValue;
            _sotaStatusField.action = _Action::NONE;
            _sotaStatusField.color  = theme::YELLOW;
            break;
        case update::Status::VERIFYING:
            _setSotaVersion(installedVersion, latestVersion);
            _setSotaStatus("Verifying...", _Action::NONE, theme::YELLOW);
            break;
        case update::Status::INSTALLING:
            _setSotaVersion(installedVersion, latestVersion);
            std::snprintf(
                _sotaStatusValue,  sizeof(_sotaStatusValue),
                "Installing %u%%", static_cast<unsigned int>(snapshot.progress)
            );
            _sotaStatusField.value  = _sotaStatusValue;
            _sotaStatusField.action = _Action::NONE;
            _sotaStatusField.color  = theme::ORANGE;
            break;
        case update::Status::SUCCESS:
            _setSotaVersion(installedVersion);
            _setSotaStatus("Installed", _Action::CHECK_SOTA, theme::GREEN);
            break;
        case update::Status::ERROR:
            _setSotaVersion(installedVersion, latestVersion);
            _setSotaStatus(snapshot.error[0] != '\0'
                ? snapshot.error
                : "Update failed", _Action::CHECK_SOTA, theme::RED
            );
            break;
        default:
            _setSotaVersion(installedVersion);
            _setSotaStatus("Unknown state", _Action::NONE, theme::RED);
            break;
    }
}

void Updates::_updateSotaFields(ST7796S::MSP4021 &tft) {
    _prepareSotaFields();
    _updateField(tft, _sotaVersionField);
    _updateField(tft, _sotaStatusField);
}

void Updates::_setSotaStatus(const char* const value, const _Action action, const uint16_t color) {
    text::copy(_sotaStatusValue, sizeof(_sotaStatusValue), value);

    _sotaStatusField.value  = _sotaStatusValue;
    _sotaStatusField.action = action;
    _sotaStatusField.color  = color;
}

void Updates::_setSotaVersion(const char* installedVersion, const char* latestVersion) {
    if (installedVersion == nullptr || installedVersion[0] == '\0') { installedVersion = "Not installed"; }

    if (latestVersion != nullptr && latestVersion[0] != '\0' && !text::equals(installedVersion, latestVersion))
        { std::snprintf(_sotaVersionValue, sizeof(_sotaVersionValue), "%s -> %s", installedVersion, latestVersion); }
    else { text::copy  (_sotaVersionValue, sizeof(_sotaVersionValue), installedVersion); }

    _sotaVersionField.value = _sotaVersionValue;
}

void Updates::_actionCheckSota(ST7796S::MSP4021 &tft) {
    update::checkSotaUpdate();
    _updateSotaFields(tft);
}

void Updates::_actionDownloadSota(ST7796S::MSP4021 &tft) {
    if (update::startSotaUpdate()) {
        _setSotaStatus("Starting...", _Action::NONE, theme::YELLOW);
        _updateField(tft, _sotaStatusField);
        return;
    }
    _updateSotaFields(tft);
}

void Updates::_preparePotaFields() {
    const update::PotaSnapshot snapshot = update::potaSnapshot();

    _setPotaRecords(snapshot.records);

    switch (snapshot.status) {
        case update::Status::NOT_INSTALLED:
        case update::Status::IDLE:
            _setPotaStatus("Check", _Action::CHECK_POTA, theme::CYAN);
            break;

        case update::Status::CHECKING:
            std::snprintf(
                _potaStatusValue,
                sizeof(_potaStatusValue),
                "Checking %u%%",
                static_cast<unsigned int>(snapshot.progress)
            );
            _potaStatusField.value  = _potaStatusValue;
            _potaStatusField.action = _Action::NONE;
            _potaStatusField.color  = theme::GREY;
            break;

        case update::Status::UP_TO_DATE:
            _setPotaStatus(
                "Up to date",
                _Action::CHECK_POTA,
                theme::GREEN
            );
            break;

        case update::Status::AVAILABLE:
            _setPotaStatus(
                "Download",
                _Action::DOWNLOAD_POTA,
                theme::CYAN
            );
            break;

        case update::Status::DOWNLOADING:
            std::snprintf(
                _potaStatusValue,
                sizeof(_potaStatusValue),
                "Downloading %u%%",
                static_cast<unsigned int>(snapshot.progress)
            );
            _potaStatusField.value  = _potaStatusValue;
            _potaStatusField.action = _Action::NONE;
            _potaStatusField.color  = theme::YELLOW;
            break;

        case update::Status::INSTALLING:
            std::snprintf(
                _potaStatusValue,
                sizeof(_potaStatusValue),
                "Installing %u%%",
                static_cast<unsigned int>(snapshot.progress)
            );
            _potaStatusField.value  = _potaStatusValue;
            _potaStatusField.action = _Action::NONE;
            _potaStatusField.color  = theme::ORANGE;
            break;

        case update::Status::VERIFYING:
            _setPotaStatus(
                "Verifying...",
                _Action::NONE,
                theme::YELLOW
            );
            break;

        case update::Status::SUCCESS:
            _setPotaStatus(
                "Installed",
                _Action::CHECK_POTA,
                theme::GREEN
            );
            break;

        case update::Status::ERROR:
            _setPotaStatus(
                snapshot.error[0] != '\0'
                    ? snapshot.error
                    : "Update failed",
                _Action::CHECK_POTA,
                theme::RED
            );
            break;

        default:
            _setPotaStatus(
                "Unknown state",
                _Action::NONE,
                theme::RED
            );
            break;
    }
}

void Updates::_updatePotaFields(ST7796S::MSP4021 &tft) {
    _preparePotaFields();
    _updateField(tft, _potaVersionField);
    _updateField(tft, _potaStatusField);
}

void Updates::_setPotaStatus(const char* const value, const _Action action, const uint16_t color) {
    text::copy(
        _potaStatusValue,
        sizeof(_potaStatusValue),
        value
    );

    _potaStatusField.value  = _potaStatusValue;
    _potaStatusField.action = action;
    _potaStatusField.color  = color;
}

void Updates::_setPotaRecords(const uint32_t records) {
    if (records == 0U) {
        text::copy(
            _potaVersionValue,
            sizeof(_potaVersionValue),
            "Not installed"
        );
    } else {
        std::snprintf(
            _potaVersionValue,
            sizeof(_potaVersionValue),
            "%lu parks",
            static_cast<unsigned long>(records)
        );
    }

    _potaVersionField.value = _potaVersionValue;
}

void Updates::_actionCheckPota(ST7796S::MSP4021 &tft) {
    update::checkPotaUpdate();
    _updatePotaFields(tft);
}

void Updates::_actionDownloadPota(ST7796S::MSP4021 &tft) {
    if (update::startPotaUpdate()) {
        _setPotaStatus(
            "Starting...",
            _Action::NONE,
            theme::YELLOW
        );
        _updateField(tft, _potaStatusField);
        return;
    }

    _updatePotaFields(tft);
}

void Updates::update(ST7796S::MSP4021 &tft) {
    _updateFirmwareFields(tft);
    _updateSotaFields(tft);
    _updatePotaFields(tft);
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
    _prepareSotaFields();
    _preparePotaFields();

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
                _actionCheckSota(tft);
                return true;
            case _Action::DOWNLOAD_SOTA:
                _actionDownloadSota(tft);
                return true;
            case _Action::CHECK_POTA:
                _actionCheckPota(tft);
                return true;
            case _Action::DOWNLOAD_POTA:
                _actionDownloadPota(tft);
                return true;
            case _Action::NONE:
            default: return false;
        }
    }
    return false;
}
