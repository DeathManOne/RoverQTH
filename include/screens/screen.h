#pragma once

#include <cstdint>

#include <MSP4021.h>

namespace screens {
    class Screen {
        public:
            virtual ~Screen() = default;

            virtual void preload() = 0;
            virtual void draw(ST7796S::MSP4021 &tft) = 0;

            virtual void update(
                ST7796S::MSP4021 &tft,
                uint32_t &nextRefreshIn
            ) = 0;

            virtual bool handleTouch(
                ST7796S::MSP4021 &tft,
                int x,
                int y
            ) = 0;

            virtual bool isEditing() const {
                return false;
            }
    };
}
