#pragma once

#include <cstdint>

namespace services::display {
    enum class Button {MARK_QTH, OTA, MENU, COUNT};
    enum class ButtonState {UNAVAILABLE, READY, RUNNING};

    void begin();
    void start();
    void shutdown();

    void update(uint32_t &nextRefreshIn);
    void handleTouch();

    ButtonState buttonState(Button button);
    void setButtonState(Button button, ButtonState state);

    bool readTouch(int &x, int &y);

    void clearBoot();
    void drawBoot();
    void drawBootLogo();
    void updateBootWifi(bool* value);
    void updateBootSD(bool* value);
    void updateBootGPS(bool* value);
    void updateBootGPSProgress(uint8_t progress);
    bool handleBootTouch(int x, int y);
}
