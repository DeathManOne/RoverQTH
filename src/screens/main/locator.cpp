/*
 * screens/main/locator.cpp
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
#include "screens/mockup/grid.h"
#include "ui/settings/themes/defaults.h"
#include "ui/settings/mockup.h"

#include "ui/fonts/locator_40.h"
#include "ui/fonts/RobotoMono_Regular_12.h"
#include "ui/fonts/RobotoMono_Regular_14.h"
#include "ui/fonts/RobotoMono_Regular_18.h"

namespace screens::main::locator {
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
            int separatorY = 0;
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

        Sota sota;
        Mark mark;
        Grid locator      ("---- -- -- --");
        Grid statusTop    ("---");
        Grid statusBottom ("--- --  -  -- ---  -  ---- ---");

        void clearArea(ST7796S::MSP4021 &tft, const Area &area);
        void setValue(char* field, const char* value);
        void setArea(Area &area, int outerX, int outerY, int outerW, int outerH);
        void updateGrid(ST7796S::MSP4021 &tft, Grid &grid, const char* value, void (*drawFn)(ST7796S::MSP4021 &));
        void updateSota(ST7796S::MSP4021 &tft, char* field, const char* value);
        void updateMark(ST7796S::MSP4021 &tft, char* field, const char* value);
        void drawGrid(ST7796S::MSP4021 &tft, const Grid &grid, uint16_t color);
        void drawTargetSeparator(ST7796S::MSP4021 &tft, const TargetInfo &info, uint16_t color);
        void drawSota(ST7796S::MSP4021 &tft);
        void drawMark(ST7796S::MSP4021 &tft);
        void drawLocator(ST7796S::MSP4021 &tft);
        void drawStatusTop(ST7796S::MSP4021 &tft);
        void drawStatusBottom(ST7796S::MSP4021 &tft);
        void computeTargetTitleArea(ST7796S::MSP4021 &tft, TargetInfo &info);
        void clearMarkColumn(ST7796S::MSP4021 &tft, int col);
        void drawMarkColumn(ST7796S::MSP4021 &tft, int col);
        void updateMarkColumn(ST7796S::MSP4021 &tft, char* field, const char* value, int col);
        

        void clearArea(ST7796S::MSP4021 &tft, const Area &area) {
            tft.rectFill(
                area.outerX + 1, area.outerY + 1,
                area.outerW - 2, area.outerH - 2,
                ui::settings::themes::defaults::BLACK
            );
        }

        void setValue(char* field, const char* value) {
            strncpy(field, value, VALUE_SIZE - 1);
            field[VALUE_SIZE - 1] = '\0';
        }

        void setArea(Area &area, int outerX, int outerY, int outerW, int outerH) {
            const int gap = ui::settings::mockup::GAP;

            area.outerX = outerX;
            area.outerY = outerY;
            area.outerW = outerW;
            area.outerH = outerH;

            area.innerX = area.outerX + gap;
            area.innerY = area.outerY + gap;
            area.innerW = area.outerW - (gap * 2);
            area.innerH = area.outerH - (gap * 2);
        }

        void updateGrid(ST7796S::MSP4021 &tft, Grid &grid, const char* value, void (*drawFn)(ST7796S::MSP4021 &)) {
            if (strcmp(grid.value, value) == 0)
                { return; }
            setValue(grid.value, value);
            clearArea(tft, grid.area);
            drawFn(tft);

            if (&grid == &locator) {
                clearArea(tft, statusBottom.area);
                drawStatusBottom(tft);
            }
        }

        void updateSota(ST7796S::MSP4021 &tft, char* field, const char* value) {
            if (strcmp(field, value) == 0)
                { return; }
            setValue(field, value);
            computeTargetTitleArea(tft, sota.info);

            clearArea(tft, sota.info.oldTitleArea);
            clearArea(tft, sota.info.titleArea);
            clearArea(tft, sota.info.area);

            drawSota(tft);
        }

        void updateMark(ST7796S::MSP4021 &tft, char* field, const char* value) {
            if (strcmp(field, value) == 0)
                { return; }
            setValue(field, value);
            computeTargetTitleArea(tft, mark.info);

            clearArea(tft, mark.info.oldTitleArea);
            clearArea(tft, mark.info.titleArea);
            clearArea(tft, mark.info.area);

            drawMark(tft);
        }

        void drawGrid(ST7796S::MSP4021 &tft, const Grid &grid, uint16_t color) {
            tft.setFont(ST7796S::Locator_40);
            tft.setTextColor(color);
            tft.textCenter(
                grid.area.innerX, grid.area.innerY,
                grid.area.innerW, grid.area.innerH,
                grid.value
            );
        }

        void drawTargetSeparator(ST7796S::MSP4021 &tft, const TargetInfo &info, uint16_t color) {
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
                    ui::settings::themes::defaults::BORDER
                );
            }

            if (rightW > 0) {
                tft.lineH(
                    rightX, info.separatorY,
                    rightW,
                    ui::settings::themes::defaults::BORDER
                );
            }
        }

        void drawSota(ST7796S::MSP4021 &tft) {
            constexpr int COL_COUNT = 4;
            const int colW = sota.info.area.innerW / COL_COUNT;

            tft.setFont(ST7796S::RobotoMono_Regular_14);

            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textBottomCenter(
                sota.info.area.innerX,  sota.info.area.innerY,
                colW,                   sota.info.area.innerH / 2,
                "BRG"
            );

            tft.textBottomCenter(
                sota.info.area.innerX + colW,   sota.info.area.innerY,
                colW,                           sota.info.area.innerH / 2,
                "DIST"
            );

            tft.textBottomCenter(
                sota.info.area.innerX + (colW * 2), sota.info.area.innerY,
                colW,                               sota.info.area.innerH / 2,
                "PTS"
            );

            tft.textBottomCenter(
                sota.info.area.innerX + (colW * 3), sota.info.area.innerY,
                colW,                               sota.info.area.innerH / 2,
                "ALT"
            );

            tft.setTextColor(ui::settings::themes::defaults::CYAN);
            tft.textTopCenter(
                sota.info.area.innerX,  sota.info.area.innerY + (sota.info.area.innerH / 2),
                colW,                   sota.info.area.innerH / 2,
                sota.info.bearing
            );

            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textTopCenter(
                sota.info.area.innerX + colW,   sota.info.area.innerY + (sota.info.area.innerH / 2),
                colW,                           sota.info.area.innerH / 2,
                sota.info.distance
            );

            tft.setTextColor(ui::settings::themes::defaults::YELLOW);
            tft.textTopCenter(
                sota.info.area.innerX + (colW * 2), sota.info.area.innerY + (sota.info.area.innerH / 2),
                colW,                               sota.info.area.innerH / 2,
                sota.points
            );

            tft.setTextColor(ui::settings::themes::defaults::WHITE);
            tft.textTopCenter(
                sota.info.area.innerX + (colW * 3), sota.info.area.innerY + (sota.info.area.innerH / 2),
                colW,                               sota.info.area.innerH / 2,
                sota.altitude
            );
            drawTargetSeparator(tft, sota.info, ui::settings::themes::defaults::ORANGE);
        }

        void drawMark(ST7796S::MSP4021 &tft) {
            constexpr int COL_COUNT = 3;
            const int colW          = mark.info.area.innerW / COL_COUNT;
            const int labelH        = mark.info.area.innerH / 2;
            const int valueH        = mark.info.area.innerH - labelH;

            constexpr const char* labels[] = {"BRG", "DIST", "T+"};
            const char* const values[] = {mark.info.bearing, mark.info.distance, mark.timer};

            tft.setFont(ST7796S::RobotoMono_Regular_14);

            for (int i = 0; i < COL_COUNT; i++) {
                const int colX = mark.info.area.innerX + (colW * i);

                tft.setTextColor(ui::settings::themes::defaults::GREY);
                tft.textBottomCenter(
                    colX, mark.info.area.innerY,
                    colW, labelH,
                    labels[i]
                );

                tft.setTextColor(i == 0 ? ui::settings::themes::defaults::CYAN : ui::settings::themes::defaults::WHITE);
                tft.textTopCenter(
                    colX, mark.info.area.innerY + labelH,
                    colW, valueH,
                    values[i]
                );
            }
            drawTargetSeparator(tft, mark.info, ui::settings::themes::defaults::GREEN);
        }

        void drawLocator(ST7796S::MSP4021 &tft) {
            drawGrid(tft, locator, ui::settings::themes::defaults::YELLOW);
        }

        void drawStatusTop(ST7796S::MSP4021 &tft) {
            tft.setFont(ST7796S::RobotoMono_Regular_12);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textTopRight(
                statusTop.area.innerX, statusTop.area.innerY,
                statusTop.area.innerW, statusTop.area.innerH,
                statusTop.value
            );
        }

        void drawStatusBottom(ST7796S::MSP4021 &tft) {
            tft.setFont(ST7796S::RobotoMono_Regular_12);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textTopCenter(
                statusBottom.area.innerX, statusBottom.area.innerY,
                statusBottom.area.innerW, statusBottom.area.innerH,
                statusBottom.value
            );
        }

        void computeTargetTitleArea(ST7796S::MSP4021 &tft, TargetInfo &info) {
            tft.setFont(ST7796S::RobotoMono_Regular_18);
            info.oldTitleArea = info.titleArea;

            const int gap       = ui::settings::mockup::GAP;
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

        void clearMarkColumn(ST7796S::MSP4021 &tft, int col) {
            constexpr int COL_COUNT = 3;
            constexpr int CORNER_PAD = 5;

            const int colW = mark.info.area.innerW / COL_COUNT;
            int colX       = mark.info.area.innerX + (colW * col);
            int drawW      = colW;

            if (col == (COL_COUNT - 1))
                { drawW -= CORNER_PAD; }
            else if (col == 0) {
                colX += CORNER_PAD;
                drawW -= CORNER_PAD;
            }

            tft.rectFill(
                colX,   mark.info.area.innerY,
                drawW,  mark.info.area.innerH + CORNER_PAD,
                ui::settings::themes::defaults::BLACK
            );
        }

        void drawMarkColumn(ST7796S::MSP4021 &tft, int col) {
            constexpr int COL_COUNT = 3;
            const int colW          = mark.info.area.innerW / COL_COUNT;
            const int labelH        = mark.info.area.innerH / 2;
            const int valueH        = mark.info.area.innerH - labelH;
            const int colX          = mark.info.area.innerX + (colW * col);

            constexpr const char* labels[] = {"BRG", "DIST", "T+"};
            const char* const values[] = {mark.info.bearing, mark.info.distance, mark.timer};

            tft.setFont(ST7796S::RobotoMono_Regular_14);

            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textBottomCenter(
                colX, mark.info.area.innerY,
                colW, labelH,
                labels[col]
            );

            tft.setTextColor(col == 0 ? ui::settings::themes::defaults::CYAN : ui::settings::themes::defaults::WHITE);
            tft.textTopCenter(
                colX, mark.info.area.innerY + labelH,
                colW, valueH,
                values[col]
            );
        }

        void updateMarkColumn(ST7796S::MSP4021 &tft, char* field, const char* value, int col) {
            if (strcmp(field, value) == 0)
                { return; }
            setValue(field, value);
            clearMarkColumn(tft, col);
            drawMarkColumn(tft, col);
        }
    }

    void setLocator     (const char* value) { setValue(locator.value,       value); }
    void setStatusTop   (const char* value) { setValue(statusTop.value,     value); }
    void setStatusBottom(const char* value) { setValue(statusBottom.value,  value); }

    void setSOTACode    (const char* value) { setValue(sota.info.title,     value); }
    void setSOTABearing (const char* value) { setValue(sota.info.bearing,   value); }
    void setSOTADistance(const char* value) { setValue(sota.info.distance,  value); }
    void setSOTAPoints  (const char* value) { setValue(sota.points,         value); }
    void setSOTAAltitude(const char* value) { setValue(sota.altitude,       value); }

    void setMarkLocator (const char* value) { setValue(mark.info.title,     value); }
    void setMarkBearing (const char* value) { setValue(mark.info.bearing,   value); }
    void setMarkDistance(const char* value) { setValue(mark.info.distance,  value); }
    void setMarkTimer   (const char* value) { setValue(mark.timer,          value); }

    void updateLocator      (ST7796S::MSP4021 &tft, const char* value) { updateGrid(tft, locator,   value, drawLocator); }
    void updateStatusTop    (ST7796S::MSP4021 &tft, const char* value) { updateGrid(tft, statusTop, value, drawStatusTop); }

    void updateSOTACode     (ST7796S::MSP4021 &tft, const char* value) { updateSota(tft, sota.info.title,    value); }
    void updateSOTABearing  (ST7796S::MSP4021 &tft, const char* value) { updateSota(tft, sota.info.bearing,  value); }
    void updateSOTADistance (ST7796S::MSP4021 &tft, const char* value) { updateSota(tft, sota.info.distance, value); }
    void updateSOTAPoints   (ST7796S::MSP4021 &tft, const char* value) { updateSota(tft, sota.points,        value); }
    void updateSOTAAltitude (ST7796S::MSP4021 &tft, const char* value) { updateSota(tft, sota.altitude,      value); }

    void updateMarkLocator  (ST7796S::MSP4021 &tft, const char* value) { updateMark(tft, mark.info.title,           value); }
    void updateMarkBearing  (ST7796S::MSP4021 &tft, const char* value) { updateMarkColumn(tft, mark.info.bearing,   value, 0); }
    void updateMarkDistance (ST7796S::MSP4021 &tft, const char* value) { updateMarkColumn(tft, mark.info.distance,  value, 1); }
    void updateMarkTimer    (ST7796S::MSP4021 &tft, const char* value) { updateMarkColumn(tft, mark.timer,          value, 2); }

    void updateStatusBottom(ST7796S::MSP4021 &tft, const char* value) {
        if (strcmp(statusBottom.value, value) == 0)
            { return; }
        setValue(statusBottom.value, value);
        clearArea(tft, statusBottom.area);
        drawStatusBottom(tft);
    }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);
        const int gap           = ui::settings::mockup::GAP;
        const int gridX         = screens::mockup::grid::outerX();
        const int gridY         = screens::mockup::grid::outerY();
        const int gridW         = screens::mockup::grid::outerWidth();
        const int gridH         = screens::mockup::grid::outerHeight();
        const int rowCount      = ui::settings::mockup::GRID_REF_ROW_COUNT;
        const int virtualRowH   = (gridH + (gap * 4)) / rowCount;

        setArea(
            sota.info.area,
            gridX,      gridY + gap,
            gridW,      virtualRowH
        );
        sota.info.separatorY = sota.info.area.outerY + sota.info.area.outerH + gap;
        computeTargetTitleArea(tft, sota.info);

        setArea(
            mark.info.area,                     gridX,
            gridY + gridH - gap - virtualRowH,  gridW,
            virtualRowH
        );
        mark.info.separatorY = mark.info.area.outerY - gap;
        computeTargetTitleArea(tft, mark.info);

        tft.setFont(ST7796S::RobotoMono_Regular_12);
        const int statusH       = tft.textHeight(statusBottom.value) + (gap * 2);
        const int statusTopY    = sota.info.titleArea.outerY + sota.info.titleArea.outerH + gap;

        setArea(
            statusTop.area,
            gridX + gap,        statusTopY,
            gridW - (gap * 2),  statusH
        );

        tft.setFont(ST7796S::Locator_40);
        const int locatorH = tft.textHeight(locator.value) + (gap * 2);
        const int locatorY = gridY + ((gridH - locatorH) / 2);

        setArea(
            locator.area,
            gridX + gap,        locatorY,
            gridW - (gap * 2),  locatorH
        );
        
        setArea(
            statusBottom.area,
            gridX + gap,        locator.area.outerY + (locator.area.outerH / 2) + (gap * 2),
            gridW - (gap * 2),  statusH
        );

        drawSota(tft);
        drawLocator(tft);
        drawStatusTop(tft);
        drawStatusBottom(tft);
        drawMark(tft);
    }
}