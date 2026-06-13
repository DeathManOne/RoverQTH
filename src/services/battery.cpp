#include <Arduino.h>
#include "services/battery.h"

namespace services::battery {
    namespace {
        uint8_t batteryPin  = 0;
        uint8_t percent     = 0;
        float voltage       = 0.0f;

        constexpr float ADC_VREF        = 3.3f;
        constexpr int ADC_MAX           = 4095;
        constexpr float DIVIDER_RATIO   = 2.0f;
        constexpr float BATTERY_MIN     = 3.30f;
        constexpr float BATTERY_MAX     = 4.20f;
    }

    float   getVoltage() { return voltage; }
    uint8_t getPercent() { return percent; }
    bool    isPresent()  { return voltage > 2.5f; }

    bool begin(uint8_t pin) {
        batteryPin = pin;
        pinMode(batteryPin, INPUT);

        analogReadResolution(12);
        #ifdef ESP32
            analogSetPinAttenuation(batteryPin, ADC_11db);
        #endif
        return true;
    }

    bool update() {
        const int raw = analogRead(batteryPin);
        voltage = ((static_cast<float>(raw) / ADC_MAX) * ADC_VREF * DIVIDER_RATIO);
        if (voltage <= BATTERY_MIN) { percent = 0; }
        else if (voltage >= BATTERY_MAX) { percent = 100; }
        else { percent = static_cast<uint8_t>(((voltage - BATTERY_MIN) * 100.0f) / (BATTERY_MAX - BATTERY_MIN)); }
        return true;
    }
}