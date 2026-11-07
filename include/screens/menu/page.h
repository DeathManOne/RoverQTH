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

#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"
#include "ui/widgets/buttons.h"

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace screens::menu {
    /** @brief Base contract and shared drawing helpers for menu pages. */
    class Page {
        public:
            /** @brief Selects the normal grid or keyboard presentation mode. */
            enum class Mode {GRID, KEYBOARD};

            /** @brief Rectangular layout area assigned to a menu field. */
            struct Area {
                int x = 0;
                int y = 0;
                int w = 0;
                int h = 0;
            };

            /** @brief Describes one labeled menu field and its associated action. */
            template<typename _Action>
            struct Field {
                Area area;
                const char* label = nullptr;
                const char* value = nullptr;
                _Action action {};
                uint16_t color    = ui::settings::themes::defaults::WHITE;
            };

            /** @brief Enables safe polymorphic destruction. */
            virtual ~Page() = default;

            /**
             * @brief Draws the complete menu page.
             * @param tft Display instance used to draw the page.
             */
            virtual void draw(ST7796S::MSP4021 &tft) = 0;

            /**
             * @brief Refreshes dynamic values displayed by the page.
             * @param tft Display instance used to refresh the page.
             */
            virtual void update(ST7796S::MSP4021 &tft) {}

            /**
             * @brief Handles a mapped touch event for the page.
             * @param tft Display instance available to redraw affected controls.
             * @param x Horizontal touch coordinate in pixels.
             * @param y Vertical touch coordinate in pixels.
             * @return `true` when the touch event was handled.
             */
            virtual bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) { return false; }

            /**
             * @brief Reports whether the page currently displays its keyboard.
             * @return `true` when the page is in keyboard mode.
             */
            virtual bool isEditing() const { return _mode == Mode::KEYBOARD; }

            /** @brief Returns the page to its normal grid mode. */
            virtual void reset() { _mode = Mode::GRID; }

        protected:
            /** @brief Current presentation mode of the page. */
            Mode _mode = Mode::GRID;

            /**
             * @brief Draws an uppercase page title and its separator line.
             * @param tft Display instance used to draw the title.
             * @param x Horizontal origin of the title area.
             * @param y Vertical origin of the title area.
             * @param w Width of the title area in pixels.
             * @param h Height of the title area in pixels.
             * @param gap Gap between the title and its separator line.
             * @param title Title text to convert and draw.
             */
            static void _drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap, const char* title) {
                char upper[32];
                size_t i = 0;
                while (i < sizeof(upper) - 1U && title[i] != '\0') {
                    upper[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(title[i])));
                    ++i;
                }
                upper[i] = '\0';

                tft.setFont(ST7796S::RobotoMono_Bold_16);
                tft.setTextColor(ui::settings::themes::defaults::GREEN);
                tft.textCenterLeft(x, y, w, h, upper);
                tft.lineH(x, y + h + gap, w, ui::settings::themes::defaults::BORDER);
            }

            /**
             * @brief Creates a menu field with its label, action and value color.
             * @tparam _Action Action type associated with the field.
             * @param label Label displayed by the field.
             * @param action Action associated with the field.
             * @param color Color used to draw the field value.
             * @return Initialized field descriptor.
             */
            template<typename _Action>
            static Field<_Action> _makeField(const char* label, _Action action, uint16_t color) {
                Field<_Action> field;
                field.label  = label;
                field.action = action;
                field.color  = color;
                return field;
            }

            /**
             * @brief Assigns a rectangular drawing and touch area to a field.
             * @tparam _Action Action type associated with the field.
             * @param field Field whose area is updated.
             * @param x Horizontal origin of the area.
             * @param y Vertical origin of the area.
             * @param w Width of the area in pixels.
             * @param h Height of the area in pixels.
             */
            template<typename _Action>
            static void _makeFieldArea(Field<_Action> &field, int x, int y, int w, int h) {
                field.area.x = x;
                field.area.y = y;
                field.area.w = w;
                field.area.h = h;
            }

            /**
             * @brief Draws a complete labeled field line.
             * @tparam _Action Action type associated with the field.
             * @param tft Display instance used to draw the line.
             * @param field Field descriptor to draw.
             */
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

            /**
             * @brief Clears and redraws only the value portion of a field.
             * @tparam _Action Action type associated with the field.
             * @param tft Display instance used to refresh the value.
             * @param field Field descriptor whose value is redrawn.
             */
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

            /**
             * @brief Tests whether a touch coordinate lies inside a field.
             * @tparam _Action Action type associated with the field.
             * @param field Field whose touch area is tested.
             * @param tx Horizontal touch coordinate in pixels.
             * @param ty Vertical touch coordinate in pixels.
             * @return `true` when the field contains the touch coordinate.
             */
            template<typename _Action>
            static bool _isPressed(const Field<_Action> &field, int tx, int ty) {
                const ui::widgets::buttons::ButtonArea area =
                    ui::widgets::buttons::makeArea(
                        field.area.x, field.area.y,
                        field.area.w, field.area.h
                    );
                return ui::widgets::buttons::isPressed(area, tx, ty);
            }
    };
}
