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

#include <cctype>
#include <cstdio>
#include <cstring>

#include <mbedtls/sha256.h>

#include "services/storage.h"
#include "services/update.h"
#include "services/wifi.h"

namespace update  = services::update;
namespace storage = services::storage;
namespace wifi    = services::wifi;

namespace {
    constexpr size_t VERSION_SIZE         = 16;
    constexpr size_t SHA256_TEXT_SIZE     = 65;
    constexpr size_t ERROR_SIZE           = 48;
    constexpr size_t DOWNLOAD_BUFFER_SIZE = 4096;
    constexpr uint32_t HTTP_TIMEOUT_MS    = 15000;
    constexpr uint32_t STREAM_TIMEOUT_MS  = 20000;

    update::Status _status    = update::Status::IDLE;
    portMUX_TYPE _lock     = portMUX_INITIALIZER_UNLOCKED;
    uint8_t _progress      = 0;
    uint32_t _expectedSize = 0;
    bool _taskRunning      = false;

    char _latestVersion[VERSION_SIZE] {};
    char _expectedSha256[SHA256_TEXT_SIZE] {};
    char _error[ERROR_SIZE] {};

    void _copyText(char* destination, size_t destinationSize, const char* source) {
        if (destination == nullptr || destinationSize == 0) { return; }

        if (source == nullptr)
            { source = ""; }
        std::snprintf(destination, destinationSize, "%s", source);
    }

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

        _copyText(_error, sizeof(_error), value);
        _status = update::Status::ERROR;

        portEXIT_CRITICAL(&_lock);
        if (logCode != nullptr)
            { storage::appendErrorRecord(logCode); }
    }

    bool _versionToBuild(const char* version, uint32_t& build) {
        if (version == nullptr || version[0] == '\0') { return false; }

        uint16_t parts[3] {0, 0, 0};
        uint8_t part   = 0;
        uint8_t digits = 0;

        for (size_t index = 0; version[index] != '\0'; ++index) {
            const unsigned char character = static_cast<unsigned char>(version[index]);

            if (character == '.') {
                if (digits == 0 || part >= 2) { return false; }
                ++part;
                digits = 0;
                continue;
            }

            if (!std::isdigit(character) || digits >= 3) { return false; }
            parts[part] = static_cast<uint16_t>(parts[part] * 10U + character - '0');
            ++digits;
        }

        if (part != 2 || digits == 0) { return false; }
        build =
            static_cast<uint32_t>(parts[0])   * 1000000UL
            + static_cast<uint32_t>(parts[1]) * 1000UL
            + static_cast<uint32_t>(parts[2]);
        return true;
    }

    bool _isSha256(const char* value) {
        if (value == nullptr || std::strlen(value) != 64) { return false; }
        for (size_t index = 0; index < 64; ++index) {
            if (!std::isxdigit(static_cast<unsigned char>(value[index])))
                { return false; }
        }
        return true;
    }

    void _normalizeSha256(char output[SHA256_TEXT_SIZE], const char* input) {
        for (size_t index = 0; index < 64; ++index)
            { output[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(input[index]))); }
        output[64] = '\0';
    }

    void _digestToText(const uint8_t hashBytes[32], char output[SHA256_TEXT_SIZE]) {
        static constexpr char HEX_CHARACTERS[] = "0123456789abcdef";

        for (size_t index = 0; index < 32; ++index) {
            const uint8_t value = hashBytes[index];
            output[index * 2]     = HEX_CHARACTERS[static_cast<size_t>(value >> 4)];
            output[index * 2 + 1] = HEX_CHARACTERS[static_cast<size_t>(value & 0x0F)];
        }
        output[64] = '\0';
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

        const char* version  = document["version"].as<const char*>();
        const char* sha256   = document["sha256"].as<const char*>();
        const uint32_t size  = document["size"] | 0U;
        uint32_t localBuild  = 0;
        uint32_t remoteBuild = 0;

        if (!_versionToBuild(PROJECT_VERSION, localBuild)  || !_versionToBuild(version, remoteBuild) || size == 0 || !_isSha256(sha256)) {
            _setError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            _finishTask();
            return;
        }

        char normalizedSha[SHA256_TEXT_SIZE];
        _normalizeSha256(normalizedSha, sha256);

        portENTER_CRITICAL(&_lock);
        _copyText(_latestVersion,  sizeof(_latestVersion),  version);
        _copyText(_expectedSha256, sizeof(_expectedSha256), normalizedSha);

        _expectedSize = size;
        _progress     = 0;
        _error[0]     = '\0';
        _status       = remoteBuild > localBuild ? update::Status::AVAILABLE : update::Status::UP_TO_DATE;

        portEXIT_CRITICAL(&_lock);
        storage::appendLogRecord(remoteBuild > localBuild ? "OTA_UPDATE_AVAILABLE" : "OTA_UP_TO_DATE");
        _finishTask();
    }

    void _installTask(void*) {
        uint32_t expectedSize = 0;

        char expectedSha[SHA256_TEXT_SIZE];
        portENTER_CRITICAL(&_lock);
        expectedSize = _expectedSize;

        _copyText(expectedSha, sizeof(expectedSha), _expectedSha256);
        portEXIT_CRITICAL(&_lock);

        WiFiClientSecure client;
        HTTPClient http;

        _setStatus(update::Status::DOWNLOADING);
        _setProgress(0);

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

        mbedtls_sha256_context shaContext;
        mbedtls_sha256_init(&shaContext);

        if (mbedtls_sha256_starts_ret(&shaContext, 0) != 0) {
            Update.abort();
            http.end();

            mbedtls_sha256_free(&shaContext);
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
            if (mbedtls_sha256_update_ret(&shaContext, buffer, static_cast<size_t>(read)) != 0) {
                transferOk = false;
                break;
            }

            if (
                ::Update.write(buffer, static_cast<size_t>(read)) != static_cast<size_t>(read)) {
                transferOk = false;
                break;
            }

            received += static_cast<uint32_t>(read);
            _setProgress(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / expectedSize));
        }

        http.end();
        if (!transferOk || received != expectedSize) {
            ::Update.abort();
            mbedtls_sha256_free(&shaContext);

            _setError("Download interrupted", "OTA_DOWNLOAD_INCOMPLETE");
            _finishTask();
            return;
        }

        _setStatus(update::Status::VERIFYING);
        uint8_t digest[32];

        if (mbedtls_sha256_finish_ret(&shaContext, digest) != 0) {
            ::Update.abort();
            mbedtls_sha256_free(&shaContext);

            _setError("SHA check failed", "OTA_SHA_FINISH_FAILED");
            _finishTask();
            return;
        }

        mbedtls_sha256_free(&shaContext);
        char calculatedSha[SHA256_TEXT_SIZE];

        _digestToText(digest, calculatedSha);
        if (std::strcmp(calculatedSha, expectedSha) != 0) {
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

    bool _startTask(TaskFunction_t function, const char* name, uint32_t stackSize) {
        portENTER_CRITICAL(&_lock);
        if (_taskRunning) {
            portEXIT_CRITICAL(&_lock);
            return false;
        }

        _taskRunning = true;
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
    if (isBusy()) { return false; }

    _setProgress(0);
    _setStatus(Status::CHECKING);
    return _startTask(_checkTask, "OTA check", 8192);
}

bool update::startUpdate() {
    if (!wifi::isConnected()) {
        _setError("WiFi not connected", "OTA_WIFI_NOT_CONNECTED");
        return false;
    }
    if (isBusy() || !isAvailable()) { return false; }
    return _startTask(_installTask, "OTA install", 12288);
}

bool update::isUpdating() {
    const Status current = status();
    return
        current == Status::DOWNLOADING ||
        current == Status::VERIFYING   ||
        current == Status::INSTALLING;
}

bool update::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);
    return busy;
}

bool update::isAvailable() {
    return status() == Status::AVAILABLE;
}

int update::progress() {
    portENTER_CRITICAL(&_lock);
    const int value = _progress;
    portEXIT_CRITICAL(&_lock);
    return value;
}

update::Status update::status() {
    portENTER_CRITICAL(&_lock);
    const Status value = _status;
    portEXIT_CRITICAL(&_lock);
    return value;
}

const char* update::currentVersion() {
    return PROJECT_VERSION;
}

bool update::getLatestVersion(char* buffer, size_t size) {
    if (buffer == nullptr || size == 0) { return false; }

    portENTER_CRITICAL(&_lock);
    _copyText(buffer, size, _latestVersion);

    const bool available = _latestVersion[0] != '\0';
    portEXIT_CRITICAL(&_lock);

    return available;
}

bool update::getError(char* buffer, size_t size) {
    if (buffer == nullptr || size == 0) { return false; }

    portENTER_CRITICAL(&_lock);
    _copyText(buffer, size, _error);

    const bool available = _error[0] != '\0';
    portEXIT_CRITICAL(&_lock);

    return available;
}