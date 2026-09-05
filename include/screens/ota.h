/*
 * include/screens/ota.h
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

#include "database/ota.h"
#include "screens/screen.h"
#include "ui/widgets/buttons.h"

namespace screens {
    /** @brief Provides interactive SOTA and POTA search, sorting and selection. */
    class Ota final : public Screen {
        private:
            static constexpr size_t RESULTS_PER_PAGE = 6U;
            static constexpr size_t NO_RESULT_SELECTED =
                database::ota::RESULT_CAPACITY;

            enum class SearchType : uint8_t {SOTA, POTA};
            enum class SearchMode : uint8_t {NEAREST, CODE};
            enum class SortMode : uint8_t {DISTANCE, CODE, POINTS};
            enum class SelectionStatus : uint8_t {NONE, PENDING, SAVED, ERROR};
            enum class ClearSelectionStatus : uint8_t {
                HIDDEN,
                AVAILABLE,
                CLEARED,
                ERROR
            };
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
                    ResultsViewStatus initialStatus,
                    size_t initialCount
                ) : status(initialStatus), count(initialCount) {}

                ResultsViewStatus status;
                size_t count;
            };

            SortMode _sortMode = SortMode::DISTANCE;
            SearchType _searchType = SearchType::SOTA;
            SearchType _submittedSearchType = SearchType::SOTA;
            SearchMode _searchMode = SearchMode::NEAREST;

            SelectionStatus _selectionStatus = SelectionStatus::NONE;
            ResultsViewStatus _displayedResultStatus =
                ResultsViewStatus::UNKNOWN;
            ClearSelectionStatus _clearSelectionStatus =
                ClearSelectionStatus::HIDDEN;

            char _codePrefix[database::ota::CODE_SIZE] {};
            size_t _resultOrder[database::ota::RESULT_CAPACITY] {};
            size_t _displayedResultCount = 0U;
            size_t _displayedResultPage = 0U;
            size_t _resultPage = 0U;
            size_t _visibleResultCount = 0U;
            size_t _resultOrderCount = 0U;
            size_t _selectedResultIndex = NO_RESULT_SELECTED;
            uint8_t _radiusIndex = 6U;
            bool _keyboardActive = false;
            bool _searchSubmitted = false;
            bool _searchButtonShowingStop = false;

            ui::widgets::buttons::ButtonArea _sotaTypeButton {};
            ui::widgets::buttons::ButtonArea _potaTypeButton {};
            ui::widgets::buttons::ButtonArea _nearestModeButton {};
            ui::widgets::buttons::ButtonArea _codeModeButton {};
            ui::widgets::buttons::ButtonArea _sortButton {};
            ui::widgets::buttons::ButtonArea _parameterButton {};
            ui::widgets::buttons::ButtonArea _searchButton {};
            ui::widgets::buttons::ButtonArea _clearSelectionButton {};
            ui::widgets::buttons::ButtonArea _previousPageButton {};
            ui::widgets::buttons::ButtonArea _nextPageButton {};
            ui::widgets::buttons::ButtonArea
                _resultButtons[RESULTS_PER_PAGE] {};

            void _drawChoiceButton(
                ST7796S::MSP4021 &tft,
                const ui::widgets::buttons::ButtonArea &area,
                const char* label,
                bool available,
                bool selected
            );
            void _drawResultRow(
                ST7796S::MSP4021 &tft,
                size_t visibleIndex,
                const database::ota::SearchResult &result,
                SelectionStatus selectionStatus,
                bool imperial
            );
            void _drawTypeSelector(ST7796S::MSP4021 &tft);
            void _drawModeSelector(ST7796S::MSP4021 &tft);
            void _drawParameter(ST7796S::MSP4021 &tft);
            void _openCodeKeyboard(ST7796S::MSP4021 &tft);
            void _drawSearchButton(
                ST7796S::MSP4021 &tft,
                bool searching
            );
            bool _requestSearch();
            ResultsView _currentResultsView();
            bool _sourceResultAt(
                size_t index,
                database::ota::SearchResult &result
            );
            bool _resultAt(
                size_t index,
                database::ota::SearchResult &result
            );
            bool _resultComesBefore(
                const database::ota::SearchResult &left,
                const database::ota::SearchResult &right
            );
            void _rebuildResultOrder(size_t resultCount);
            bool _saveSelectedResult();
            void _drawClearSelection(ST7796S::MSP4021 &tft);
            bool _clearSelection();
            size_t _pageCount(size_t resultCount);
            void _drawPagination(
                ST7796S::MSP4021 &tft,
                size_t resultCount
            );
            bool _drawReadyResults(
                ST7796S::MSP4021 &tft,
                size_t resultCount
            );
            void _drawResultsStatus(ST7796S::MSP4021 &tft);

        public:
            /** @copydoc Screen::preload() */
            void preload() override;
            /** @copydoc Screen::draw() */
            void draw(ST7796S::MSP4021 &tft) override;

            /** @copydoc Screen::update() */
            void update(
                ST7796S::MSP4021 &tft,
                uint32_t &nextRefreshIn
            ) override;

            /** @copydoc Screen::handleTouch() */
            bool handleTouch(
                ST7796S::MSP4021 &tft,
                int x,
                int y
            ) override;

            /** @copydoc Screen::isEditing() */
            bool isEditing() const override;
    };
}
