#include <Arduino.h>
#include <array>
#include <cstddef>
#include <MSP4021.h>
#include <SPI.h>

#include "screens/boot.h"
#include "screens/main.h"
#include "screens/menu.h"
#include "screens/ota.h"
#include "services/display.h"
#include "services/navigation.h"
#include "services/ota.h"
#include "services/qth.h"
#include "services/settings.h"
#include "services/storage.h"
#include "ui/mockup/buttons.h"
#include "ui/settings/themes/defaults.h"
#include "ui/widgets/buttons.h"

namespace services::display {
    static void draw();

    static bool loadTouchCalibration();
    static bool calibrateTouch();
    static void applyTouchCalibration(settings::Calibration &calibration);
    static void readTouchCalibration(settings::Calibration &calibration);
    static void clear();

    static void updateMainMARK();

    static bool handleMenuTouch(int x, int y);
    static bool isMenuEditing();
    static void resetMenu();

    static void drawOta();
    static bool handleOtaTouch(int x, int y);
    static bool isOtaEditing();
}

namespace service  = services::display;
namespace state    = services::display;
namespace navigation = services::navigation;
namespace ota      = services::ota;
namespace qth      = services::qth;
namespace settings = services::settings;
namespace storage  = services::storage;
namespace theme    = ui::settings::themes::defaults;
namespace buttons  = ui::widgets::buttons;
namespace mockupButtons = ui::mockup::buttons;

namespace {
    enum class ScreenId {MAIN, MENU, OTA};

    SPIClass _tftSPI(FSPI);
    ST7796S::MSP4021* _tftInstance = nullptr;

    std::array<state::ButtonState, static_cast<size_t>(state::Button::COUNT)>
        _buttonStates {};

    screens::Boot _bootScreen;
    screens::Main _mainScreen;
    screens::Menu _menuScreen;
    screens::Ota _otaScreen;

    ScreenId _currentScreenId = ScreenId::MAIN;
    screens::Screen* _currentScreen = &_mainScreen;

    uint32_t _lastTouchMs = 0U;
    constexpr uint32_t TOUCH_DEBOUNCE_MS = 180U;

    ST7796S::MSP4021& _tft() { return *_tftInstance; }

    mockupButtons::Id _visualButtonId(const state::Button button) {
        switch (button) {
            case state::Button::MARK_QTH: return mockupButtons::Id::MARK_QTH;
            case state::Button::OTA:      return mockupButtons::Id::OTA;
            case state::Button::MENU:     return mockupButtons::Id::MENU;
            case state::Button::COUNT:
            default:                      return mockupButtons::Id::COUNT;
        }
    }

    mockupButtons::State _visualButtonState(const state::ButtonState value) {
        switch (value) {
            case state::ButtonState::READY:   return mockupButtons::State::READY;
            case state::ButtonState::RUNNING: return mockupButtons::State::RUNNING;
            case state::ButtonState::UNAVAILABLE:
            default:                          return mockupButtons::State::UNAVAILABLE;
        }
    }

    void _resetState() {
        _currentScreenId = ScreenId::MAIN;
        _currentScreen = &_mainScreen;
        _buttonStates.fill(state::ButtonState::UNAVAILABLE);

        mockupButtons::setState(
            mockupButtons::Id::MARK_QTH,
            mockupButtons::State::UNAVAILABLE
        );
        mockupButtons::setState(
            mockupButtons::Id::OTA,
            mockupButtons::State::UNAVAILABLE
        );
        mockupButtons::setState(
            mockupButtons::Id::MENU,
            mockupButtons::State::UNAVAILABLE
        );
    }

    void _setScreen(const ScreenId screen) {
        _currentScreenId = screen;
        switch (screen) {
            case ScreenId::MENU: _currentScreen = &_menuScreen; break;
            case ScreenId::OTA:  _currentScreen = &_otaScreen;  break;
            case ScreenId::MAIN:
            default:             _currentScreen = &_mainScreen; break;
        }
    }

    bool _touchDebounced() {
        const uint32_t now = millis();
        if ((now - _lastTouchMs) >= TOUCH_DEBOUNCE_MS) {
            _lastTouchMs = now;
            return false;
        }
        return true;
    }

    void _syncOtaAvailability() {
        if (state::buttonState(state::Button::OTA) != state::ButtonState::UNAVAILABLE)
            { return; }

        const bool summitsAvailable =
            ota::snapshot(ota::Type::SUMMITS).status != ota::Status::UNAVAILABLE;
        const bool parksAvailable =
            ota::snapshot(ota::Type::PARKS).status != ota::Status::UNAVAILABLE;

        if (!summitsAvailable && !parksAvailable) { return; }

        state::setButtonState(state::Button::OTA, state::ButtonState::READY);
        mockupButtons::updateOTA(_tft());
    }

    void _toggleMARK() {
        const navigation::MarkState markState = navigation::markState();

        if (markState == navigation::MarkState::IDLE) {
            if (!navigation::startMark()) { return; }
            if (!qth::saveTemporaryRecord()) {
                navigation::clearMark();
                storage::appendErrorRecord("QTH_TEMP_CREATE_FAILED");
                return;
            }

            if (!qth::resetTrace()) {
                qth::discardTemporaryRecord();
                navigation::clearMark();
                storage::appendErrorRecord("QTH_TRACE_CREATE_FAILED");
                return;
            }

            storage::appendLogRecord("QTH_RECORDING_STARTED");
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
            mockupButtons::updateMARK(_tft());
            service::updateMainMARK();
            return;
        }

        if (markState == navigation::MarkState::RECORDING && !navigation::stopMark())
            { return; }

        if (navigation::markState() != navigation::MarkState::READY_TO_SAVE)
            { return; }

        if (!qth::isCurrentRecordLongEnough()) {
            if (!qth::discardTemporaryRecord())
                { storage::appendErrorRecord("QTH_TEMP_DELETE_FAILED"); }
            if (!qth::discardTemporaryTrace())
                { storage::appendErrorRecord("QTH_TRACE_DELETE_FAILED"); }
            storage::appendLogRecord("QTH_RECORDING_DISCARDED");
            navigation::clearMark();
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::READY);
            mockupButtons::updateMARK(_tft());
            service::updateMainMARK();
            return;
        }

        navigation::MarkSnapshot snapshot {};
        if (!navigation::getMarkSnapshot(snapshot) || !snapshot.hasEnd) {
            storage::appendErrorRecord("QTH_FINAL_POINT_UNAVAILABLE");
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
            mockupButtons::updateMARK(_tft());
            service::updateMainMARK();
            return;
        }

        qth::TracePoint finalPoint {};
        finalPoint.utc       = snapshot.stopUTC;
        finalPoint.latitude  = snapshot.end.latitude;
        finalPoint.longitude = snapshot.end.longitude;
        finalPoint.altitude  = snapshot.end.altitude;

        if (!qth::appendFinalTracePoint(finalPoint)) {
            storage::appendErrorRecord("QTH_FINAL_POINT_APPEND_FAILED");
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
            mockupButtons::updateMARK(_tft());
            service::updateMainMARK();
            return;
        }

        if (!qth::saveCurrentRecord()) {
            state::setButtonState(state::Button::MARK_QTH, state::ButtonState::RUNNING);
            mockupButtons::updateMARK(_tft());
            service::updateMainMARK();
            return;
        }

        storage::appendLogRecord("QTH_RECORDING_SAVED");
        navigation::clearMark();
        state::setButtonState(state::Button::MARK_QTH, state::ButtonState::READY);
        mockupButtons::updateMARK(_tft());
        service::updateMainMARK();
    }

    bool _handleMainTouch(int x, int y) {
        if (state::buttonState(state::Button::MARK_QTH) != state::ButtonState::UNAVAILABLE &&
            buttons::isPressed(buttons::markQTH, x, y)
        ) {
            _toggleMARK();
            return true;
        }

        if (state::buttonState(state::Button::OTA) != state::ButtonState::UNAVAILABLE &&
            buttons::isPressed(buttons::ota, x, y)
        ) {
            state::setButtonState(state::Button::OTA, state::ButtonState::RUNNING);
            _setScreen(ScreenId::OTA);
            service::draw();
            return true;
        }

        if (state::buttonState(state::Button::MENU) != state::ButtonState::UNAVAILABLE &&
            buttons::isPressed(buttons::menu, x, y)
        ) {
            state::setButtonState(state::Button::MENU, state::ButtonState::RUNNING);
            _setScreen(ScreenId::MENU);
            service::draw();
            return true;
        }

        return false;
    }

    bool _handleMenuTouch(int x, int y) {
        if (service::isMenuEditing())
            { return service::handleMenuTouch(x, y); }

        if (state::buttonState(state::Button::OTA) != state::ButtonState::UNAVAILABLE &&
            buttons::isPressed(buttons::ota, x, y)
        ) {
            service::resetMenu();
            state::setButtonState(state::Button::MENU, state::ButtonState::READY);
            state::setButtonState(state::Button::OTA, state::ButtonState::RUNNING);
            _setScreen(ScreenId::OTA);
            service::draw();
            return true;
        }

        if (buttons::isPressed(buttons::menu, x, y)) {
            service::resetMenu();
            state::setButtonState(state::Button::MENU, state::ButtonState::READY);
            _setScreen(ScreenId::MAIN);
            service::draw();
            return true;
        }

        return service::handleMenuTouch(x, y);
    }

    bool _handleOtaTouch(int x, int y) {
        if (service::isOtaEditing()) {
            const bool handled = service::handleOtaTouch(x, y);
            if (handled && !service::isOtaEditing()) {
                service::clear();
                service::drawOta();
            }
            return true;
        }

        if (buttons::isPressed(buttons::ota, x, y)) {
            state::setButtonState(state::Button::OTA, state::ButtonState::READY);
            _setScreen(ScreenId::MAIN);
            service::draw();
            return true;
        }

        if (state::buttonState(state::Button::MENU) != state::ButtonState::UNAVAILABLE &&
            buttons::isPressed(buttons::menu, x, y)
        ) {
            state::setButtonState(state::Button::OTA, state::ButtonState::READY);
            state::setButtonState(state::Button::MENU, state::ButtonState::RUNNING);
            _setScreen(ScreenId::MENU);
            service::draw();
            return true;
        }

        return service::handleOtaTouch(x, y);
    }
}

void service::begin() {
    _resetState();
    _tftSPI.begin(TFT_CLK, TFT_MISO, TFT_MOSI);

    if (_tftInstance == nullptr) {
        _tftInstance = new ST7796S::MSP4021(
            _tftSPI,
            TFT_TOUCH_CS,
            TFT_SCREEN_CS,
            TFT_SCREEN_DC,
            TFT_WIDTH,
            TFT_HEIGHT,
            TFT_SCREEN_RST
        );
    }

    _tft().setRotation(static_cast<uint8_t>(settings::getTFTRotation()));

    if (loadTouchCalibration()) { return; }
    while (!calibrateTouch()) { delay(10); }
}

service::ButtonState service::buttonState(const Button button) {
    const size_t index = static_cast<size_t>(button);
    if (index >= _buttonStates.size()) { return ButtonState::UNAVAILABLE; }
    return _buttonStates[index];
}

void service::setButtonState(const Button button, const ButtonState state) {
    const size_t index = static_cast<size_t>(button);
    if (index >= _buttonStates.size()) { return; }

    switch (state) {
        case ButtonState::UNAVAILABLE:
        case ButtonState::READY:
        case ButtonState::RUNNING:
            break;
        default:
            return;
    }

    _buttonStates[index] = state;
    mockupButtons::setState(
        _visualButtonId(button),
        _visualButtonState(state)
    );
}

void service::shutdown() {
    if (_tftInstance == nullptr) { return; }
    clear();
    _tft().shutdown();
}

bool service::loadTouchCalibration() {
    settings::Calibration calibration;
    if (!settings::getTouchCalibration(calibration)) { return false; }

    applyTouchCalibration(calibration);
    return true;
}

bool service::calibrateTouch() {
    settings::Calibration normal;
    _tft().setRotation(static_cast<uint8_t>(settings::TFTRotation::NORMAL));
    if (_tft().TCalibrate()) { readTouchCalibration(normal); }
    else { return false; }

    settings::Calibration reversed;
    _tft().setRotation(static_cast<uint8_t>(settings::TFTRotation::REVERSED));
    if (_tft().TCalibrate()) { readTouchCalibration(reversed); }
    else { return false; }

    _tft().setRotation(static_cast<uint8_t>(settings::getTFTRotation()));
    if (!settings::setTouchCalibration(normal, reversed)) { return false; }

    storage::appendLogRecord("TOUCH_CALIBRATION_SAVED");
    return loadTouchCalibration();
}

void service::applyTouchCalibration(settings::Calibration &calibration) {
    _tft().TCalibrate(
        calibration.swapXY,
        calibration.invertX,
        calibration.invertY,
        calibration.coeffXA,
        calibration.coeffXB,
        calibration.coeffXC,
        calibration.coeffYA,
        calibration.coeffYB,
        calibration.coeffYC
    );
}

void service::readTouchCalibration(settings::Calibration &calibration) {
    _tft().TCalibrateInfo(
        calibration.swapXY,
        calibration.invertX,
        calibration.invertY,
        calibration.coeffXA,
        calibration.coeffXB,
        calibration.coeffXC,
        calibration.coeffYA,
        calibration.coeffYB,
        calibration.coeffYC
    );
}

bool service::readTouch(int &x, int &y) {
    return _tft().TRead(x, y);
}

void service::clear() {
    if (_tftInstance == nullptr) { return; }
    _tft().fillScreen(theme::BLACK);
}

void service::updateMainMARK() { _mainScreen.updateMARK(_tft()); }

bool service::handleMenuTouch(int x, int y) {
    const bool handled = _menuScreen.handleTouch(_tft(), x, y);

    switch (_menuScreen.takeDisplayRequest()) {
        case screens::menu::Displayer::Request::RELOAD_TOUCH_CALIBRATION:
            service::loadTouchCalibration();
            service::clear();
            _menuScreen.draw(_tft());
            break;
        case screens::menu::Displayer::Request::RECALIBRATE_TOUCH:
            while (!service::calibrateTouch()) { delay(10); }
            service::clear();
            _menuScreen.draw(_tft());
            break;
        case screens::menu::Displayer::Request::NONE:
        default:
            break;
    }

    return handled;
}
bool service::isMenuEditing() { return _menuScreen.isEditing(); }
void service::resetMenu() { _menuScreen.reset(); }

void service::drawOta() { _otaScreen.draw(_tft()); }
bool service::handleOtaTouch(int x, int y) {
    return _otaScreen.handleTouch(_tft(), x, y);
}
bool service::isOtaEditing() { return _otaScreen.isEditing(); }

void service::clearBoot() { _bootScreen.clear(_tft()); }
void service::drawBoot() { _bootScreen.draw(_tft()); }
void service::drawBootLogo() { _bootScreen.drawLogo(_tft()); }
void service::updateBootWifi(bool* value) { _bootScreen.updateWifi(_tft(), value); }
void service::updateBootSD(bool* value) { _bootScreen.updateSD(_tft(), value); }
void service::updateBootGPS(bool* value) { _bootScreen.updateGPS(_tft(), value); }
void service::updateBootGPSProgress(uint8_t progress) {
    _bootScreen.updateGPSProgress(_tft(), progress);
}
bool service::handleBootTouch(int x, int y) {
    return _bootScreen.handleTouch(x, y);
}

void service::start() { service::draw(); }

void service::draw() {
    _currentScreen->preload();
    _currentScreen->draw(_tft());
}

void service::update(uint32_t &nextRefreshIn) {
    _syncOtaAvailability();
    _currentScreen->update(_tft(), nextRefreshIn);
}

void service::handleTouch() {
    int x, y;
    if (!service::readTouch(x, y) || _touchDebounced()) { return; }

    switch (_currentScreenId) {
        case ScreenId::MAIN: _handleMainTouch(x, y); break;
        case ScreenId::MENU: _handleMenuTouch(x, y); break;
        case ScreenId::OTA: _handleOtaTouch(x, y); break;
        default: break;
    }
}
