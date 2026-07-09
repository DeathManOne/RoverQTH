/*
 * screens/menu/battery.cpp
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
#include "screens/menu/battery.h"
#include "screens/mockup/grid.h"
#include "services/settings.h"
#include "ui/settings/mockup.h"
#include "ui/settings/themes/defaults.h"

#include "ui/fonts/RobotoMono_Bold_16.h"
#include "ui/fonts/RobotoMono_Regular_14.h"

namespace screens::menu::battery {
    namespace {
        enum class Mode {GRID, KEYBOARD};
        Mode mode = Mode::GRID;

        enum class Action {
            NONE,
            CAPACITY,
            MINIMAL,
            NOMINAL,
            MAXIMAL,
            RATIO_HIGH,
            RATIO_LOW
        };

        struct Area {
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;
        };

        struct Field {
            Area area;
            const char* label   = nullptr;
            const char* value   = nullptr;
            Action action       = Action::NONE;
            uint16_t color      = ui::settings::themes::defaults::WHITE;
        };

        Field makeField(const char* label, Action action, uint16_t color);
        void makeFieldArea(Field &field, int x, int y, int w, int h);
        void updateField(ST7796S::MSP4021 &tft, const Field &field);
        bool isPressed(const Field &field, int tx, int ty);
        void drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap);
        void drawLine(ST7796S::MSP4021 &tft, const Field &field);

        uint32_t nextCapacity(uint32_t value);
        const char* capacityToText(uint32_t value);
        void actionCapacity(ST7796S::MSP4021 &tft, Field &field);

        float nextVoltage(float voltage);
        const char* voltageToText(float voltage, char* buffer, size_t size);
        void actionMinimal(ST7796S::MSP4021 &tft, Field &field);
        void actionMaximal(ST7796S::MSP4021 &tft, Field &field);
        void actionNominal(ST7796S::MSP4021 &tft, Field &field);

        uint8_t nextRatioHigh(uint8_t value);
        uint8_t previousRatioHigh(uint8_t value);
        const char* ratioToText(uint8_t value, char* buffer, size_t size);
        void actionRatio(ST7796S::MSP4021 &tft, Field &field);

        char capacityValue[16]  = "";
        char minimalValue[16]   = "";
        char nominalValue[16]   = "";
        char maximalValue[16]   = "";
        char ratioHighValue[16] = "";
        char ratioLowValue[16]  = "";

        Field capacityField     = makeField("Capacity",     Action::CAPACITY,   ui::settings::themes::defaults::GREEN);
        Field minimalField      = makeField("Minimal",      Action::MINIMAL,    ui::settings::themes::defaults::GREEN);
        Field nominalField      = makeField("Nominal",      Action::NOMINAL,    ui::settings::themes::defaults::GREEN);
        Field maximalField      = makeField("Maximal",      Action::MAXIMAL,    ui::settings::themes::defaults::GREEN);
        Field ratioHighField    = makeField("Ratio high",   Action::RATIO_HIGH, ui::settings::themes::defaults::GREEN);
        Field ratioLowField     = makeField("Ratio low",    Action::RATIO_LOW,  ui::settings::themes::defaults::GREEN);

        Field* fields[] = {
            &capacityField,
            &minimalField,
            &nominalField,
            &maximalField,
            &ratioHighField,
            &ratioLowField
        };

        Field makeField(const char* label, Action action, uint16_t color) {
            Field field;
            field.label     = label;
            field.action    = action;
            field.color     = color;
            return field;
        }

        void makeFieldArea(Field &field, int x, int y, int w, int h) {
            Area area;
            area.x = x;
            area.y = y;
            area.w = w;
            area.h = h;
            field.area = area;
        }

        void updateField(ST7796S::MSP4021 &tft, const Field &field) {
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

        bool isPressed(const Field &field, int tx, int ty) {
            return
                tx >= field.area.x && tx < field.area.x + field.area.w &&
                ty >= field.area.y && ty < field.area.y + field.area.h;
        }

        void drawTitle(ST7796S::MSP4021 &tft, int x, int y, int w, int h, int gap) {
            tft.setFont(ST7796S::RobotoMono_Bold_16);
            tft.setTextColor(ui::settings::themes::defaults::GREEN);
            tft.textCenterLeft(x, y, w, h, "BATTERY");
            tft.lineH(x, y + h + gap, w, ui::settings::themes::defaults::BORDER);
        }

        void drawLine(ST7796S::MSP4021 &tft, const Field &field) {
            const int gap = ui::settings::mockup::GAP;

            tft.setFont(ST7796S::RobotoMono_Regular_14);
            tft.setTextColor(ui::settings::themes::defaults::GREY);
            tft.textCenterLeft(
                field.area.x,        field.area.y,
                field.area.w / 2,    field.area.h,
                field.label
            );

            tft.setTextColor(field.color);
            tft.textCenterRight(
                field.area.x + (field.area.w / 2),  field.area.y,
                (field.area.w / 2) - gap,           field.area.h,
                field.value
            );
        }

        uint32_t nextCapacity(uint32_t value) {
            value += 500;
            if (value > 10000)
                { return 500; }
            return value;
        }

        const char* capacityToText(uint32_t value) {
            std::snprintf(
                capacityValue,
                sizeof(capacityValue),
                "%lu mAh",
                static_cast<unsigned long>(value)
            );
            return capacityValue;
        }

        void actionCapacity(ST7796S::MSP4021 &tft, Field &field) {
            uint32_t capacity = nextCapacity(services::settings::getBatteryCapacity());
            services::settings::setBatteryCapacity(capacity);
            field.value = capacityToText(capacity);
            updateField(tft, field);
        }

        float nextVoltage(float voltage) {
            voltage += 0.10f;
            if (voltage > 4.30f)
                { return 2.50f; }
            return voltage;
        }

        const char* voltageToText(float voltage, char* buffer, size_t size) {
            std::snprintf(buffer, size, "%.2f V", voltage);
            return buffer;
        }

        void actionMinimal(ST7796S::MSP4021 &tft, Field &field) {
            float minimal = nextVoltage(services::settings::getBatteryMinimal());
            services::settings::setBatteryMinimal(minimal);
            field.value = voltageToText(minimal, minimalValue, sizeof(minimalValue));
            updateField(tft, field);
        }

        void actionNominal(ST7796S::MSP4021 &tft, Field &field) {
            float nominal = nextVoltage(services::settings::getBatteryNominal());
            services::settings::setBatteryNominal(nominal);
            field.value = voltageToText(nominal, nominalValue, sizeof(nominalValue));
            updateField(tft, field);
        }

        void actionMaximal(ST7796S::MSP4021 &tft, Field &field) {
            float maximal = nextVoltage(services::settings::getBatteryMaximal());
            services::settings::setBatteryMaximal(maximal);
            field.value = voltageToText(maximal, maximalValue, sizeof(maximalValue));
            updateField(tft, field);
        }

        uint8_t nextRatioHigh(uint8_t value) {
            if (value >= 99)
                { return 1; }
            return value + 1;
        }

        uint8_t previousRatioHigh(uint8_t value) {
            if (value <= 1)
                { return 99; }
            return value - 1;
        }

        const char* ratioToText(uint8_t value, char* buffer, size_t size) {
            std::snprintf(buffer, size, "%u %%", value);
            return buffer;
        }

        void actionRatio(ST7796S::MSP4021 &tft, Field &field) {
            uint8_t value = services::settings::getBatteryRatioHigh();

            if (field.action == Action::RATIO_HIGH)
                { value = nextRatioHigh(value); }
            else { value = previousRatioHigh(value); }
            services::settings::setBatteryRatioHigh(value);

            ratioHighField.value = ratioToText(value,       ratioHighValue, sizeof(ratioHighValue));
            ratioLowField.value  = ratioToText(100 - value, ratioLowValue,  sizeof(ratioLowValue));

            updateField(tft, ratioHighField);
            updateField(tft, ratioLowField);
        }
    }

    bool isEditing()    { return mode == Mode::KEYBOARD; }
    void reset()        { mode = Mode::GRID; }

    void draw(ST7796S::MSP4021 &tft) {
        screens::mockup::grid::draw(tft);

        const int gap   = ui::settings::mockup::GAP;
        const int x     = screens::mockup::grid::innerX()       + (gap * 2);
        const int y     = screens::mockup::grid::innerY()       + (gap * 2);
        const int w     = screens::mockup::grid::innerWidth()   - (gap * 4);
        const int rowH  = 28;

        int rowY = y + rowH + (gap * 3);
        for (Field* field : fields) {
            makeFieldArea(*field, x, rowY, w, rowH);
            rowY += rowH;
        }

        const uint32_t capacity = services::settings::getBatteryCapacity();
        const float minimal     = services::settings::getBatteryMinimal();
        const float nominal     = services::settings::getBatteryNominal();
        const float maximal     = services::settings::getBatteryMaximal();
        const uint8_t ratioHigh = services::settings::getBatteryRatioHigh();

        capacityField.value     = capacityToText(capacity);
        minimalField.value      = voltageToText(minimal,        minimalValue,   sizeof(minimalValue));
        nominalField.value      = voltageToText(nominal,        nominalValue,   sizeof(nominalValue));
        maximalField.value      = voltageToText(maximal,        maximalValue,   sizeof(maximalValue));
        ratioHighField.value    = ratioToText(ratioHigh,        ratioHighValue, sizeof(ratioHighValue));
        ratioLowField.value     = ratioToText(100 - ratioHigh,  ratioLowValue,  sizeof(ratioLowValue));

        drawTitle(tft, x, y, w, rowH, gap);
        for (Field* field : fields)
            { drawLine(tft, *field); }
    }

    void update(ST7796S::MSP4021 &tft) {}
    bool handleTouch(ST7796S::MSP4021 &tft, int x, int y) {
        for (Field* field : fields) {
            if (!isPressed(*field, x, y))
                { continue; }
            switch (field->action) {
                case Action::CAPACITY:
                    actionCapacity(tft, *field);
                    return true;
                case Action::MINIMAL:
                    actionMinimal(tft, *field);
                    return true;
                case Action::NOMINAL:
                    actionNominal(tft, *field);
                    return true;
                case Action::MAXIMAL:
                    actionMaximal(tft, *field);
                    return true;
                case Action::RATIO_HIGH:
                    actionRatio(tft, *field);
                    return true;
                case Action::RATIO_LOW:
                    actionRatio(tft, *field);
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }
}