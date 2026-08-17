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
#include "utilities/ota.h"
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
namespace ota      = utilities::ota;
namespace buttons  = ui::mockup::buttons;
namespace grid     = ui::mockup::grid;
namespace right    = ui::mockup::right;
namespace uiMockup = ui::settings::mockup;
namespace theme    = ui::settings::themes::defaults;
namespace button   = ui::widgets::buttons;

namespace {
    constexpr uint8_t DEFAULT_RADIUS_INDEX = 6U;
    constexpr uint8_t RADIUS_COUNT         = 14U;
    constexpr size_t RESULTS_PER_PAGE      = 6U;
    constexpr int RESULTS_NAV_HEIGHT       = 24;
    constexpr int RESULT_ROW_HEIGHT        = 34;

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

    SearchType _searchType          = SearchType::SOTA;
    SearchType _submittedSearchType = SearchType::SOTA;
    SearchMode _searchMode          = SearchMode::NEAREST;

    ResultsViewStatus _displayedResultStatus = ResultsViewStatus::UNKNOWN;

    size_t _displayedResultCount  = 0U;
    size_t _displayedResultPage   = 0U;
    size_t _resultPage            = 0U;
    uint8_t _radiusIndex          = DEFAULT_RADIUS_INDEX;
    bool _searchSubmitted         = false;
    bool _searchButtonShowingStop = false;

    button::ButtonArea _sotaTypeButton     {};
    button::ButtonArea _potaTypeButton     {};
    button::ButtonArea _nearestModeButton  {};
    button::ButtonArea _codeModeButton     {};
    button::ButtonArea _areaModeButton     {};
    button::ButtonArea _parameterButton    {};
    button::ButtonArea _searchButton       {};
    button::ButtonArea _previousPageButton {};
    button::ButtonArea _nextPageButton     {};

    void _drawChoiceButton(ST7796S::MSP4021 &tft, const button::ButtonArea &area,
        const char* const label, const bool available, const bool selected
    );
    void _drawResultRow(ST7796S::MSP4021 &tft, const size_t visibleIndex,
        const ota::SearchResult &result
    );
    void _drawTypeSelector(ST7796S::MSP4021 &tft);
    void _drawModeSelector(ST7796S::MSP4021 &tft);
    void _drawParameter(ST7796S::MSP4021 &tft);
    void _drawSearchButton(ST7796S::MSP4021 &tft, const bool searching);
    bool _requestNearestSearch();
    ResultsView _currentResultsView();
    bool _resultAt(const size_t index, ota::SearchResult &result);
    size_t _pageCount(const size_t resultCount);
    void _drawPagination(ST7796S::MSP4021 &tft, const size_t resultCount);
    bool _drawReadyResults(ST7796S::MSP4021 &tft, const size_t resultCount);
    void _drawResultsStatus(ST7796S::MSP4021 &tft);

    void _drawChoiceButton(ST7796S::MSP4021 &tft, const button::ButtonArea &area,
        const char* const label, const bool available, const bool selected
    ) {
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

        if (_submittedSearchType == SearchType::SOTA) {
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

    bool _resultAt(const size_t index, ota::SearchResult &result) {
        switch (_submittedSearchType) {
            case SearchType::POTA:
                return sPota::nearbyResult(index, result);

            case SearchType::SOTA:
            default:
                return sSota::nearbyResult(index, result);
        }
    }

    void _drawResultRow(ST7796S::MSP4021 &tft, const size_t visibleIndex,
        const ota::SearchResult &result
    ) {
        constexpr int PADDING        = 6;
        constexpr int CODE_WIDTH     = 120;
        constexpr int DISTANCE_WIDTH = 88;

        const int rowX = grid::innerX();
        const int rowY =
            grid::innerY() +
            RESULTS_NAV_HEIGHT +
            uiMockup::GAP +
            static_cast<int>(visibleIndex) * RESULT_ROW_HEIGHT;
        const int rowW = grid::innerWidth();
        const int rowH = RESULT_ROW_HEIGHT - uiMockup::GAP;

        char distanceLabel[20];
        char pointsLabel[20];

        std::snprintf(
            distanceLabel,
            sizeof(distanceLabel),
            "%.1f KM",
            result.distanceKm
        );

        std::snprintf(
            pointsLabel,
            sizeof(pointsLabel),
            "%u +%u PTS",
            static_cast<unsigned int>(result.points),
            static_cast<unsigned int>(result.bonus)
        );

        tft.setFont(ST7796S::RobotoMono_Regular_14);

        tft.setTextColor(theme::WHITE);
        tft.textCenterLeft(
            rowX + PADDING,
            rowY,
            CODE_WIDTH - PADDING,
            rowH,
            result.code
        );

        tft.setTextColor(theme::CYAN);
        tft.textCenter(
            rowX + CODE_WIDTH,
            rowY,
            DISTANCE_WIDTH,
            rowH,
            distanceLabel
        );

        if (_submittedSearchType == SearchType::SOTA) {
            tft.setTextColor(theme::GREEN);
            tft.textCenterRight(
                rowX + CODE_WIDTH + DISTANCE_WIDTH,
                rowY,
                rowW - CODE_WIDTH - DISTANCE_WIDTH - PADDING,
                rowH,
                pointsLabel
            );
        }
    }

    size_t _pageCount(const size_t resultCount) {
        if (resultCount == 0U) {
            return 0U;
        }

        return (resultCount + RESULTS_PER_PAGE - 1U) /
            RESULTS_PER_PAGE;
    }

    void _drawPagination(ST7796S::MSP4021 &tft, const size_t resultCount) {
        constexpr int PAGE_BUTTON_WIDTH = 40;

        const size_t pageCount = _pageCount(resultCount);
        if (pageCount == 0U) {
            return;
        }

        if (_resultPage >= pageCount) {
            _resultPage = pageCount - 1U;
        }

        const int navX = grid::innerX();
        const int navY = grid::innerY();
        const int navW = grid::innerWidth();
        const int navH = RESULTS_NAV_HEIGHT;

        _previousPageButton = button::makeArea(
            navX,
            navY,
            PAGE_BUTTON_WIDTH,
            navH
        );

        _nextPageButton = button::makeArea(
            navX + navW - PAGE_BUTTON_WIDTH,
            navY,
            PAGE_BUTTON_WIDTH,
            navH
        );

        tft.rectFill(navX, navY, navW, navH, theme::BLACK);

        _drawChoiceButton(
            tft,
            _previousPageButton,
            "<",
            _resultPage > 0U,
            false
        );

        _drawChoiceButton(
            tft,
            _nextPageButton,
            ">",
            _resultPage + 1U < pageCount,
            false
        );

        char pageLabel[16];
        const int written = std::snprintf(
            pageLabel,
            sizeof(pageLabel),
            "%u/%u",
            static_cast<unsigned int>(_resultPage + 1U),
            static_cast<unsigned int>(pageCount)
        );

        if (written > 0 &&
            static_cast<size_t>(written) < sizeof(pageLabel)
        ) {
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(theme::WHITE);
            tft.textCenter(
                navX + PAGE_BUTTON_WIDTH,
                navY,
                navW - (PAGE_BUTTON_WIDTH * 2),
                navH,
                pageLabel
            );
        }
    }

    bool _drawReadyResults(ST7796S::MSP4021 &tft, const size_t resultCount) {
        _drawPagination(tft, resultCount);

        const size_t firstResultIndex =
            _resultPage * RESULTS_PER_PAGE;
        const size_t remainingCount =
            resultCount - firstResultIndex;
        const size_t visibleCount =
            remainingCount < RESULTS_PER_PAGE
                ? remainingCount
                : RESULTS_PER_PAGE;

        for (size_t visibleIndex = 0U;
            visibleIndex < visibleCount;
            ++visibleIndex
        ) {
            ota::SearchResult result {};
            const size_t resultIndex =
                firstResultIndex + visibleIndex;

            if (!_resultAt(resultIndex, result)) {
                return false;
            }

            _drawResultRow(tft, visibleIndex, result);
        }

        return true;
    }

    void _drawResultsStatus(ST7796S::MSP4021 &tft) {
        const ResultsView view = _currentResultsView();

        if (view.status == _displayedResultStatus &&
            view.count  == _displayedResultCount  &&
            _resultPage == _displayedResultPage
        ) { return; }

        _displayedResultStatus = view.status;
        _displayedResultCount  = view.count;
        _displayedResultPage   = _resultPage;

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

        switch (view.status) {
            case ResultsViewStatus::SEARCHING:
                label = _submittedSearchType == SearchType::SOTA
                    ? "SEARCHING SOTA"
                    : "SEARCHING POTA";
                color = theme::CYAN;
                break;
            case ResultsViewStatus::READY:
                if (_drawReadyResults(tft, view.count)) {
                    return;
                }
                label = "SEARCH ERROR";
                color = theme::RED;
                break;
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
    _searchMode            = SearchMode::NEAREST;
    _radiusIndex           = DEFAULT_RADIUS_INDEX;
    _searchSubmitted       = false;
    _displayedResultStatus = ResultsViewStatus::UNKNOWN;
    _displayedResultCount  = 0U;
    _displayedResultPage   = 0U;
    _resultPage            = 0U;

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

    const ResultsView resultsView = _currentResultsView();
    if (resultsView.status == ResultsViewStatus::READY) {
        const size_t pageCount = _pageCount(resultsView.count);

        if (button::isPressed(_previousPageButton, x, y)) {
            if (_resultPage > 0U) {
                --_resultPage;
                _drawResultsStatus(tft);
            }
            return true;
        }
        if (button::isPressed(_nextPageButton, x, y)) {
            if (_resultPage + 1U < pageCount) {
                ++_resultPage;
                _drawResultsStatus(tft);
            }
            return true;
        }
    }

    if (button::isPressed(_sotaTypeButton, x, y)) {
        if (sotaAvailable && _searchType != SearchType::SOTA) {
            _searchType      = SearchType::SOTA;
            _drawTypeSelector(tft);
        }
        return true;
    }
    if (button::isPressed(_potaTypeButton, x, y)) {
        if (potaAvailable && _searchType != SearchType::POTA) {
            _searchType = SearchType::POTA;
            _drawTypeSelector(tft);
        }
        return true;
    }
    if (button::isPressed(_nearestModeButton, x, y)) {
        if (_searchMode != SearchMode::NEAREST) {
            _searchMode = SearchMode::NEAREST;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_codeModeButton, x, y)) {
        if (_searchMode != SearchMode::CODE) {
            _searchMode = SearchMode::CODE;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_areaModeButton, x, y)) {
        if (_searchMode != SearchMode::AREA) {
            _searchMode = SearchMode::AREA;
            _drawModeSelector(tft);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_parameterButton, x, y)) {
        if (_searchMode == SearchMode::NEAREST) {
            _radiusIndex = static_cast<uint8_t>((_radiusIndex + 1U) % RADIUS_COUNT);
            _drawParameter(tft);
        }
        return true;
    }
    if (button::isPressed(_searchButton, x, y)) {
        if (_searchMode == SearchMode::NEAREST && _requestNearestSearch()) {
            _submittedSearchType = _searchType;
            _searchSubmitted     = true;
            _resultPage          = 0U;
            _drawSearchButton(tft, true);
            _drawTypeSelector(tft);
            _drawModeSelector(tft);
            _drawParameter(tft);
            _drawResultsStatus(tft);
        }
        return true;
    }
    return false;
}
