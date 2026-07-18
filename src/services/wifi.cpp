/*
 * src/services/wifi.cpp
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

#include <Arduino.h>
#include <cstdio>
#include <WiFi.h>

#include "services/storage.h"
#include "services/wifi.h"

namespace storage = services::storage;
namespace wifi    = services::wifi;

namespace {
    bool _initialized = false;
    bool _connecting  = false;

    uint32_t _connectionStartedAt = 0;
    uint32_t _connectionTimeoutMs = 0;

    char _requestedSSID[33] {};

    void _resetConnectionState() {
        _connecting          = false;
        _connectionStartedAt = 0;
        _connectionTimeoutMs = 0;
    }
}

bool wifi::begin() {
    if (_initialized)         { return true; }
    if (!WiFi.mode(WIFI_STA)) { return false; }

    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    _resetConnectionState();
    _requestedSSID[0] = '\0';
    _initialized      = true;
    return true;
}

bool wifi::isInitialized() { return _initialized; }
bool wifi::isConnected  () { return (_initialized && WiFi.status() == WL_CONNECTED); }
bool wifi::isConnecting () { return (_initialized && _connecting); }

bool wifi::connect(const char* ssid, const char* password, uint32_t timeoutSec) {
    if (!_initialized || !ssid || ssid[0] == '\0' || timeoutSec == 0) { return false; }
    if (!password) { password = ""; }

    const int written = std::snprintf(_requestedSSID, sizeof(_requestedSSID), "%s", ssid);

    if (written <= 0 || static_cast<size_t>(written) >= sizeof(_requestedSSID)) {
        _requestedSSID[0] = '\0';
        return false;
    }
    
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    _connectionStartedAt = millis();
    _connectionTimeoutMs = timeoutSec * 1000;
    _connecting          = true;

    storage::appendLogRecord("WIFI_CONNECTING");
    return true;
}

void wifi::update() {
    if (!_initialized || !_connecting) { return; }
    if (WiFi.status() == WL_CONNECTED) {
        _resetConnectionState();
        storage::appendLogRecord("WIFI_CONNECTED");
        return;
    }

    if ((millis() - _connectionStartedAt) < _connectionTimeoutMs)
        { return; }
    WiFi.disconnect();
    _resetConnectionState();
    storage::appendErrorRecord("WIFI_CONNECTION_FAILED");
}

void wifi::disconnect() {
    if (!_initialized) { return; }

    const bool wasConnected = WiFi.status() == WL_CONNECTED;
    WiFi.disconnect();
    _resetConnectionState();

    if (wasConnected) { storage::appendLogRecord("WIFI_DISCONNECTED"); }
}

bool wifi::getSSID(char* buffer, size_t size) {
    if (!buffer || size == 0) { return false; }
    buffer[0] = '\0';

    if (isConnected()) {
        const String currentSSID = WiFi.SSID();
        if (currentSSID.length() == 0) { return false; }

        const int written = std::snprintf(buffer, size, "%s", currentSSID.c_str());
        return (written > 0 && static_cast<size_t>(written) < size);
    }

    if (_requestedSSID[0] == '\0') { return false; }
    const int written = std::snprintf(buffer, size, "%s", _requestedSSID);
    return (written > 0 && static_cast<size_t>(written) < size);
}

int32_t wifi::getRSSI() {
    if (!isConnected()) { return 0; }
    return WiFi.RSSI();
}