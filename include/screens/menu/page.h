/*
 * include/screens/menu/page.h
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

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <MSP4021.h>

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

namespace screens::menu {
    class Page {
        public:
            enum class Mode {GRID, KEYBOARD};
            struct Area {
                int x = 0;
                int y = 0;
                int w = 0;
                int h = 0;
            };

            template<typename _Action>
            struct Field {
                Area area;
                const char* label = nullptr;
                const char* value = nullptr;
                _Action action {};
                uint16_t color = ui::settings::themes::defaults::WHITE;
            };

            virtual ~Page() = default;
            virtual void draw       (ST7796S::MSP4021 &tft) = 0;
            virtual void update     (ST7796S::MSP4021 &tft) {}
            virtual bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) { return false; }

            virtual bool isEditing() const { return _mode == Mode::KEYBOARD; }
            virtual void reset()           { _mode = Mode::GRID; }
        protected:
            Mode _mode = Mode::GRID;
            static void _drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap, const char* title) {
                char upper[32];
                size_t i = 0;
                while (title[i] != '\0' && i < sizeof(upper) - 1) {
                    upper[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(title[i])));
                    ++i;
                }
                upper[i] = '\0';

                tft.setFont(ST7796S::RobotoMono_Bold_16);
                tft.setTextColor(ui::settings::themes::defaults::GREEN);
                tft.textCenterLeft(x, y, w, h, upper);
                tft.lineH(x, y + h + gap, w, ui::settings::themes::defaults::BORDER);
            }

            template<typename _Action>
            static Field<_Action> _makeField(const char* label, _Action action, uint16_t color) {
                Field<_Action> field;
                field.label  = label;
                field.action = action;
                field.color  = color;
                return field;
            }

            template<typename _Action>
            static void _makeFieldArea(Field<_Action> &field, int x, int y, int w, int h) {
                field.area.x = x;
                field.area.y = y;
                field.area.w = w;
                field.area.h = h;
            }

            template<typename _Action>
            static void _drawLine(ST7796S::MSP4021 &tft, const Field<_Action> &field) {
                const int gap = ui::settings::mockup::GAP;

                tft.setFont(ST7796S::RobotoMono_Regular_14);
                tft.setTextColor(ui::settings::themes::defaults::GREY);
                tft.textCenterLeft(
                    field.area.x,       field.area.y,
                    field.area.w / 2,   field.area.h,
                    field.label
                );
                tft.setTextColor(field.color);
                tft.textCenterRight(
                    field.area.x + (field.area.w / 2),  field.area.y,
                    (field.area.w / 2) - gap,           field.area.h,
                    field.value
                );
            }
            
            template<typename _Action>
            static void _updateField(ST7796S::MSP4021 &tft, const Field<_Action> &field) {
                const int gap = ui::settings::mockup::GAP;
                tft.rectFill(
                    field.area.x + (field.area.w / 2),  field.area.y,
                    (field.area.w / 2) - gap,           field.area.h,
                    ui::settings::themes::defaults::BLACK
                );
                tft.setFont(ST7796S::RobotoMono_Regular_14);
                tft.setTextColor(field.color);
                tft.textCenterRight(
                    field.area.x + (field.area.w / 2),  field.area.y,
                    (field.area.w / 2) - gap,           field.area.h,
                    field.value
                );
            }

            template<typename _Action>
            static bool _isPressed(const Field<_Action> &field, int tx, int ty) {
                return
                    tx >= field.area.x && tx < field.area.x + field.area.w &&
                    ty >= field.area.y && ty < field.area.y + field.area.h;
            }
    };
}