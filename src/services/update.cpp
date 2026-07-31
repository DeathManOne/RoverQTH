/*
 * src/services/update.cpp
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
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

#include "services/storage.h"
#include "services/update.h"
#include "services/wifi.h"
#include "utilities/hash.h"
#include "utilities/text.h"
#include "utilities/version.h"

namespace update   = services::update;
namespace storage  = services::storage;
namespace wifi     = services::wifi;
namespace hash     = utilities::hash;
namespace text     = utilities::text;
namespace uVersion = utilities::version;

namespace {
    constexpr size_t DOWNLOAD_BUFFER_SIZE = 4096;
    constexpr uint32_t HTTP_TIMEOUT_MS    = 15000;
    constexpr uint32_t STREAM_TIMEOUT_MS  = 20000;

    update::Status _status = update::Status::IDLE;
    portMUX_TYPE _lock     = portMUX_INITIALIZER_UNLOCKED;
    uint8_t _progress      = 0;
    uint32_t _expectedSize = 0;
    bool _taskRunning      = false;

    char _latestVersion[update::VERSION_SIZE]    {};
    char _expectedSha256[hash::SHA256_TEXT_SIZE] {};
    char _error[update::ERROR_SIZE]              {};

    void _setStatus(update::Status value);
    void _setProgress(uint8_t value);
    void _setError(const char* value, const char* logCode);
    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* url);
    void _finishTask();
    void _checkTask(void*);
    void _installTask(void*);
    bool _startTask(TaskFunction_t function, const char* name, uint32_t stackSize, update::Status initialStatus);

    void _setStatus(update::Status value) {
        portENTER_CRITICAL(&_lock);
        _status = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setProgress(uint8_t value) {
        if (value > 100) { value = 100; }
        portENTER_CRITICAL(&_lock);
        _progress = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setError(const char* value, const char* logCode) {
        portENTER_CRITICAL(&_lock);

        text::copy(_error, sizeof(_error), value);
        _status = update::Status::ERROR;

        portEXIT_CRITICAL(&_lock);
        if (logCode != nullptr)
            { storage::appendErrorRecord(logCode); }
    }

    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* url) {
        client.setInsecure();
        client.setTimeout(HTTP_TIMEOUT_MS);

        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setTimeout(HTTP_TIMEOUT_MS);

        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        if (!http.begin(client, url))
            { return false; }
        const int httpCode = http.GET();

        return httpCode == HTTP_CODE_OK;
    }

    void _finishTask() {
        portENTER_CRITICAL(&_lock);
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);
        vTaskDelete(nullptr);
    }

    void _checkTask(void*) {
        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_FIRMWARE_MANIFEST)) {
            http.end();
            _setError("Manifest unavailable", "OTA_MANIFEST_HTTP_FAILED");
            _finishTask();
            return;
        }

        StaticJsonDocument<384> document;
        const DeserializationError jsonError = deserializeJson(document, http.getStream());
        http.end();

        if (jsonError) {
            _setError("Invalid manifest", "OTA_MANIFEST_INVALID");
            _finishTask();
            return;
        }

        const char* const remoteVersion = document["version"].as<const char*>();
        const char* const sha256        = document["sha256"].as<const char*>();
        const uint32_t size             = document["size"] | 0U;

        uVersion::Comparison comparison;
        if (
            !uVersion::compare(remoteVersion, PROJECT_VERSION, comparison) ||
            size == 0                                                      ||
            !hash::isSha256Text(sha256)
        ) {
            _setError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            _finishTask();
            return;
        }

        char normalizedSha[hash::SHA256_TEXT_SIZE];
        if (!hash::normalizeSha256Text(sha256, normalizedSha, sizeof(normalizedSha))) {
            _setError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            _finishTask();
            return;
        }

        const bool updateAvailable = comparison == uVersion::Comparison::NEWER;
        portENTER_CRITICAL(&_lock);
    
        text::copy(_latestVersion,  sizeof(_latestVersion),  remoteVersion);
        text::copy(_expectedSha256, sizeof(_expectedSha256), normalizedSha);

        _expectedSize = size;
        _progress     = 0;
        _error[0]     = '\0';
        _status       = updateAvailable
            ? update::Status::AVAILABLE
            : update::Status::UP_TO_DATE;

        portEXIT_CRITICAL(&_lock);
        storage::appendLogRecord(updateAvailable ? "OTA_UPDATE_AVAILABLE" : "OTA_UP_TO_DATE");
        _finishTask();
    }

    void _installTask(void*) {
        uint32_t expectedSize = 0;
        char expectedSha[hash::SHA256_TEXT_SIZE];

        portENTER_CRITICAL(&_lock);
        expectedSize = _expectedSize;
        text::copy(expectedSha, sizeof(expectedSha), _expectedSha256);
        portEXIT_CRITICAL(&_lock);

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_FIRMWARE)) {
            http.end();
            _setError("Firmware unavailable", "OTA_FIRMWARE_HTTP_FAILED");
            _finishTask();
            return;
        }

        const int announcedSize = http.getSize();
        if (announcedSize > 0 && static_cast<uint32_t>(announcedSize) != expectedSize) {
            http.end();
            _setError("Wrong file size", "OTA_SIZE_HEADER_MISMATCH");
            _finishTask();
            return;
        }

        if (!::Update.begin(expectedSize, U_FLASH)) {
            http.end();
            _setError("OTA space unavailable", "OTA_BEGIN_FAILED");
            _finishTask();
            return;
        }

        hash::Sha256 sha256;
        if (!sha256.begin()) {
            Update.abort();
            http.end();
            _setError("SHA init failed", "OTA_SHA_INIT_FAILED");
            _finishTask();
            return;
        }

        uint8_t buffer[DOWNLOAD_BUFFER_SIZE];
        WiFiClient* stream  = http.getStreamPtr();
        uint32_t received   = 0;
        uint32_t lastDataAt = millis();
        bool transferOk     = true;

        while (received < expectedSize) {
            const int available = stream->available();

            if (available <= 0) {
                if (!http.connected() || millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    transferOk = false;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            size_t toRead = static_cast<size_t>(available);
            if (toRead > sizeof(buffer)) { toRead = sizeof(buffer); }

            const uint32_t remaining = expectedSize - received;
            if (toRead > remaining) { toRead = remaining; }

            const int read = stream->readBytes(buffer, toRead);
            if (read <= 0) { continue; }

            lastDataAt = millis();
            if (!sha256.update(buffer, static_cast<size_t>(read))) {
                transferOk = false;
                break;
            }

            if (::Update.write(buffer, static_cast<size_t>(read)) != static_cast<size_t>(read)) {
                transferOk = false;
                break;
            }

            received += static_cast<uint32_t>(read);
            _setProgress(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / expectedSize));
        }

        http.end();
        if (!transferOk || received != expectedSize) {
            ::Update.abort();
            _setError("Download interrupted", "OTA_DOWNLOAD_INCOMPLETE");
            _finishTask();
            return;
        }

        _setStatus(update::Status::VERIFYING);
        char calculatedSha[hash::SHA256_TEXT_SIZE];
        if (!sha256.finish(calculatedSha, sizeof(calculatedSha))) {
            ::Update.abort();
            _setError("SHA check failed", "OTA_SHA_FINISH_FAILED");
            _finishTask();
            return;
        }

        if (!text::equals(calculatedSha, expectedSha)) {
            ::Update.abort();
            _setError("SHA-256 mismatch", "OTA_SHA_MISMATCH");
            _finishTask();
            return;
        }

        _setStatus(update::Status::INSTALLING);
        if (!::Update.end(true)) {
            _setError("Firmware rejected", "OTA_END_FAILED");
            _finishTask();
            return;
        }

        _setProgress(100);
        _setStatus(update::Status::SUCCESS);

        storage::appendLogRecord("OTA_UPDATE_SUCCESS");
        vTaskDelay(pdMS_TO_TICKS(1500));
        ESP.restart();
    }

    bool _startTask(TaskFunction_t function, const char* name, uint32_t stackSize, update::Status initialStatus) {
        portENTER_CRITICAL(&_lock);

        if (_taskRunning) {
            portEXIT_CRITICAL(&_lock);
            return false;
        }

        _taskRunning = true;
        _progress    = 0;
        _error[0]    = '\0';
        _status      = initialStatus;

        portEXIT_CRITICAL(&_lock);

        const BaseType_t result = xTaskCreate(function, name, stackSize, nullptr, 1, nullptr);
        if (result != pdPASS) {
            portENTER_CRITICAL(&_lock);
            _taskRunning = false;
            portEXIT_CRITICAL(&_lock);

            _setError("Task creation failed", "OTA_TASK_CREATE_FAILED");
            return false;
        }
        return true;
    }
}

bool update::begin() {
    portENTER_CRITICAL(&_lock);
    _status            = Status::IDLE;
    _progress          = 0;
    _latestVersion[0]  = '\0';
    _expectedSha256[0] = '\0';
    _error[0]          = '\0';
    _expectedSize      = 0;
    _taskRunning       = false;
    portEXIT_CRITICAL(&_lock);
    return true;
}

bool update::checkUpdate() {
    if (!wifi::isConnected()) {
        _setError("WiFi not connected", "OTA_WIFI_NOT_CONNECTED");
        return false;
    }
    return _startTask(_checkTask, "OTA check", 8192, Status::CHECKING);
}

bool update::startUpdate() {
    if (!wifi::isConnected()) {
        _setError("WiFi not connected", "OTA_WIFI_NOT_CONNECTED");
        return false;
    }

    portENTER_CRITICAL(&_lock);
    const bool available = !_taskRunning && _status == Status::AVAILABLE;
    portEXIT_CRITICAL(&_lock);

    if (!available) { return false; }
    return _startTask(_installTask, "OTA install", 12288, Status::DOWNLOADING);
}

bool update::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);
    return busy;
}

update::Snapshot update::snapshot() {
    Snapshot value;
    portENTER_CRITICAL(&_lock);
    value.status   = _status;
    value.progress = _progress;
    text::copy(value.latestVersion, sizeof(value.latestVersion), _latestVersion);
    text::copy(value.error,         sizeof(value.error),         _error);
    portEXIT_CRITICAL(&_lock);
    return value;
}

const char* update::currentVersion() {
    return PROJECT_VERSION;
}
