#pragma once

#include <cstdint>

namespace services::battery {
    bool begin(uint8_t pin);
    bool update();
    bool isPresent();

    float getVoltage();
    uint8_t getPercent();
}