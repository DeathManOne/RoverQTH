/*
 * src/screens/main/locator.cpp
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

#include <cstring>
#include <cstddef>
#include "screens/main/locator.h"
#include "ui/fonts/locator_40.h"
#include "ui/fonts/RobotoMono_Regular_12.h"
#include "ui/fonts/RobotoMono_Regular_14.h"
#include "ui/fonts/RobotoMono_Regular_18.h"
#include "ui/mockup/grid.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"

namespace locator = screens::main::locator;
namespace grid    = ui::mockup::grid;
namespace theme   = ui::settings::themes::defaults;
namespace mockup  = ui::settings::mockup;

namespace {
    constexpr size_t VALUE_SIZE = 32;
    struct Area {
        int outerX = 0;
        int outerY = 0;
        int outerW = 0;
        int outerH = 0;
        int innerX = 0;
        int innerY = 0;
        int innerW = 0;
        int innerH = 0;
    };

    struct TargetInfo {
        Area area;
        Area titleArea;
        Area oldTitleArea;
        int separatorY            = 0;
        char title[VALUE_SIZE]    = "---";
        char bearing[VALUE_SIZE]  = "---";
        char distance[VALUE_SIZE] = "---";
    };

    struct Sota {
        TargetInfo info;
        char points[VALUE_SIZE]   = "---";
        char altitude[VALUE_SIZE] = "---";
    };

    struct Mark {
        TargetInfo info;
        char timer[VALUE_SIZE] = "-- : -- : --";
    };

    struct Grid {
        Area area;
        char value[VALUE_SIZE] = {};
        Grid(const char* initial) {
            strncpy(value, initial, VALUE_SIZE - 1);
            value[VALUE_SIZE - 1] = '\0';
        }
    };

    Sota _sota;
    Mark _mark;

    Grid _locator("---- -- -- --");
    Grid _statusTop("---");
    Grid _statusBottom("--- --  -  -- ---  -  ---- ---");

    void _clearArea(ST7796S::MSP4021 &tft, const Area &area);
    void _setValue(char* field, const char* value);
    void _setArea(Area &area, int outerX, int outerY, int outerW, int outerH);

    void _drawGrid  (ST7796S::MSP4021 &tft, const Grid &grid, uint16_t color);
    void _updateGrid(ST7796S::MSP4021 &tft, Grid &grid, const char* value, void (*drawFn)(ST7796S::MSP4021 &));

    void _drawSota(ST7796S::MSP4021 &tft);
    void _updateSota(ST7796S::MSP4021 &tft, char* field, const char* value);

    void _drawMark(ST7796S::MSP4021 &tft);
    void _updateMark(ST7796S::MSP4021 &tft, char* field, const char* value);

    void _drawTargetSeparator(ST7796S::MSP4021 &tft, const TargetInfo &info, uint16_t color);
    void _drawLocator(ST7796S::MSP4021 &tft);
    void _drawStatusTop(ST7796S::MSP4021 &tft);
    void _drawStatusBottom(ST7796S::MSP4021 &tft);
    void _computeTargetTitleArea(ST7796S::MSP4021 &tft, TargetInfo &info);

    void _clearMarkColumn(ST7796S::MSP4021 &tft, int col);
    void _drawMarkColumn(ST7796S::MSP4021 &tft, int col);
    void _updateMarkColumn(ST7796S::MSP4021 &tft, char* field, const char* value, int col);
    

    void _clearArea(ST7796S::MSP4021 &tft, const Area &area) {
        tft.rectFill(
            area.outerX + 1, area.outerY + 1,
            area.outerW - 2, area.outerH - 2,
            theme::BLACK
        );
    }

    void _setValue(char* field, const char* value) {
        strncpy(field, value, VALUE_SIZE - 1);
        field[VALUE_SIZE - 1] = '\0';
    }

    void _setArea(Area &area, int outerX, int outerY, int outerW, int outerH) {
        const int gap = mockup::GAP;

        area.outerX = outerX;
        area.outerY = outerY;
        area.outerW = outerW;
        area.outerH = outerH;

        area.innerX = area.outerX + gap;
        area.innerY = area.outerY + gap;
        area.innerW = area.outerW - (gap * 2);
        area.innerH = area.outerH - (gap * 2);
    }

    void _updateGrid(ST7796S::MSP4021 &tft, Grid &grid, const char* value, void (*drawFn)(ST7796S::MSP4021 &)) {
        if (strcmp(grid.value, value) == 0) { return; }

        _setValue(grid.value, value);
        _clearArea(tft, grid.area);
        drawFn(tft);

        if (&grid == &_locator) {
            _clearArea(tft, _statusBottom.area);
            _drawStatusBottom(tft);
        }
    }

    void _updateSota(ST7796S::MSP4021 &tft, char* field, const char* value) {
        if (strcmp(field, value) == 0) { return; }

        _setValue(field, value);
        _computeTargetTitleArea(tft, _sota.info);

        _clearArea(tft, _sota.info.oldTitleArea);
        _clearArea(tft, _sota.info.titleArea);
        _clearArea(tft, _sota.info.area);

        _drawSota(tft);
    }

    void _updateMark(ST7796S::MSP4021 &tft, char* field, const char* value) {
        if (strcmp(field, value) == 0) { return; }

        _setValue(field, value);
        _computeTargetTitleArea(tft, _mark.info);

        _clearArea(tft, _mark.info.oldTitleArea);
        _clearArea(tft, _mark.info.titleArea);
        _clearArea(tft, _mark.info.area);

        _drawMark(tft);
    }

    void _drawGrid(ST7796S::MSP4021 &tft, const Grid &grid, uint16_t color) {
        tft.setFont(ST7796S::Locator_40);
        tft.setTextColor(color);
        tft.textCenter(
            grid.area.innerX, grid.area.innerY,
            grid.area.innerW, grid.area.innerH,
            grid.value
        );
    }

    void _drawTargetSeparator(ST7796S::MSP4021 &tft, const TargetInfo &info, uint16_t color) {
        const int spacing = 8;

        const int textX = info.titleArea.innerX;
        const int textY = info.titleArea.innerY;
        const int textW = info.titleArea.innerW;

        const int leftW = textX - info.area.innerX - spacing;
        const int rightX = textX + textW + spacing;
        const int rightW = (info.area.innerX + info.area.innerW) - rightX;

        tft.setFont(ST7796S::RobotoMono_Regular_18);
        tft.setTextColor(color);
        tft.text(textX, textY, info.title);

        if (leftW > 0) {
            tft.lineH(
                info.area.innerX, info.separatorY,
                leftW,
                theme::BORDER
            );
        }

        if (rightW > 0) {
            tft.lineH(
                rightX, info.separatorY,
                rightW,
                theme::BORDER
            );
        }
    }

    void _drawSota(ST7796S::MSP4021 &tft) {
        constexpr int COL_COUNT = 4;
        const int colW          = _sota.info.area.innerW / COL_COUNT;

        tft.setFont(ST7796S::RobotoMono_Regular_14);
        tft.setTextColor(theme::GREY);
        tft.textBottomCenter(
            _sota.info.area.innerX, _sota.info.area.innerY,
            colW,                   _sota.info.area.innerH / 2,
            "BRG"
        );
        tft.textBottomCenter(
            _sota.info.area.innerX + colW, _sota.info.area.innerY,
            colW,                          _sota.info.area.innerH / 2,
            "DIST"
        );
        tft.textBottomCenter(
            _sota.info.area.innerX + (colW * 2), _sota.info.area.innerY,
            colW,                                _sota.info.area.innerH / 2,
            "PTS"
        );
        tft.textBottomCenter(
            _sota.info.area.innerX + (colW * 3), _sota.info.area.innerY,
            colW,                                _sota.info.area.innerH / 2,
            "ALT"
        );
        tft.setTextColor(theme::CYAN);
        tft.textTopCenter(
            _sota.info.area.innerX, _sota.info.area.innerY + (_sota.info.area.innerH / 2),
            colW,                   _sota.info.area.innerH / 2,
            _sota.info.bearing
        );

        tft.setTextColor(theme::WHITE);
        tft.textTopCenter(
            _sota.info.area.innerX + colW, _sota.info.area.innerY + (_sota.info.area.innerH / 2),
            colW,                          _sota.info.area.innerH / 2,
            _sota.info.distance
        );

        tft.setTextColor(theme::YELLOW);
        tft.textTopCenter(
            _sota.info.area.innerX + (colW * 2), _sota.info.area.innerY + (_sota.info.area.innerH / 2),
            colW,                                _sota.info.area.innerH / 2,
            _sota.points
        );

        tft.setTextColor(theme::WHITE);
        tft.textTopCenter(
            _sota.info.area.innerX + (colW * 3), _sota.info.area.innerY + (_sota.info.area.innerH / 2),
            colW,                                _sota.info.area.innerH / 2,
            _sota.altitude
        );
        _drawTargetSeparator(tft, _sota.info, theme::ORANGE);
    }

    void _drawMark(ST7796S::MSP4021 &tft) {
        constexpr int COL_COUNT = 3;
        const int colW          = _mark.info.area.innerW / COL_COUNT;
        const int labelH        = _mark.info.area.innerH / 2;
        const int valueH        = _mark.info.area.innerH - labelH;

        constexpr const char* labels[] = {"BRG", "DIST", "T+"};
        const char* const values[]     = {_mark.info.bearing, _mark.info.distance, _mark.timer};

        tft.setFont(ST7796S::RobotoMono_Regular_14);
        for (int i = 0; i < COL_COUNT; i++) {
            const int colX = _mark.info.area.innerX + (colW * i);

            tft.setTextColor(theme::GREY);
            tft.textBottomCenter(
                colX, _mark.info.area.innerY,
                colW, labelH,
                labels[i]
            );
            tft.setTextColor(i == 0 ? theme::CYAN : theme::WHITE);
            tft.textTopCenter(
                colX, _mark.info.area.innerY + labelH,
                colW, valueH,
                values[i]
            );
        }
        _drawTargetSeparator(tft, _mark.info, theme::GREEN);
    }

    void _drawLocator(ST7796S::MSP4021 &tft) {
        _drawGrid(tft, _locator, theme::YELLOW);
    }

    void _drawStatusTop(ST7796S::MSP4021 &tft) {
        tft.setFont(ST7796S::RobotoMono_Regular_12);
        tft.setTextColor(theme::GREY);
        tft.textTopRight(
            _statusTop.area.innerX, _statusTop.area.innerY,
            _statusTop.area.innerW, _statusTop.area.innerH,
            _statusTop.value
        );
    }

    void _drawStatusBottom(ST7796S::MSP4021 &tft) {
        tft.setFont(ST7796S::RobotoMono_Regular_12);
        tft.setTextColor(theme::GREY);
        tft.textTopCenter(
            _statusBottom.area.innerX, _statusBottom.area.innerY,
            _statusBottom.area.innerW, _statusBottom.area.innerH,
            _statusBottom.value
        );
    }

    void _computeTargetTitleArea(ST7796S::MSP4021 &tft, TargetInfo &info) {
        tft.setFont(ST7796S::RobotoMono_Regular_18);
        info.oldTitleArea = info.titleArea;

        const int gap       = mockup::GAP;
        const int textW     = tft.textWidth(info.title);
        const int textH     = tft.textHeight(info.title);
        const int centerX   = info.area.innerX + (info.area.innerW / 2);

        info.titleArea.outerX = centerX - (textW / 2) - (gap * 2);
        info.titleArea.outerY = info.separatorY - (textH / 2) - gap;
        info.titleArea.outerW = textW + (gap * 4);
        info.titleArea.outerH = textH + (gap * 2);

        info.titleArea.innerX = info.titleArea.outerX + (gap * 2);
        info.titleArea.innerY = info.titleArea.outerY + gap;
        info.titleArea.innerW = textW;
        info.titleArea.innerH = textH;
    }

    void _clearMarkColumn(ST7796S::MSP4021 &tft, int col) {
        constexpr int COL_COUNT = 3;
        constexpr int CORNER_PAD = 5;

        const int colW = _mark.info.area.innerW / COL_COUNT;
        int colX       = _mark.info.area.innerX + (colW * col);
        int drawW      = colW;

        if (col == (COL_COUNT - 1)) { drawW -= CORNER_PAD; }
        else if (col == 0) {
            colX += CORNER_PAD;
            drawW -= CORNER_PAD;
        }

        tft.rectFill(
            colX,   _mark.info.area.innerY,
            drawW,  _mark.info.area.innerH + CORNER_PAD,
            theme::BLACK
        );
    }

    void _drawMarkColumn(ST7796S::MSP4021 &tft, int col) {
        constexpr int COL_COUNT = 3;
        const int colW          = _mark.info.area.innerW / COL_COUNT;
        const int labelH        = _mark.info.area.innerH / 2;
        const int valueH        = _mark.info.area.innerH - labelH;
        const int colX          = _mark.info.area.innerX + (colW * col);

        constexpr const char* labels[] = {"BRG", "DIST", "T+"};
        const char* const values[] = {_mark.info.bearing, _mark.info.distance, _mark.timer};

        tft.setFont(ST7796S::RobotoMono_Regular_14);
        tft.setTextColor(theme::GREY);
        tft.textBottomCenter(
            colX, _mark.info.area.innerY,
            colW, labelH,
            labels[col]
        );
        tft.setTextColor(col == 0 ? theme::CYAN : theme::WHITE);
        tft.textTopCenter(
            colX, _mark.info.area.innerY + labelH,
            colW, valueH,
            values[col]
        );
    }

    void _updateMarkColumn(ST7796S::MSP4021 &tft, char* field, const char* value, int col) {
        if (strcmp(field, value) == 0)
            { return; }
        _setValue(field, value);
        _clearMarkColumn(tft, col);
        _drawMarkColumn(tft, col);
    }
}

void locator::setLocator     (const char* value) { _setValue(_locator.value,       value); }
void locator::setStatusTop   (const char* value) { _setValue(_statusTop.value,     value); }
void locator::setStatusBottom(const char* value) { _setValue(_statusBottom.value,  value); }

void locator::setSOTACode    (const char* value) { _setValue(_sota.info.title,     value); }
void locator::setSOTABearing (const char* value) { _setValue(_sota.info.bearing,   value); }
void locator::setSOTADistance(const char* value) { _setValue(_sota.info.distance,  value); }
void locator::setSOTAPoints  (const char* value) { _setValue(_sota.points,         value); }
void locator::setSOTAAltitude(const char* value) { _setValue(_sota.altitude,       value); }

void locator::setMarkLocator (const char* value) { _setValue(_mark.info.title,     value); }
void locator::setMarkBearing (const char* value) { _setValue(_mark.info.bearing,   value); }
void locator::setMarkDistance(const char* value) { _setValue(_mark.info.distance,  value); }
void locator::setMarkTimer   (const char* value) { _setValue(_mark.timer,          value); }

void locator::updateLocator     (ST7796S::MSP4021 &tft, const char* value) { _updateGrid(tft, _locator,   value, _drawLocator); }
void locator::updateStatusTop   (ST7796S::MSP4021 &tft, const char* value) { _updateGrid(tft, _statusTop, value, _drawStatusTop); }

void locator::updateSOTACode    (ST7796S::MSP4021 &tft, const char* value) { _updateSota(tft, _sota.info.title,    value); }
void locator::updateSOTABearing (ST7796S::MSP4021 &tft, const char* value) { _updateSota(tft, _sota.info.bearing,  value); }
void locator::updateSOTADistance(ST7796S::MSP4021 &tft, const char* value) { _updateSota(tft, _sota.info.distance, value); }
void locator::updateSOTAPoints  (ST7796S::MSP4021 &tft, const char* value) { _updateSota(tft, _sota.points,        value); }
void locator::updateSOTAAltitude(ST7796S::MSP4021 &tft, const char* value) { _updateSota(tft, _sota.altitude,      value); }

void locator::updateMarkLocator (ST7796S::MSP4021 &tft, const char* value) { _updateMark(tft, _mark.info.title,           value); }
void locator::updateMarkBearing (ST7796S::MSP4021 &tft, const char* value) { _updateMarkColumn(tft, _mark.info.bearing,   value, 0); }
void locator::updateMarkDistance(ST7796S::MSP4021 &tft, const char* value) { _updateMarkColumn(tft, _mark.info.distance,  value, 1); }
void locator::updateMarkTimer   (ST7796S::MSP4021 &tft, const char* value) { _updateMarkColumn(tft, _mark.timer,          value, 2); }

void locator::updateStatusBottom(ST7796S::MSP4021 &tft, const char* value) {
    if (strcmp(_statusBottom.value, value) == 0)
        { return; }
    _setValue(_statusBottom.value, value);
    _clearArea(tft, _statusBottom.area);
    _drawStatusBottom(tft);
}

void locator::draw(ST7796S::MSP4021 &tft) {
    grid::draw(tft);

    const int gap           = mockup::GAP;
    const int gridX         = grid::outerX();
    const int gridY         = grid::outerY();
    const int gridW         = grid::outerWidth();
    const int gridH         = grid::outerHeight();
    const int rowCount      = mockup::GRID_REF_ROW_COUNT;
    const int virtualRowH   = (gridH + (gap * 4)) / rowCount;

    _setArea(
        _sota.info.area,
        gridX,      gridY + gap,
        gridW,      virtualRowH
    );
    _sota.info.separatorY = _sota.info.area.outerY + _sota.info.area.outerH + gap;
    _computeTargetTitleArea(tft, _sota.info);

    _setArea(
        _mark.info.area,                     gridX,
        gridY + gridH - gap - virtualRowH,  gridW,
        virtualRowH
    );
    _mark.info.separatorY = _mark.info.area.outerY - gap;
    _computeTargetTitleArea(tft, _mark.info);

    tft.setFont(ST7796S::RobotoMono_Regular_12);
    const int statusH    = tft.textHeight(_statusBottom.value) + (gap * 2);
    const int statusTopY = _sota.info.titleArea.outerY + _sota.info.titleArea.outerH + gap;

    _setArea(
        _statusTop.area,
        gridX + gap,        statusTopY,
        gridW - (gap * 2),  statusH
    );

    tft.setFont(ST7796S::Locator_40);
    const int locatorH = tft.textHeight(_locator.value) + (gap * 2);
    const int locatorY = gridY + ((gridH - locatorH) / 2);

    _setArea(
        _locator.area,
        gridX + gap,        locatorY,
        gridW - (gap * 2),  locatorH
    );
    
    _setArea(
        _statusBottom.area,
        gridX + gap,        _locator.area.outerY + (_locator.area.outerH / 2) + (gap * 2),
        gridW - (gap * 2),  statusH
    );

    _drawSota(tft);
    _drawLocator(tft);
    _drawStatusTop(tft);
    _drawStatusBottom(tft);
    _drawMark(tft);
}
