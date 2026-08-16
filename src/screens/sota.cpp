/*
 * src/screens/sota.cpp
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

#include "screens/main/title.h"
#include "screens/sota.h"
#include "services/dtc.h"
#include "services/gps.h"
#include "services/pota.h"
#include "services/sota.h"
#include "ui/mockup/buttons.h"
#include "ui/mockup/grid.h"
#include "ui/mockup/right.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"
#include "ui/widgets/buttons.h"

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace title    = screens::main::title;
namespace sota     = screens::sota;
namespace dtc      = services::dtc;
namespace gps      = services::gps;
namespace sPota    = services::pota;
namespace sSota    = services::sota;
namespace buttons  = ui::mockup::buttons;
namespace grid     = ui::mockup::grid;
namespace right    = ui::mockup::right;
namespace uiMockup = ui::settings::mockup;
namespace theme    = ui::settings::themes::defaults;
namespace button   = ui::widgets::buttons;

namespace {
    constexpr uint8_t DEFAULT_RADIUS_INDEX = 6U;
    constexpr uint8_t RADIUS_COUNT         = 14U;

    constexpr const char* RADIUS_LABELS[RADIUS_COUNT] = {
        "RADIUS 5 KM",
        "RADIUS 10 KM",
        "RADIUS 15 KM",
        "RADIUS 20 KM",
        "RADIUS 25 KM",
        "RADIUS 50 KM",
        "RADIUS 75 KM",
        "RADIUS 100 KM",
        "RADIUS 125 KM",
        "RADIUS 150 KM",
        "RADIUS 175 KM",
        "RADIUS 200 KM",
        "RADIUS 250 KM",
        "RADIUS ALL"
    };

    constexpr double RADIUS_VALUES[RADIUS_COUNT] = {
        5.0,
        10.0,
        15.0,
        20.0,
        25.0,
        50.0,
        75.0,
        100.0,
        125.0,
        150.0,
        175.0,
        200.0,
        250.0,
        0.0
    };

    enum class SearchType : uint8_t {SOTA, POTA};
    enum class SearchMode : uint8_t {NEAREST, CODE, AREA};
    enum class ResultsViewStatus : uint8_t {
        UNKNOWN,
        HIDDEN,
        SEARCHING,
        READY,
        EMPTY,
        ERROR
    };

    struct ResultsView {
        ResultsView(
            const ResultsViewStatus initialStatus = ResultsViewStatus::UNKNOWN,
            const size_t initialCount = 0U
        ) :
            status(initialStatus),
            count(initialCount) {}

        ResultsViewStatus status;
        size_t count;
    };

    SearchType _searchType = SearchType::SOTA;
    SearchMode _searchMode = SearchMode::NEAREST;

    ResultsViewStatus _displayedResultStatus = ResultsViewStatus::UNKNOWN;

    size_t _displayedResultCount  = 0U;
    uint8_t _radiusIndex          = DEFAULT_RADIUS_INDEX;
    bool _searchSubmitted         = false;
    bool _searchButtonShowingStop = false;

    button::ButtonArea _sotaTypeButton    {};
    button::ButtonArea _potaTypeButton    {};
    button::ButtonArea _nearestModeButton {};
    button::ButtonArea _codeModeButton    {};
    button::ButtonArea _areaModeButton    {};
    button::ButtonArea _parameterButton   {};
    button::ButtonArea _searchButton      {};

    void _drawChoiceButton(ST7796S::MSP4021 &tft, const button::ButtonArea &area, const char* const label, const bool available, const bool selected) {
        tft.rectFill(
            area.x,     area.y,
            area.width, area.height,
            theme::BLACK
        );

        if (available && selected) {
            tft.rectRoundFill(
                area.x,           area.y,
                area.width,       area.height,
                uiMockup::RADIUS, theme::GREEN
            );
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(theme::BLACK);
        } else {
            tft.rectRound(
                area.x,           area.y,
                area.width,       area.height,
                uiMockup::RADIUS, theme::BORDER
            );
            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(available ? theme::WHITE : theme::GREY);
        }

        tft.textCenter(
            area.x,     area.y,
            area.width, area.height,
            label
        );
    }

    void _drawTypeSelector(ST7796S::MSP4021 &tft) {
        const int gap     = uiMockup::GAP;
        const int rowH    = right::innerHeight() / uiMockup::RIGHT_ROW_COUNT;
        const int buttonW = (right::innerWidth() - (gap * 3)) / 2;
        const int buttonH = rowH - (gap * 2);
        const int buttonY = right::innerY() + gap;
        const int sotaX   = right::innerX() + gap;
        const int potaX   = sotaX + buttonW + gap;

        _sotaTypeButton = button::makeArea(sotaX, buttonY, buttonW, buttonH);
        _potaTypeButton = button::makeArea(potaX, buttonY, buttonW, buttonH);

        const bool controlsEnabled = !_searchButtonShowingStop;
        const bool sotaAvailable   = controlsEnabled &&
            sSota::snapshot().status != sSota::Status::UNAVAILABLE;
        const bool potaAvailable   = controlsEnabled &&
            sPota::snapshot().status != sPota::Status::UNAVAILABLE;

        _drawChoiceButton(tft, _sotaTypeButton, "SOTA", sotaAvailable, _searchType == SearchType::SOTA);
        _drawChoiceButton(tft, _potaTypeButton, "POTA", potaAvailable, _searchType == SearchType::POTA);
    }

    void _drawModeSelector(ST7796S::MSP4021 &tft) {
        const int gap     = uiMockup::GAP;
        const int rowH    = right::innerHeight() / uiMockup::RIGHT_ROW_COUNT;
        const int buttonX = right::innerX() + gap;
        const int buttonW = right::innerWidth() - (gap * 2);
        const int buttonH = rowH - (gap * 2);

        _nearestModeButton = button::makeArea(
            buttonX,
            right::innerY() + rowH + gap,
            buttonW,
            buttonH
        );

        _codeModeButton = button::makeArea(
            buttonX,
            right::innerY() + (rowH * 2) + gap,
            buttonW,
            buttonH
        );

        _areaModeButton = button::makeArea(
            buttonX,
            right::innerY() + (rowH * 3) + gap,
            buttonW,
            buttonH
        );

        const bool controlsEnabled = !_searchButtonShowingStop;

        _drawChoiceButton(
            tft, _nearestModeButton, "NEAREST",
            controlsEnabled, _searchMode == SearchMode::NEAREST
        );

        _drawChoiceButton(
            tft, _codeModeButton, "CODE",
            controlsEnabled, _searchMode == SearchMode::CODE
        );

        _drawChoiceButton(
            tft, _areaModeButton, "AREA",
            controlsEnabled, _searchMode == SearchMode::AREA
        );
    }

    void _drawParameter(ST7796S::MSP4021 &tft) {
        const int gap     = uiMockup::GAP;
        const int rowH    = right::innerHeight() / uiMockup::RIGHT_ROW_COUNT;
        const int buttonX = right::innerX() + gap;
        const int buttonY = right::innerY() + (rowH * 4) + gap;
        const int buttonW = right::innerWidth() - (gap * 2);
        const int buttonH = rowH - (gap * 2);

        _parameterButton  = button::makeArea(buttonX, buttonY, buttonW, buttonH);
        const char* label = nullptr;

        switch (_searchMode) {
            case SearchMode::CODE:
                label = "ENTER CODE";
                break;
            case SearchMode::AREA:
                label = "ENTER AREA";
                break;
            case SearchMode::NEAREST:
            default:
                label = RADIUS_LABELS[_radiusIndex];
                break;
        }

        _drawChoiceButton(tft, _parameterButton, label, !_searchButtonShowingStop, false);
    }

    void _drawSearchButton(ST7796S::MSP4021 &tft, const bool searching) {
        const int gap     = uiMockup::GAP;
        const int rowH    = right::innerHeight() / uiMockup::RIGHT_ROW_COUNT;
        const int buttonX = right::innerX() + gap;
        const int buttonY = right::innerY() + (rowH * 5) + gap;
        const int buttonW = right::innerWidth() - (gap * 2);
        const int buttonH = rowH - (gap * 2);

        _searchButton = button::makeArea(buttonX, buttonY, buttonW, buttonH);

        tft.rectFill(
            _searchButton.x,     _searchButton.y,
            _searchButton.width, _searchButton.height,
            theme::BLACK
        );

        const uint16_t color = searching ? theme::RED : theme::GREEN;
        tft.rectRound(
            _searchButton.x,     _searchButton.y,
            _searchButton.width, _searchButton.height,
            uiMockup::RADIUS, color
        );

        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(color);
        tft.textCenter(
            _searchButton.x,     _searchButton.y,
            _searchButton.width, _searchButton.height,
            searching ? "STOP" : "SEARCH"
        );

        _searchButtonShowingStop = searching;
    }

    bool _requestNearestSearch() {
        gps::Snapshot position {};
        if (!gps::getSnapshot(position) ||
            !position.positionValid
        ) { return false; }

        const double radiusKm = RADIUS_VALUES[_radiusIndex];
        switch (_searchType) {
            case SearchType::POTA:
                return sPota::requestNearby(
                    position.latitude,
                    position.longitude,
                    radiusKm
                );
            case SearchType::SOTA:
            default:
                return sSota::requestNearby(
                    position.latitude,
                    position.longitude,
                    radiusKm
                );
        }
    }

    ResultsView _currentResultsView() {
        if (!_searchSubmitted) {
            return {ResultsViewStatus::HIDDEN, 0U};
        }

        if (_searchType == SearchType::SOTA) {
            const sSota::NearbySnapshot snapshot =
                sSota::nearbySnapshot();

            switch (snapshot.status) {
                case sSota::NearbyStatus::SEARCHING:
                    return {ResultsViewStatus::SEARCHING, 0U};
                case sSota::NearbyStatus::READY:
                    return {ResultsViewStatus::READY, snapshot.count};
                case sSota::NearbyStatus::EMPTY:
                    return {ResultsViewStatus::EMPTY, 0U};
                case sSota::NearbyStatus::IDLE:
                    return {ResultsViewStatus::HIDDEN, 0U};
                case sSota::NearbyStatus::ERROR:
                case sSota::NearbyStatus::UNAVAILABLE:
                default:
                    return {ResultsViewStatus::ERROR, 0U};
            }
        }

        const sPota::NearbySnapshot snapshot =
            sPota::nearbySnapshot();

        switch (snapshot.status) {
            case sPota::NearbyStatus::SEARCHING:
                return {ResultsViewStatus::SEARCHING, 0U};
            case sPota::NearbyStatus::READY:
                return {ResultsViewStatus::READY, snapshot.count};
            case sPota::NearbyStatus::EMPTY:
                return {ResultsViewStatus::EMPTY, 0U};
            case sPota::NearbyStatus::IDLE:
                return {ResultsViewStatus::HIDDEN, 0U};
            case sPota::NearbyStatus::ERROR:
            case sPota::NearbyStatus::UNAVAILABLE:
            default:
                return {ResultsViewStatus::ERROR, 0U};
        }
    }

    void _drawResultsStatus(ST7796S::MSP4021 &tft) {
        const ResultsView view = _currentResultsView();

        if (view.status == _displayedResultStatus &&
            view.count == _displayedResultCount
        ) { return; }

        _displayedResultStatus = view.status;
        _displayedResultCount  = view.count;

        tft.rectFill(
            grid::innerX(),
            grid::innerY(),
            grid::innerWidth(),
            grid::innerHeight(),
            theme::BLACK
        );

        if (view.status == ResultsViewStatus::HIDDEN) {
            return;
        }

        const char* label = nullptr;
        uint16_t color    = theme::WHITE;
        char countLabel[24];

        switch (view.status) {
            case ResultsViewStatus::SEARCHING:
                label = "SEARCHING";
                color = theme::CYAN;
                break;

            case ResultsViewStatus::READY: {
                const int written = std::snprintf(
                    countLabel,
                    sizeof(countLabel),
                    "%u RESULTS",
                    static_cast<unsigned int>(view.count)
                );

                label =
                    written > 0 &&
                    static_cast<size_t>(written) < sizeof(countLabel)
                        ? countLabel
                        : "RESULTS";

                color = theme::GREEN;
                break;
            }

            case ResultsViewStatus::EMPTY:
                label = "NO RESULTS";
                color = theme::GREY;
                break;

            case ResultsViewStatus::ERROR:
            case ResultsViewStatus::UNKNOWN:
            default:
                label = "SEARCH ERROR";
                color = theme::RED;
                break;
        }

        tft.setFont(ST7796S::RobotoMono_Bold_16);
        tft.setTextColor(color);
        tft.textCenter(
            grid::innerX(),
            grid::innerY(),
            grid::innerWidth(),
            grid::innerHeight(),
            label
        );
    }
}

void sota::preload() {
    _searchMode       = SearchMode::NEAREST;
    _radiusIndex      = DEFAULT_RADIUS_INDEX;
    _searchSubmitted  = false;
    _displayedResultStatus = ResultsViewStatus::UNKNOWN;
    _displayedResultCount  = 0U;

    const bool sotaAvailable = sSota::snapshot().status != sSota::Status::UNAVAILABLE;
    const bool potaAvailable = sPota::snapshot().status != sPota::Status::UNAVAILABLE;

    if      (sotaAvailable) { _searchType = SearchType::SOTA; }
    else if (potaAvailable) { _searchType = SearchType::POTA; }
    else                    { _searchType = SearchType::SOTA; }
}

void sota::draw(ST7796S::MSP4021& tft) {
    tft.fillScreen(theme::BLACK);

    title::draw(tft);
    grid::draw(tft);
    _drawResultsStatus(tft);
    right::draw(tft);

    _drawSearchButton(tft, _currentResultsView().status == ResultsViewStatus::SEARCHING);
    _drawTypeSelector(tft);
    _drawModeSelector(tft);
    _drawParameter(tft);

    buttons::draw(tft);
}

void sota::update(ST7796S::MSP4021& tft, uint32_t& nextRefreshIn) {
    char date[16];
    char time[16];
    char battery[8];

    dtc::getDate(date, sizeof(date));
    dtc::getTime(time, sizeof(time));
    title::getBatteryLevel(battery, sizeof(battery));

    title::updateDate(tft, date);
    title::updateTime(tft, time);
    title::updateBattery(tft, battery);

    const bool searching = _currentResultsView().status == ResultsViewStatus::SEARCHING;
    if (searching != _searchButtonShowingStop) {
        _drawSearchButton(tft, searching);
        _drawTypeSelector(tft);
        _drawModeSelector(tft);
        _drawParameter(tft);
    }

    _drawResultsStatus(tft);

    nextRefreshIn = 1000;
}

bool sota::handleTouch(ST7796S::MSP4021& tft, const int x, const int y) {
    const bool sotaAvailable = sSota::snapshot().status != sSota::Status::UNAVAILABLE;
    const bool potaAvailable = sPota::snapshot().status != sPota::Status::UNAVAILABLE;

    if (_searchButtonShowingStop) {
        if (button::isPressed(_searchButton, x, y)) {
            switch (_searchType) {
                case SearchType::POTA:
                    sPota::cancelNearby();
                    break;
                case SearchType::SOTA:
                default:
                    sSota::cancelNearby();
                    break;
            }
            return true;
        }
        return false;
    }

    if (button::isPressed(_sotaTypeButton, x, y)) {
        if (sotaAvailable && _searchType != SearchType::SOTA) {
            _searchType      = SearchType::SOTA;
            _searchSubmitted = false;
            _drawTypeSelector(tft);
        }
        return true;
    }
    if (button::isPressed(_potaTypeButton, x, y)) {
        if (potaAvailable && _searchType != SearchType::POTA) {
            _searchType = SearchType::POTA;
            _searchSubmitted = false;
            _drawTypeSelector(tft);
        }
        return true;
    }
    if (button::isPressed(_nearestModeButton, x, y)) {
        if (_searchMode != SearchMode::NEAREST) {
            _searchMode = SearchMode::NEAREST;
            _searchSubmitted = false;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_codeModeButton, x, y)) {
        if (_searchMode != SearchMode::CODE) {
            _searchMode = SearchMode::CODE;
            _searchSubmitted = false;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_areaModeButton, x, y)) {
        if (_searchMode != SearchMode::AREA) {
            _searchMode = SearchMode::AREA;
            _searchSubmitted = false;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_parameterButton, x, y)) {
        if (_searchMode == SearchMode::NEAREST) {
            _radiusIndex = static_cast<uint8_t>((_radiusIndex + 1U) % RADIUS_COUNT);
            _searchSubmitted = false;
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_searchButton, x, y)) {
        if (_searchMode == SearchMode::NEAREST &&
            _requestNearestSearch()
        ) {
            _searchSubmitted = true;
            _drawSearchButton(tft, true);
            _drawTypeSelector(tft);
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    return false;
}
