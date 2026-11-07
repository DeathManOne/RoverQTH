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
#include <HTTPClient.h>
#include <limits>
#include <Update.h>
#include <WiFiClientSecure.h>

#include "database/ota.h"
#include "database/ota/parks.h"
#include "database/ota/summits.h"
#include "security/otaRootCA.h"
#include "services/ota.h"
#include "services/storage.h"
#include "services/update.h"
#include "services/wifi.h"
#include "utilities/hash.h"
#include "utilities/json/reader.h"
#include "utilities/ota/summits.h"
#include "utilities/text.h"
#include "utilities/version.h"

namespace parksDB   = database::ota::parks;
namespace summitsDB = database::ota::summits;
namespace otaRootCA = security::otaRootCA;
namespace ota       = services::ota;
namespace storage   = services::storage;
namespace update    = services::update;
namespace wifi      = services::wifi;
namespace hash      = utilities::hash;
namespace json      = utilities::json;
namespace summits   = utilities::ota::summits;
namespace text      = utilities::text;
namespace version   = utilities::version;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr size_t DOWNLOAD_BUFFER_SIZE = 4096;
    constexpr size_t MANIFEST_BUFFER_SIZE = 512U;
    constexpr uint32_t HTTP_TIMEOUT_MS    = 15000;
    constexpr uint32_t STREAM_TIMEOUT_MS  = 20000;
    constexpr const char* POTA_CSV_PATH   = "/RoverQTH/tmp/pota.tmp";
    constexpr const char* SOTA_CSV_PATH   = "/RoverQTH/tmp/sota.tmp";

    const char* DATABASE_HEADERS[]         = {"ETag"};
    constexpr size_t DATABASE_HEADER_COUNT = sizeof(DATABASE_HEADERS) / sizeof(DATABASE_HEADERS[0]);

    enum class OperationTarget         : uint8_t {FIRMWARE, SOTA, POTA};
    enum class DatabaseDownloadFailure : uint8_t {NONE, STREAM_DISCONNECTED, STREAM_TIMEOUT, SD_WRITE};

    struct SotaVersionContext {
        char* version;
        size_t size;
        char line[128];
        size_t length;
        bool parsed;
        bool valid;
    };

    struct DatabaseSourceInfo {
        uint64_t size = 0U;
        char etag[database::ota::ETAG_SIZE] {};
    };

    update::Status _firmwareStatus = update::Status::IDLE;
    update::Status _sotaStatus     = update::Status::NOT_INSTALLED;
    update::Status _potaStatus     = update::Status::NOT_INSTALLED;

    uint8_t _firmwareProgress      = 0;
    uint32_t _firmwareExpectedSize = 0;
    uint8_t _sotaProgress          = 0;
    uint32_t _sotaRecords          = 0;
    uint8_t _potaProgress          = 0;
    uint32_t _potaRecords          = 0;
    bool _taskRunning              = false;

    char _firmwareLatestVersion[update::FIRMWARE_VERSION_SIZE] {};
    char _firmwareExpectedSha256[hash::SHA256_TEXT_SIZE]       {};
    char _firmwareError[update::ERROR_SIZE]                    {};
    char _sotaInstalledVersion[update::SOTA_VERSION_SIZE]      {};
    char _sotaLatestVersion[update::SOTA_VERSION_SIZE]         {};
    char _sotaError[update::ERROR_SIZE]                        {};
    char _potaError[update::ERROR_SIZE]                        {};

    void _setStatus(OperationTarget target, update::Status value);
    void _setProgress(OperationTarget target, uint8_t value);
    void _setError(OperationTarget target, const char* value, const char* logCode);
    void _setFirmwareStatus(update::Status value);
    void _setFirmwareProgress(uint8_t value);
    void _setFirmwareError(const char* value, const char* logCode);
    void _configureHttp(HTTPClient& http, WiFiClientSecure& client);
    bool _validateHttpResponse(int httpCode, const char* logFormat);
    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* url, DatabaseSourceInfo* sourceInfo = nullptr);
    bool _openHead(HTTPClient& http, WiFiClientSecure& client, const char* url, DatabaseSourceInfo& sourceInfo);
    bool _isDatabaseTarget(OperationTarget target);
    bool _isOperationTarget(OperationTarget target);
    bool _readRemoteDatabaseSource(OperationTarget target, const char* url, DatabaseSourceInfo& sourceInfo);
    bool _prepareDatabaseCheck(OperationTarget target, const char* path, const char* url, DatabaseSourceInfo& sourceInfo);
    void _publishDatabaseAvailability(OperationTarget target, bool installedAvailable, uint32_t records, const char* installedVersion, bool updateAvailable);
    void _beginDatabaseDownload(OperationTarget target);
    void _beginDatabaseInstallation(OperationTarget target, const char* version);
    void _beginDatabaseVerification(OperationTarget target);
    void _finishDatabaseInstallation(OperationTarget target, uint32_t records, const char* installedVersion);
    void _failDatabaseInstallation(OperationTarget target, const char* path, const char* error, const char* errorLogCode, bool invalidateCache);
    bool _readDatabaseSourceInfo(HTTPClient& http, DatabaseSourceInfo& sourceInfo);
    bool _matchesDatabaseSource(const char* localEtag, uint64_t localSize, const DatabaseSourceInfo& remote);
    bool _isDatabaseUpdateAvailable(bool installedAvailable, const char* installedEtag, uint64_t installedSize, const DatabaseSourceInfo& remote);
    bool _deleteTemporaryFile(const char* path, const char* failureLogCode);
    bool _prepareDatabaseTemporaryFile(OperationTarget target, const char* path);
    const char* _databaseLogCode(OperationTarget target, const char* sotaCode, const char* potaCode);
    bool _invalidateDatabaseCache(OperationTarget target);
    bool _validateWifiPrerequisite(OperationTarget target);
    bool _validateDatabaseUpdatePrerequisites(OperationTarget target);
    void _logDatabaseDownloadFailure(OperationTarget target, DatabaseDownloadFailure failure);
    bool _validateDatabaseDownload(OperationTarget target, const char* path, uint64_t received, const DatabaseSourceInfo& sourceInfo);
    void _updateDatabaseDownloadProgress(OperationTarget target, uint64_t received, int announcedSize);
    bool _readResponseBody(HTTPClient& http, char* buffer, size_t size, void (*progressCallback)(uint8_t) = nullptr);
    bool _acquireTaskSlot();
    bool _isUpdateOperationBusy();
    void _releaseTaskSlot();
    bool _isUpdateAvailable(OperationTarget target);
    void _finishTask();
    void _checkFirmwareTask(void*);
    void _checkSotaTask(void*);
    void _checkPotaTask(void*);
    void _installFirmwareTask(void*);
    void _databaseProgress(OperationTarget target, uint8_t progress);
    void _installSotaTask(void*);
    void _installPotaTask(void*);
    void _sotaDatabaseProgress(uint8_t progress, void* userData);
    void _potaDatabaseProgress(uint8_t progress, void* userData);
    bool _startTask(TaskFunction_t function, const char* name, uint32_t stackSize, OperationTarget target, update::Status initialStatus);
    bool _checkDatabaseUpdate(OperationTarget target, TaskFunction_t function, const char* taskName);
    bool _startDatabaseUpdate(OperationTarget target, TaskFunction_t function, const char* taskName);
    bool _readSotaVersionChunk(const uint8_t* data, size_t length, void* userData);
    bool _readSotaVersion(char* version, size_t size);
    bool _downloadDatabaseCsv(OperationTarget target, const char* url, const char* path, DatabaseSourceInfo& sourceInfo);
    bool _downloadSotaCsv(char* version, size_t versionSize, DatabaseSourceInfo& sourceInfo);

    void _setStatus(const OperationTarget target, const update::Status value) {
        if (!_isOperationTarget(target)) {
            storage::appendErrorRecord("UPDATE_TARGET_INVALID");
            return;
        }

        portENTER_CRITICAL(&_lock);
        switch (target) {
            case OperationTarget::FIRMWARE:
                _firmwareStatus = value;
                break;
            case OperationTarget::SOTA:
                _sotaStatus     = value;
                break;
            case OperationTarget::POTA:
                _potaStatus     = value;
                break;
        }
        portEXIT_CRITICAL(&_lock);
    }

    void _setProgress(const OperationTarget target, uint8_t value) {
        if (!_isOperationTarget(target)) {
            storage::appendErrorRecord("UPDATE_TARGET_INVALID");
            return;
        }
        if (value > 100U) { value = 100U; }

        portENTER_CRITICAL(&_lock);
        switch (target) {
            case OperationTarget::FIRMWARE:
                _firmwareProgress = value;
                break;
            case OperationTarget::SOTA:
                _sotaProgress     = value;
                break;
            case OperationTarget::POTA:
                _potaProgress     = value;
                break;
        }
        portEXIT_CRITICAL(&_lock);
    }

    void _setError(const OperationTarget target, const char* const value, const char* const logCode) {
        if (!_isOperationTarget(target) || value == nullptr || value[0] == '\0') {
            storage::appendErrorRecord("UPDATE_ERROR_ARGUMENT_INVALID");
            return;
        }
        portENTER_CRITICAL(&_lock);

        char* error            = nullptr;
        update::Status* status = nullptr;

        switch (target) {
            case OperationTarget::FIRMWARE:
                error  = _firmwareError;
                status = &_firmwareStatus;
                break;
            case OperationTarget::SOTA:
                error  = _sotaError;
                status = &_sotaStatus;
                break;
            case OperationTarget::POTA:
                error  = _potaError;
                status = &_potaStatus;
                break;
        }

        if (error != nullptr && status != nullptr) {
            text::copy(error, update::ERROR_SIZE, value);
            *status = update::Status::ERROR;
        }

        portEXIT_CRITICAL(&_lock);
        if (logCode != nullptr) { storage::appendErrorRecord(logCode); }
    }

    void _setFirmwareStatus(update::Status value) {
        _setStatus(OperationTarget::FIRMWARE, value);
    }

    void _setFirmwareProgress(const uint8_t value) {
        _setProgress(OperationTarget::FIRMWARE, value);
    }

    void _setFirmwareError(const char* value, const char* logCode) {
        _setError(OperationTarget::FIRMWARE, value, logCode);
    }

    void _configureHttp(HTTPClient& http, WiFiClientSecure& client) {
        client.setCACert(otaRootCA::OTA_ROOT_CA);
        client.setTimeout(HTTP_TIMEOUT_MS);

        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    }

    bool _validateHttpResponse(const int httpCode, const char* const logFormat) {
        if (httpCode == HTTP_CODE_OK) { return true; }

        if (logFormat == nullptr) {
            storage::appendErrorRecord("HTTP_RESPONSE_INVALID");
            return false;
        }

        char logCode[48] {};
        const int written = snprintf(logCode, sizeof(logCode), logFormat, httpCode);

        if (written > 0 && static_cast<size_t>(written) < sizeof(logCode))
            { storage::appendErrorRecord(logCode); }
        else { storage::appendErrorRecord("HTTP_RESPONSE_INVALID"); }
        return false;
    }

    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* const url, DatabaseSourceInfo* const sourceInfo) {
        if (url == nullptr || url[0] == '\0') {
            storage::appendErrorRecord(sourceInfo != nullptr ? "DATABASE_GET_ARGUMENT_INVALID" : "OTA_GET_ARGUMENT_INVALID");
            return false;
        }
        _configureHttp(http, client);

        if (!http.begin(client, url)) {
            storage::appendErrorRecord(sourceInfo != nullptr ? "DATABASE_GET_BEGIN_FAILED" : "OTA_GET_BEGIN_FAILED");
            return false;
        }

        if (sourceInfo != nullptr) { http.collectHeaders(DATABASE_HEADERS, DATABASE_HEADER_COUNT); }

        const int httpCode = http.GET();
        if (!_validateHttpResponse(httpCode, sourceInfo != nullptr ? "DATABASE_GET_HTTP_%d" : "OTA_GET_HTTP_%d"))
            { return false; }
        return sourceInfo == nullptr || _readDatabaseSourceInfo(http, *sourceInfo);
    }

    bool _openHead(HTTPClient& http, WiFiClientSecure& client, const char* const url, DatabaseSourceInfo& sourceInfo) {
        if (url == nullptr || url[0] == '\0') {
            storage::appendErrorRecord("DATABASE_HEAD_ARGUMENT_INVALID");
            return false;
        }
        _configureHttp(http, client);

        if (!http.begin(client, url)) {
            storage::appendErrorRecord("DATABASE_HEAD_BEGIN_FAILED");
            return false;
        }

        http.collectHeaders(DATABASE_HEADERS, DATABASE_HEADER_COUNT);
        const int httpCode = http.sendRequest("HEAD");

        if (!_validateHttpResponse(httpCode, "DATABASE_HEAD_HTTP_%d"))
            { return false; }
        return _readDatabaseSourceInfo(http, sourceInfo);
    }

    bool _isDatabaseTarget(const OperationTarget target) {
        return target == OperationTarget::SOTA || target == OperationTarget::POTA;
    }

    bool _isOperationTarget(const OperationTarget target) {
        return target == OperationTarget::FIRMWARE || _isDatabaseTarget(target);
    }

    bool _readRemoteDatabaseSource(const OperationTarget target, const char* const url, DatabaseSourceInfo& sourceInfo) {
        if (!_isDatabaseTarget(target) || url == nullptr || url[0] == '\0') {
            storage::appendErrorRecord("DATABASE_SOURCE_ARGUMENT_INVALID");
            return false;
        }

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openHead(http, client, url, sourceInfo)) {
            http.end();
            _setError(
                target,
                _databaseLogCode(target, "SOTA check failed", "POTA check failed"),
                _databaseLogCode(target, "SOTA_HEAD_FAILED",  "POTA_HEAD_FAILED")
            );
            return false;
        }

        http.end();
        return true;
    }

    bool _prepareDatabaseCheck(const OperationTarget target, const char* const path, const char* const url, DatabaseSourceInfo& sourceInfo) {
        if (!_prepareDatabaseTemporaryFile(target, path))        { return false; }

        _setProgress(target, 10U);
        if (!_readRemoteDatabaseSource(target, url, sourceInfo)) { return false; }

        _setProgress(target, 75U);
        return true;
    }

    void _publishDatabaseAvailability(const OperationTarget target, const bool installedAvailable, const uint32_t records, const char* const installedVersion, const bool updateAvailable) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }

        if (!installedAvailable && !_invalidateDatabaseCache(target)) { return; }
        portENTER_CRITICAL(&_lock);

        if (target == OperationTarget::SOTA) {
            _sotaRecords = installedAvailable ? records : 0U;
            text::copy(_sotaInstalledVersion, sizeof(_sotaInstalledVersion), installedAvailable && installedVersion != nullptr ? installedVersion : "");

            _sotaLatestVersion[0] = '\0';
            _sotaProgress         = 100U;
            _sotaStatus           = updateAvailable ? update::Status::AVAILABLE : update::Status::UP_TO_DATE;
        } else {
            _potaRecords  = installedAvailable ? records : 0U;
            _potaProgress = 100U;
            _potaStatus   = updateAvailable ? update::Status::AVAILABLE : update::Status::UP_TO_DATE;
        }

        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord(
            _databaseLogCode(
                target,
                updateAvailable ? "SOTA_UPDATE_AVAILABLE" : "SOTA_UP_TO_DATE",
                updateAvailable ? "POTA_UPDATE_AVAILABLE" : "POTA_UP_TO_DATE"
            )
        );
    }

    void _beginDatabaseDownload(const OperationTarget target) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }

        _setStatus(target, update::Status::DOWNLOADING);
        _setProgress(target, 0U);
    }

    void _beginDatabaseInstallation(const OperationTarget target, const char* const version) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }

        portENTER_CRITICAL(&_lock);
        if (target == OperationTarget::SOTA) {
            text::copy(_sotaLatestVersion, sizeof(_sotaLatestVersion), version != nullptr ? version : "");
            _sotaProgress = 60U;
            _sotaStatus   = update::Status::INSTALLING;
        } else {
            _potaProgress = 60U;
            _potaStatus   = update::Status::INSTALLING;
        }
        portEXIT_CRITICAL(&_lock);
    }

    void _beginDatabaseVerification(const OperationTarget target) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }
        _setProgress(target, 95U);
        _setStatus(target, update::Status::VERIFYING);
    }

    void _finishDatabaseInstallation(const OperationTarget target, const uint32_t records, const char* const installedVersion) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }

        portENTER_CRITICAL(&_lock);
        if (target == OperationTarget::SOTA) {
            _sotaRecords = records;
            text::copy(_sotaInstalledVersion, sizeof(_sotaInstalledVersion), installedVersion != nullptr ? installedVersion : "");
            _sotaProgress = 100U;
            _sotaStatus   = update::Status::SUCCESS;
        } else {
            _potaRecords  = records;
            _potaProgress = 100U;
            _potaStatus   = update::Status::SUCCESS;
        }
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord(_databaseLogCode(target, "SOTA_UPDATE_SUCCESS", "POTA_UPDATE_SUCCESS"));
    }

    void _failDatabaseInstallation(const OperationTarget target, const char* const path, const char* const error, const char* const errorLogCode, const bool invalidateCache) {
        if (!_isDatabaseTarget(target) ||
            path            == nullptr || path[0]  == '\0' ||
            error           == nullptr || error[0] == '\0' ||
            errorLogCode    == nullptr ||
            errorLogCode[0] == '\0'
        ) {
            storage::appendErrorRecord("DATABASE_FAILURE_ARGUMENT_INVALID");
            return;
        }

        _deleteTemporaryFile(path, _databaseLogCode(target, "SOTA_TEMP_DELETE_FAILED", "POTA_TEMP_DELETE_FAILED"));

        if (invalidateCache && !_invalidateDatabaseCache(target)) { return; }
        _setError(target, error, errorLogCode);
    }

    bool _readDatabaseSourceInfo(HTTPClient& http, DatabaseSourceInfo& sourceInfo) {
        sourceInfo = DatabaseSourceInfo {};

        const int announcedSize = http.getSize();
        const String etag       = http.header("ETag");

        if (announcedSize <= 0) {
            storage::appendErrorRecord("DATABASE_CONTENT_LENGTH_INVALID");
            return false;
        }

        if (etag.isEmpty()) {
            storage::appendErrorRecord("DATABASE_ETAG_MISSING");
            return false;
        }

        sourceInfo.size = static_cast<uint64_t>(announcedSize);

        if (!text::copy(sourceInfo.etag, sizeof(sourceInfo.etag), etag.c_str())) {
            sourceInfo = DatabaseSourceInfo {};
            storage::appendErrorRecord("DATABASE_ETAG_TOO_LONG");
            return false;
        }

        return true;
    }

    bool _matchesDatabaseSource(const char* const localEtag, const uint64_t localSize, const DatabaseSourceInfo& remote) {
        return text::equals(localEtag, remote.etag) && localSize == remote.size;
    }

    bool _isDatabaseUpdateAvailable(const bool installedAvailable, const char* const installedEtag, const uint64_t installedSize, const DatabaseSourceInfo& remote) {
        return !installedAvailable || !_matchesDatabaseSource(installedEtag, installedSize, remote);
    }

    bool _deleteTemporaryFile(const char* const path, const char* const failureLogCode) {
        if (path == nullptr || path[0] == '\0') {
            storage::appendErrorRecord("DATABASE_TEMP_ARGUMENT_INVALID");
            return false;
        }

        if (storage::deleteFile(path)) { return true; }
        if (failureLogCode != nullptr) { storage::appendErrorRecord(failureLogCode); }
        return false;
    }

    bool _prepareDatabaseTemporaryFile(const OperationTarget target, const char* const path) {
        if (!_isDatabaseTarget(target) || path == nullptr || path[0] == '\0') {
            storage::appendErrorRecord("DATABASE_TEMP_ARGUMENT_INVALID");
            return false;
        }
        if (_deleteTemporaryFile(path, nullptr)) { return true; }

        _setError(
            target,
            _databaseLogCode(target, "SOTA temporary file unavailable", "POTA temporary file unavailable"),
            _databaseLogCode(target, "SOTA_TEMP_DELETE_FAILED",         "POTA_TEMP_DELETE_FAILED")
        );
        return false;
    }

    const char* _databaseLogCode(const OperationTarget target, const char* const sotaCode, const char* const potaCode) {
        switch (target) {
            case OperationTarget::SOTA:     return sotaCode;
            case OperationTarget::POTA:     return potaCode;
            case OperationTarget::FIRMWARE:
            default:                        return nullptr;
        }
    }

    bool _invalidateDatabaseCache(const OperationTarget target) {
        switch (target) {
            case OperationTarget::SOTA:
                ota::invalidate(ota::Type::SUMMITS);
                return true;
            case OperationTarget::POTA:
                ota::invalidate(ota::Type::PARKS);
                return true;
            case OperationTarget::FIRMWARE:
            default:
                storage::appendErrorRecord("DATABASE_TARGET_INVALID");
                return false;
        }
    }

    bool _validateWifiPrerequisite(const OperationTarget target) {
        if (!_isOperationTarget(target)) {
            storage::appendErrorRecord("UPDATE_TARGET_INVALID");
            return false;
        }
        if (wifi::isConnected()) { return true; }

        const char* logCode = nullptr;
        switch (target) {
            case OperationTarget::FIRMWARE:
                logCode = "OTA_WIFI_NOT_CONNECTED";
                break;
            case OperationTarget::SOTA:
            case OperationTarget::POTA:
                logCode = _databaseLogCode(target, "SOTA_WIFI_NOT_CONNECTED", "POTA_WIFI_NOT_CONNECTED");
                break;
        }

        if (logCode == nullptr) {
            storage::appendErrorRecord("UPDATE_TARGET_INVALID");
            return false;
        }

        _setError(target, "WiFi not connected", logCode);
        return false;
    }

    bool _validateDatabaseUpdatePrerequisites(const OperationTarget target) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return false;
        }

        if (!storage::isReady()) {
            _setError(target, "SD card unavailable", _databaseLogCode(target, "SOTA_SD_UNAVAILABLE", "POTA_SD_UNAVAILABLE"));
            return false;
        }

        return _validateWifiPrerequisite(target);
    }

    void _logDatabaseDownloadFailure(const OperationTarget target, const DatabaseDownloadFailure failure) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }
        const char* logCode = nullptr;

        switch (failure) {
            case DatabaseDownloadFailure::STREAM_DISCONNECTED:
                logCode = _databaseLogCode(target, "SOTA_STREAM_DISCONNECTED", "POTA_STREAM_DISCONNECTED");
                break;
            case DatabaseDownloadFailure::STREAM_TIMEOUT:
                logCode = _databaseLogCode(target, "SOTA_STREAM_TIMEOUT", "POTA_STREAM_TIMEOUT");
                break;
            case DatabaseDownloadFailure::SD_WRITE:
                logCode = _databaseLogCode(target, "SOTA_SD_WRITE_FAILED", "POTA_SD_WRITE_FAILED");
                break;
            case DatabaseDownloadFailure::NONE:
            default:
                return;
        }
        if (logCode != nullptr) { storage::appendErrorRecord(logCode); }
    }

    bool _validateDatabaseDownload(const OperationTarget target, const char* const path, const uint64_t received, const DatabaseSourceInfo& sourceInfo) {
        if (!_isDatabaseTarget(target) || path == nullptr || path[0] == '\0') {
            storage::appendErrorRecord("DATABASE_DOWNLOAD_ARGUMENT_INVALID");
            return false;
        }

        const char* failureLogCode = nullptr;

        if (received == 0U)
            { failureLogCode = _databaseLogCode(target, "SOTA_EMPTY_RESPONSE",     "POTA_EMPTY_RESPONSE"); }
        else if (received != sourceInfo.size)
            { failureLogCode = _databaseLogCode(target, "SOTA_SIZE_MISMATCH",      "POTA_SIZE_MISMATCH"); }
        else if (storage::fileSize(path) != received)
            { failureLogCode = _databaseLogCode(target, "SOTA_FILE_SIZE_MISMATCH", "POTA_FILE_SIZE_MISMATCH"); }
        else { return true; }

        storage::appendErrorRecord(failureLogCode);
        _deleteTemporaryFile(path, _databaseLogCode(target, "SOTA_TEMP_DELETE_FAILED", "POTA_TEMP_DELETE_FAILED"));
        return false;
    }

    void _updateDatabaseDownloadProgress(const OperationTarget target, const uint64_t received, const int announcedSize) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }
        if (announcedSize <= 0) { return; }

        const uint8_t progress = static_cast<uint8_t>((received * 60ULL) / static_cast<uint64_t>(announcedSize));
        _setProgress(target, progress);
    }

    bool _readResponseBody(HTTPClient& http, char* const buffer, const size_t size, void (*progressCallback)(uint8_t)) {
        if (buffer == nullptr || size < 2U) { return false; }

        buffer[0]               = '\0';
        const int announcedSize = http.getSize();
        if (announcedSize >= 0 && static_cast<size_t>(announcedSize) >= size) { return false; }

        WiFiClient* const stream = http.getStreamPtr();
        if (stream == nullptr) { return false; }

        size_t received     = 0U;
        uint32_t lastDataAt = millis();

        if (progressCallback != nullptr && announcedSize > 0)
            { progressCallback(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / static_cast<size_t>(announcedSize))); }

        while (true) {
            const int available = stream->available();
            if (available > 0) {
                size_t toRead          = static_cast<size_t>(available);
                const size_t remaining = size - 1U - received;

                if (remaining == 0U) {
                    buffer[0] = '\0';
                    return false;
                }

                if (toRead > remaining) { toRead = remaining; }
                const int read = stream->readBytes(reinterpret_cast<uint8_t*>(buffer + received), toRead);

                if (read <= 0) {
                    if (!http.connected() || millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                        buffer[0] = '\0';
                        return false;
                    }

                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                }

                received += static_cast<size_t>(read);
                lastDataAt = millis();

                if (progressCallback != nullptr && announcedSize > 0)
                    { progressCallback(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / static_cast<size_t>(announcedSize))); }
                if (announcedSize >= 0 && received == static_cast<size_t>(announcedSize))
                    { break; }
                continue;
            }

            if (announcedSize >= 0 && received == static_cast<size_t>(announcedSize))
                { break; }
            if (!http.connected())
                { break; }
            if (millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                buffer[0] = '\0';
                return false;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (received == 0U || (announcedSize >= 0 && received != static_cast<size_t>(announcedSize))) {
            buffer[0] = '\0';
            return false;
        }

        buffer[received] = '\0';
        if (progressCallback != nullptr)
            { progressCallback(100U); }
        return true;
    }

    bool _acquireTaskSlot() {
        if (ota::isBusy()) { return false; }

        portENTER_CRITICAL(&_lock);
        if (_taskRunning) {
            portEXIT_CRITICAL(&_lock);
            return false;
        }

        _taskRunning = true;
        portEXIT_CRITICAL(&_lock);

        if (!ota::isBusy())
            { return true; }
        _releaseTaskSlot();
        return false;
    }

    bool _isUpdateOperationBusy() {
        return update::isBusy() || ota::isBusy();
    }

    void _releaseTaskSlot() {
        portENTER_CRITICAL(&_lock);
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);
    }

    bool _isUpdateAvailable(const OperationTarget target) {
        if (!_isOperationTarget(target)) {
            storage::appendErrorRecord("UPDATE_TARGET_INVALID");
            return false;
        }

        portENTER_CRITICAL(&_lock);
        bool available = false;
        switch (target) {
            case OperationTarget::FIRMWARE:
                available = !_taskRunning && _firmwareStatus == update::Status::AVAILABLE;
                break;
            case OperationTarget::SOTA:
                available = !_taskRunning && _sotaStatus == update::Status::AVAILABLE;
                break;
            case OperationTarget::POTA:
                available = !_taskRunning && _potaStatus == update::Status::AVAILABLE;
                break;
        }
        portEXIT_CRITICAL(&_lock);
        return available;
    }

    void _finishTask() {
        _releaseTaskSlot();
        vTaskDelete(nullptr);
    }

    class TaskFinalizer final {
        public:
            ~TaskFinalizer() { _finishTask(); }
    };

    void _checkFirmwareTask(void*) {
        TaskFinalizer finalizer;
        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_FIRMWARE_MANIFEST)) {
            http.end();
            _setFirmwareError("Manifest unavailable", "OTA_MANIFEST_HTTP_FAILED");
            return;
        }

        char manifest[MANIFEST_BUFFER_SIZE];
        const bool manifestRead =_readResponseBody(http, manifest, sizeof(manifest), _setFirmwareProgress);
        http.end();

        if (!manifestRead) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_INVALID");
            return;
        }

        json::Reader reader(manifest);
        char remoteVersion[update::FIRMWARE_VERSION_SIZE];
        char sha256[hash::SHA256_TEXT_SIZE];
        uint64_t manifestSize = 0U;

        if (!reader.valid()                                                 ||
            !reader.string("version", remoteVersion, sizeof(remoteVersion)) ||
            !reader.string("sha256",  sha256,        sizeof(sha256))        ||
            !reader.unsignedInteger("size", manifestSize)                   ||
            manifestSize == 0U                                              ||
            manifestSize > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())
        ) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            return;
        }

        const uint32_t size = static_cast<uint32_t>(manifestSize);
        version::Comparison comparison;
    
        if (!version::compare(remoteVersion, PROJECT_VERSION, comparison)) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            return;
        }

        char normalizedSha[hash::SHA256_TEXT_SIZE];
        if (!hash::normalizeSha256Text(sha256, normalizedSha, sizeof(normalizedSha))) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            return;
        }

        const bool updateAvailable = comparison == version::Comparison::NEWER;

        portENTER_CRITICAL(&_lock);
        text::copy(_firmwareLatestVersion,  sizeof(_firmwareLatestVersion),  remoteVersion);
        text::copy(_firmwareExpectedSha256, sizeof(_firmwareExpectedSha256), normalizedSha);
        _firmwareExpectedSize = size;
        _firmwareProgress     = 0;
        _firmwareError[0]     = '\0';
        _firmwareStatus       = updateAvailable ? update::Status::AVAILABLE : update::Status::UP_TO_DATE;
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord(updateAvailable ? "OTA_UPDATE_AVAILABLE" : "OTA_UP_TO_DATE");
    }

    void _checkSotaTask(void*) {
        TaskFinalizer finalizer;

        DatabaseSourceInfo remote;
        if (!_prepareDatabaseCheck(OperationTarget::SOTA, SOTA_CSV_PATH, DL_SOTA, remote))
            { return; }

        summitsDB::Info installed;
        const bool installedAvailable = summitsDB::info(installed);
        const bool updateAvailable    = _isDatabaseUpdateAvailable(installedAvailable, installed.etag, installed.sourceSize, remote);

        _publishDatabaseAvailability(OperationTarget::SOTA, installedAvailable, installed.records, installed.version, updateAvailable);
    }

    void _checkPotaTask(void*) {
        TaskFinalizer finalizer;

        DatabaseSourceInfo remote;
        if (!_prepareDatabaseCheck(OperationTarget::POTA, POTA_CSV_PATH, DL_POTA, remote))
            { return; }

        parksDB::Info installed;
        const bool installedAvailable = parksDB::info(installed);
        const bool updateAvailable    = _isDatabaseUpdateAvailable(installedAvailable, installed.etag, installed.sourceSize, remote);

        _publishDatabaseAvailability(OperationTarget::POTA, installedAvailable, installed.records, nullptr, updateAvailable);
    }

    void _installFirmwareTask(void*) {
        TaskFinalizer finalizer;

        uint32_t expectedSize = 0;
        char expectedSha[hash::SHA256_TEXT_SIZE];

        portENTER_CRITICAL(&_lock);
        expectedSize = _firmwareExpectedSize;
        text::copy(expectedSha, sizeof(expectedSha), _firmwareExpectedSha256);
        portEXIT_CRITICAL(&_lock);

        if (expectedSize == 0U || !hash::isSha256Text(expectedSha)) {
            _setFirmwareError("Invalid firmware metadata", "OTA_METADATA_INVALID");
            return;
        }

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_FIRMWARE)) {
            http.end();
            _setFirmwareError("Firmware unavailable", "OTA_FIRMWARE_HTTP_FAILED");
            return;
        }

        const int announcedSize = http.getSize();
        if (announcedSize > 0 && static_cast<uint32_t>(announcedSize) != expectedSize) {
            http.end();
            _setFirmwareError("Wrong file size", "OTA_SIZE_HEADER_MISMATCH");
            return;
        }

        if (!::Update.begin(expectedSize, U_FLASH)) {
            http.end();
            _setFirmwareError("OTA space unavailable", "OTA_BEGIN_FAILED");
            return;
        }

        hash::Sha256 sha256;
        if (!sha256.begin()) {
            Update.abort();
            http.end();
            _setFirmwareError("SHA init failed", "OTA_SHA_INIT_FAILED");
            return;
        }

        WiFiClient* const stream = http.getStreamPtr();
        if (stream == nullptr) {
            ::Update.abort();
            http.end();
            _setFirmwareError("Invalid firmware stream", "OTA_STREAM_UNAVAILABLE");
            return;
        }

        uint8_t buffer[DOWNLOAD_BUFFER_SIZE];
        uint32_t received           = 0;
        uint32_t lastDataAt         = millis();
        bool transferOk             = true;
        const char* transferError   = "Download interrupted";
        const char* transferLogCode = "OTA_DOWNLOAD_INCOMPLETE";

        while (received < expectedSize) {
            const int available = stream->available();

            if (available <= 0) {
                const bool connected = http.connected();
                if (!connected || millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    transferOk      = false;
                    transferLogCode = connected ? "OTA_STREAM_TIMEOUT" : "OTA_STREAM_DISCONNECTED";
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            size_t toRead = static_cast<size_t>(available);
            if (toRead > sizeof(buffer)) { toRead = sizeof(buffer); }

            const uint32_t remaining = expectedSize - received;
            if (toRead > remaining)      { toRead = remaining; }

            const int read = stream->readBytes(buffer, toRead);
            if (read <= 0) {
                const bool connected = http.connected();
                if (!connected || millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    transferOk      = false;
                    transferLogCode = connected ? "OTA_STREAM_TIMEOUT" : "OTA_STREAM_DISCONNECTED";
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            lastDataAt = millis();
            if (!sha256.update(buffer, static_cast<size_t>(read))) {
                transferOk      = false;
                transferError   = "SHA update failed";
                transferLogCode = "OTA_SHA_UPDATE_FAILED";
                break;
            }

            if (::Update.write(buffer, static_cast<size_t>(read)) != static_cast<size_t>(read)) {
                transferOk      = false;
                transferError   = "Flash write failed";
                transferLogCode = "OTA_WRITE_FAILED";
                break;
            }

            received += static_cast<uint32_t>(read);
            _setFirmwareProgress(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / expectedSize));
        }

        http.end();
        if (!transferOk || received != expectedSize) {
            ::Update.abort();
            _setFirmwareError(transferError, transferLogCode);
            return;
        }

        _setFirmwareStatus(update::Status::VERIFYING);
        char calculatedSha[hash::SHA256_TEXT_SIZE];
        if (!sha256.finish(calculatedSha, sizeof(calculatedSha))) {
            ::Update.abort();
            _setFirmwareError("SHA check failed", "OTA_SHA_FINISH_FAILED");
            return;
        }

        if (!text::equals(calculatedSha, expectedSha)) {
            ::Update.abort();
            _setFirmwareError("SHA-256 mismatch", "OTA_SHA_MISMATCH");
            return;
        }

        _setFirmwareStatus(update::Status::INSTALLING);
        if (!::Update.end(false)) {
            _setFirmwareError("Firmware rejected", "OTA_END_FAILED");
            return;
        }

        _setFirmwareProgress(100);
        _setFirmwareStatus(update::Status::SUCCESS);

        storage::appendLogRecord("OTA_UPDATE_SUCCESS");
        vTaskDelay(pdMS_TO_TICKS(1500));
        ESP.restart();
    }

    void _databaseProgress(const OperationTarget target, const uint8_t progress) {
        if (!_isDatabaseTarget(target)) {
            storage::appendErrorRecord("DATABASE_TARGET_INVALID");
            return;
        }
        const uint8_t boundedProgress = progress > 100U ? 100U : progress;
        const uint8_t mapped          = static_cast<uint8_t>(60U + (static_cast<uint16_t>(boundedProgress) * 35U) / 100U);
        _setProgress(target, mapped);
    }

    void _sotaDatabaseProgress(const uint8_t progress, void*) {
        _databaseProgress(OperationTarget::SOTA, progress);
    }

    void _potaDatabaseProgress(const uint8_t progress, void*) {
        _databaseProgress(OperationTarget::POTA, progress);
    }

    void _installSotaTask(void*) {
        TaskFinalizer finalizer;
        DatabaseSourceInfo remote;
        if (!_readRemoteDatabaseSource(OperationTarget::SOTA, DL_SOTA, remote)) { return; }

        summitsDB::Info current;
        if (summitsDB::info(current) && _matchesDatabaseSource(current.etag, current.sourceSize, remote)) {
            _publishDatabaseAvailability(OperationTarget::SOTA, true, current.records, current.version, false);
            return;
        }

        if (!_prepareDatabaseTemporaryFile(OperationTarget::SOTA, SOTA_CSV_PATH))
            { return; }

        char version[update::SOTA_VERSION_SIZE] {};
        DatabaseSourceInfo downloaded;

        _beginDatabaseDownload(OperationTarget::SOTA);
        if (!_downloadSotaCsv(version, sizeof(version), downloaded)) {
            _setError(OperationTarget::SOTA, "SOTA download failed", nullptr);
            return;
        }

        _beginDatabaseInstallation(OperationTarget::SOTA, version);
        if (!summitsDB::buildCandidate(SOTA_CSV_PATH, version, downloaded.etag, downloaded.size, _sotaDatabaseProgress)) {
            _failDatabaseInstallation(OperationTarget::SOTA, SOTA_CSV_PATH, "Invalid SOTA database", "SOTA_BUILD_FAILED", false);
            return;
        }

        _beginDatabaseVerification(OperationTarget::SOTA);
        if (!summitsDB::installCandidate()) {
            summitsDB::discardCandidate();
            _failDatabaseInstallation(OperationTarget::SOTA, SOTA_CSV_PATH, "SOTA install failed", "SOTA_INSTALL_FAILED", false);
            return;
        }

        if (!_invalidateDatabaseCache(OperationTarget::SOTA))
            { return; }

        summitsDB::Info installedInfo;
        if (!summitsDB::info(installedInfo) || !_matchesDatabaseSource(installedInfo.etag, installedInfo.sourceSize, downloaded)) {
            _failDatabaseInstallation(OperationTarget::SOTA, SOTA_CSV_PATH, "SOTA validation failed", "SOTA_VALIDATION_FAILED", true);
            return;
        }

        _deleteTemporaryFile(SOTA_CSV_PATH, "SOTA_TEMP_DELETE_FAILED");
        _finishDatabaseInstallation(OperationTarget::SOTA, installedInfo.records, installedInfo.version);
    }

    void _installPotaTask(void*) {
        TaskFinalizer finalizer;
        DatabaseSourceInfo remote;
        if (!_readRemoteDatabaseSource(OperationTarget::POTA, DL_POTA, remote))
            { return; }

        parksDB::Info current;
        if (parksDB::info(current) && _matchesDatabaseSource(current.etag, current.sourceSize, remote)) {
            _publishDatabaseAvailability(OperationTarget::POTA, true, current.records, nullptr, false);
            return;
        }

        if (!_prepareDatabaseTemporaryFile(OperationTarget::POTA, POTA_CSV_PATH))
            { return; }

        DatabaseSourceInfo downloaded;
        _beginDatabaseDownload(OperationTarget::POTA);
        if (!_downloadDatabaseCsv(OperationTarget::POTA, DL_POTA, POTA_CSV_PATH, downloaded)) {
            _setError(OperationTarget::POTA, "POTA download failed", nullptr);
            return;
        }

        _beginDatabaseInstallation(OperationTarget::POTA, nullptr);
        if (!parksDB::buildCandidate(POTA_CSV_PATH, downloaded.etag, downloaded.size, _potaDatabaseProgress)) {
            _failDatabaseInstallation(OperationTarget::POTA, POTA_CSV_PATH, "Invalid POTA database", "POTA_BUILD_FAILED", false);
            return;
        }

        _beginDatabaseVerification(OperationTarget::POTA);
        if (!parksDB::installCandidate()) {
            parksDB::discardCandidate();
            _failDatabaseInstallation(OperationTarget::POTA, POTA_CSV_PATH, "POTA install failed", "POTA_INSTALL_FAILED", false);
            return;
        }

        if (!_invalidateDatabaseCache(OperationTarget::POTA))
            { return; }

        parksDB::Info installedInfo;
        if (!parksDB::info(installedInfo) || !_matchesDatabaseSource(installedInfo.etag, installedInfo.sourceSize, downloaded)) {
            _failDatabaseInstallation(OperationTarget::POTA, POTA_CSV_PATH, "POTA validation failed", "POTA_VALIDATION_FAILED", true);
            return;
        }

        _deleteTemporaryFile(POTA_CSV_PATH, "POTA_TEMP_DELETE_FAILED");
        _finishDatabaseInstallation(OperationTarget::POTA, installedInfo.records, nullptr);
    }

    bool _startTask(const TaskFunction_t function, const char* const name, const uint32_t stackSize, const OperationTarget target, const update::Status initialStatus) {
        if (function  == nullptr        ||
            name      == nullptr        ||
            name[0]   == '\0'           ||
            stackSize == 0U             ||
            !_isOperationTarget(target) ||
            (initialStatus != update::Status::CHECKING && initialStatus != update::Status::DOWNLOADING)
        ) {
            storage::appendErrorRecord("UPDATE_TASK_ARGUMENT_INVALID");
            return false;
        }
        if (!_acquireTaskSlot()) { return false; }

        portENTER_CRITICAL(&_lock);
        if (target == OperationTarget::FIRMWARE) {
            _firmwareProgress = 0U;
            _firmwareError[0] = '\0';
            _firmwareStatus   = initialStatus;

            if (initialStatus == update::Status::CHECKING) {
                _firmwareLatestVersion[0]  = '\0';
                _firmwareExpectedSha256[0] = '\0';
                _firmwareExpectedSize      = 0U;
            }
        } else if (target == OperationTarget::SOTA) {
            _sotaProgress         = 0U;
            _sotaLatestVersion[0] = '\0';
            _sotaError[0]         = '\0';
            _sotaStatus           = initialStatus;
        } else {
            _potaProgress = 0U;
            _potaError[0] = '\0';
            _potaStatus   = initialStatus;
        }
        portEXIT_CRITICAL(&_lock);

        const BaseType_t result = xTaskCreate(function, name, stackSize, nullptr, 1, nullptr);
        if (result == pdPASS) { return true; }

        _releaseTaskSlot();

        const char* const failureLogCode =
            target == OperationTarget::FIRMWARE
                ? "OTA_TASK_CREATE_FAILED"
                : _databaseLogCode(target, "SOTA_TASK_CREATE_FAILED", "POTA_TASK_CREATE_FAILED");
        _setError(target, "Task creation failed", failureLogCode);
        return false;
    }

    bool _checkDatabaseUpdate(const OperationTarget target, const TaskFunction_t function, const char* const taskName) {
        if (_isUpdateOperationBusy())                      { return false; }
        if (!_validateDatabaseUpdatePrerequisites(target)) { return false; }
        return _startTask(function, taskName, 12288U, target, update::Status::CHECKING);
    }

    bool _startDatabaseUpdate(const OperationTarget target, const TaskFunction_t function, const char* const taskName) {
        if (_isUpdateOperationBusy())                      { return false; }
        if (!_isUpdateAvailable(target))                   { return false; }
        if (!_validateDatabaseUpdatePrerequisites(target)) { return false; }
        return _startTask(function, taskName, 12288U, target, update::Status::DOWNLOADING);
    }

    bool _readSotaVersionChunk(const uint8_t* const data, const size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        SotaVersionContext* const context = static_cast<SotaVersionContext*>(userData);
        if (!context->valid || context->parsed)     { return false; }

        for (size_t byteIndex = 0U; byteIndex < length; ++byteIndex) {
            const char character = static_cast<char>(data[byteIndex]);

            if (character == '\r') { continue; }
            if (character == '\n') {
                context->line[context->length] = '\0';
                context->parsed                = summits::parseListVersion(context->line, context->version, context->size);
                context->valid                 = context->parsed;
                return false;
            }
            if (context->length + 1U >= sizeof(context->line)) {
                context->valid = false;
                return false;
            }
            context->line[context->length++] = character;
        }
        return true;
    }

    bool _readSotaVersion(char* const version, const size_t size) {
        if (version == nullptr || size < summitsDB::VERSION_SIZE) { return false; }
        version[0] = '\0';
        SotaVersionContext context {version, size, {}, 0U, false, true};
        return storage::readFileChunks(SOTA_CSV_PATH, _readSotaVersionChunk, &context) && context.valid && context.parsed;
    }

    bool _downloadDatabaseCsv(const OperationTarget target, const char* const url, const char* const path, DatabaseSourceInfo& sourceInfo) {
        if (!_isDatabaseTarget(target) ||
            url  == nullptr            || url[0]  == '\0' ||
            path == nullptr            || path[0] == '\0'
        ) {
            storage::appendErrorRecord("DATABASE_DOWNLOAD_ARGUMENT_INVALID");
            return false;
        }

        sourceInfo = DatabaseSourceInfo {};

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, url, &sourceInfo)) {
            http.end();
            storage::appendErrorRecord(_databaseLogCode(target, "SOTA_HTTP_FAILED", "POTA_HTTP_FAILED"));
            return false;
        }

        WiFiClient* const stream = http.getStreamPtr();
        const int announcedSize  = http.getSize();

        if (stream == nullptr) {
            http.end();
            storage::appendErrorRecord(_databaseLogCode(target, "SOTA_STREAM_UNAVAILABLE", "POTA_STREAM_UNAVAILABLE"));
            return false;
        }

        if (!storage::beginFileWrite(path)) {
            http.end();
            storage::appendErrorRecord(_databaseLogCode(target, "SOTA_SD_OPEN_FAILED", "POTA_SD_OPEN_FAILED"));
            return false;
        }

        uint8_t buffer[DOWNLOAD_BUFFER_SIZE];
        size_t received                 = 0U;
        uint32_t lastDataAt             = millis();
        DatabaseDownloadFailure failure = DatabaseDownloadFailure::NONE;

        while (failure == DatabaseDownloadFailure::NONE && received < static_cast<size_t>(announcedSize)) {
            const int available = stream->available();

            if (available <= 0) {
                if (!http.connected()) {
                    failure = DatabaseDownloadFailure::STREAM_DISCONNECTED;
                    break;
                }

                if (millis() - lastDataAt > STREAM_TIMEOUT_MS) { failure = DatabaseDownloadFailure::STREAM_TIMEOUT; }
                else                                           { vTaskDelay(pdMS_TO_TICKS(10)); }
                continue;
            }

            size_t length = static_cast<size_t>(available);
            if (length > sizeof(buffer)) { length = sizeof(buffer); }

            const size_t remaining = static_cast<size_t>(announcedSize) - received;
            if (length > remaining)      { length = remaining; }

            const int read = stream->readBytes(buffer, length);
            if (read <= 0) {
                if (!http.connected())                              { failure = DatabaseDownloadFailure::STREAM_DISCONNECTED; }
                else if (millis() - lastDataAt > STREAM_TIMEOUT_MS) { failure = DatabaseDownloadFailure::STREAM_TIMEOUT; }
                else                                                { vTaskDelay(pdMS_TO_TICKS(10)); }
                continue;
            }

            const size_t readLength = static_cast<size_t>(read);
            if (!storage::writeFileChunk(buffer, readLength)) {
                failure = DatabaseDownloadFailure::SD_WRITE;
                break;
            }

            received  += readLength;
            lastDataAt = millis();
            _updateDatabaseDownloadProgress(target, received, announcedSize);
        }

        storage::endFileWrite();
        http.end();

        if (failure != DatabaseDownloadFailure::NONE) {
            _logDatabaseDownloadFailure(target, failure);
            _deleteTemporaryFile(path, _databaseLogCode(target, "SOTA_TEMP_DELETE_FAILED", "POTA_TEMP_DELETE_FAILED"));
            return false;
        }

        if (!_validateDatabaseDownload(target, path, received, sourceInfo))
            { return false; }
        return true;
    }

    bool _downloadSotaCsv(char* const version, const size_t versionSize, DatabaseSourceInfo& sourceInfo) {
        if (version == nullptr || versionSize < summitsDB::VERSION_SIZE) {
            storage::appendErrorRecord("SOTA_DOWNLOAD_ARGUMENT_INVALID");
            return false;
        }

        version[0] = '\0';

        if (!_downloadDatabaseCsv(OperationTarget::SOTA, DL_SOTA, SOTA_CSV_PATH, sourceInfo))
            { return false; }

        if (!_readSotaVersion(version, versionSize)) {
            storage::appendErrorRecord("SOTA_VERSION_INVALID");
            _deleteTemporaryFile(SOTA_CSV_PATH, "SOTA_TEMP_DELETE_FAILED");
            return false;
        }

        return true;
    }
}

void update::begin() {
    if (!_acquireTaskSlot()) {
        storage::appendErrorRecord("UPDATE_INIT_BUSY");
        return;
    }

    summitsDB::Info sotaInfo;
    parksDB::Info potaInfo;

    const bool storageReady  = storage::isReady();
    const bool sotaInstalled = storageReady && summitsDB::info(sotaInfo);
    const bool potaInstalled = storageReady && parksDB::info(potaInfo);

    if (sotaInstalled) {
        char databaseRecord[128];
        const int databaseWritten = snprintf(
            databaseRecord, sizeof(databaseRecord),
            "SOTA_DATABASE status=ready version=%s records=%lu source_size=%llu",
            sotaInfo.version,
            static_cast<unsigned long>(sotaInfo.records),
            static_cast<unsigned long long>(sotaInfo.sourceSize)
        );

        if (databaseWritten > 0 && static_cast<size_t>(databaseWritten) < sizeof(databaseRecord))
            { storage::appendLogRecord(databaseRecord); }
    } else { storage::appendLogRecord("SOTA_DATABASE status=not_installed"); }

    if (potaInstalled) {
        char databaseRecord[128];
        const int databaseWritten = snprintf(
            databaseRecord, sizeof(databaseRecord),
            "POTA_DATABASE status=ready records=%lu source_size=%llu",
            static_cast<unsigned long>(potaInfo.records),
            static_cast<unsigned long long>(potaInfo.sourceSize)
        );

        if (databaseWritten > 0 && static_cast<size_t>(databaseWritten) < sizeof(databaseRecord))
            { storage::appendLogRecord(databaseRecord); }
    } else { storage::appendLogRecord("POTA_DATABASE status=not_installed"); }

    portENTER_CRITICAL(&_lock);
    _firmwareStatus            = Status::IDLE;
    _firmwareProgress          = 0;
    _firmwareLatestVersion[0]  = '\0';
    _firmwareExpectedSha256[0] = '\0';
    _firmwareError[0]          = '\0';
    _firmwareExpectedSize      = 0;
    _sotaStatus                = sotaInstalled ? Status::IDLE : Status::NOT_INSTALLED;
    _sotaProgress              = 0U;
    _sotaRecords               = sotaInstalled ? sotaInfo.records : 0U;
    text::copy(_sotaInstalledVersion, sizeof(_sotaInstalledVersion), sotaInstalled ? sotaInfo.version : "");
    _sotaLatestVersion[0]      = '\0';
    _sotaError[0]              = '\0';
    _potaStatus                = potaInstalled ? Status::IDLE : Status::NOT_INSTALLED;
    _potaProgress              = 0U;
    _potaRecords               = potaInstalled ? potaInfo.records : 0U;
    _potaError[0]              = '\0';
    portEXIT_CRITICAL(&_lock);

    _releaseTaskSlot();
    storage::appendLogRecord("UPDATE_READY");
}

bool update::checkFirmwareUpdate() {
    if (_isUpdateOperationBusy())                              { return false; }
    if (!_validateWifiPrerequisite(OperationTarget::FIRMWARE)) { return false; }
    return _startTask(_checkFirmwareTask, "OTA check", 8192, OperationTarget::FIRMWARE, Status::CHECKING);
}

bool update::checkSotaUpdate() {
    return _checkDatabaseUpdate(OperationTarget::SOTA, _checkSotaTask, "SOTA check");
}

bool update::checkPotaUpdate() {
    return _checkDatabaseUpdate(OperationTarget::POTA, _checkPotaTask, "POTA check");
}

bool update::startFirmwareUpdate() {
    if (_isUpdateOperationBusy())                              { return false; }
    if (!_isUpdateAvailable(OperationTarget::FIRMWARE))        { return false; }
    if (!_validateWifiPrerequisite(OperationTarget::FIRMWARE)) { return false; }
    return _startTask(_installFirmwareTask, "OTA install", 12288, OperationTarget::FIRMWARE, Status::DOWNLOADING);
}

bool update::startSotaUpdate() {
    return _startDatabaseUpdate(OperationTarget::SOTA, _installSotaTask, "SOTA install");
}

bool update::startPotaUpdate() {
    return _startDatabaseUpdate(OperationTarget::POTA, _installPotaTask, "POTA install");
}

bool update::isBusy() {
    portENTER_CRITICAL(&_lock);
    const bool busy = _taskRunning;
    portEXIT_CRITICAL(&_lock);
    return busy;
}

update::FirmwareSnapshot update::firmwareSnapshot() {
    FirmwareSnapshot value;
    portENTER_CRITICAL(&_lock);
    value.status   = _firmwareStatus;
    value.progress = _firmwareProgress;
    text::copy(value.latestVersion, sizeof(value.latestVersion), _firmwareLatestVersion);
    text::copy(value.error,         sizeof(value.error),         _firmwareError );
    portEXIT_CRITICAL(&_lock);
    return value;
}

update::SotaSnapshot update::sotaSnapshot() {
    SotaSnapshot value;
    portENTER_CRITICAL(&_lock);
    value.status   = _sotaStatus;
    value.progress = _sotaProgress;
    value.records  = _sotaRecords;
    text::copy(value.installedVersion, sizeof(value.installedVersion), _sotaInstalledVersion);
    text::copy(value.latestVersion,    sizeof(value.latestVersion),    _sotaLatestVersion);
    text::copy(value.error,            sizeof(value.error),            _sotaError);
    portEXIT_CRITICAL(&_lock);
    return value;
}

update::PotaSnapshot update::potaSnapshot() {
    PotaSnapshot value;
    portENTER_CRITICAL(&_lock);
    value.status   = _potaStatus;
    value.progress = _potaProgress;
    value.records  = _potaRecords;
    text::copy(value.error, sizeof(value.error), _potaError);
    portEXIT_CRITICAL(&_lock);
    return value;
}

const char* update::firmwareVersion() {
    return PROJECT_VERSION;
}
