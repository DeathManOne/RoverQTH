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

#include "database/pota.h"
#include "database/sota.h"
#include "security/otaRootCA.h"
#include "services/pota.h"
#include "services/sota.h"
#include "services/storage.h"
#include "services/update.h"
#include "services/wifi.h"
#include "utilities/hash.h"
#include "utilities/json.h"
#include "utilities/sota.h"
#include "utilities/text.h"
#include "utilities/version.h"

namespace potaDB    = database::pota;
namespace sotaDB    = database::sota;
namespace otaRootCA = security::otaRootCA;
namespace pota      = services::pota;
namespace sota      = services::sota;
namespace storage   = services::storage;
namespace update    = services::update;
namespace wifi      = services::wifi;
namespace hash      = utilities::hash;
namespace json      = utilities::json;
namespace uSota     = utilities::sota;
namespace text      = utilities::text;
namespace uVersion  = utilities::version;

namespace {
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

    constexpr size_t DOWNLOAD_BUFFER_SIZE = 4096;
    constexpr size_t MANIFEST_BUFFER_SIZE = 512U;
    constexpr uint32_t HTTP_TIMEOUT_MS    = 15000;
    constexpr uint32_t STREAM_TIMEOUT_MS  = 20000;
    constexpr const char* POTA_CSV_PATH   = "/RoverQTH/tmp/pota.tmp";
    constexpr const char* SOTA_CSV_PATH   = "/RoverQTH/tmp/sota.tmp";

    enum class OperationTarget : uint8_t {FIRMWARE, SOTA, POTA};
    enum class DatabaseDownloadFailure : uint8_t {
        NONE,
        STREAM_DISCONNECTED,
        STREAM_TIMEOUT,
        SD_WRITE
    };

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
        char etag[sotaDB::ETAG_SIZE] {};
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

    void _setFirmwareStatus(update::Status value);
    void _setFirmwareProgress(uint8_t value);
    void _setFirmwareError(const char* value, const char* logCode);
    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* url, DatabaseSourceInfo* sourceInfo = nullptr);
    bool _openHead(HTTPClient& http, WiFiClientSecure& client, const char* url, DatabaseSourceInfo& sourceInfo);
    bool _readDatabaseSourceInfo(HTTPClient& http, DatabaseSourceInfo& sourceInfo);
    bool _readResponseBody(HTTPClient& http, char* buffer, size_t size, void (*progressCallback)(uint8_t) = nullptr);
    void _finishTask();
    void _checkFirmwareTask(void*);
    void _checkSotaTask(void*);
    void _checkPotaTask(void*);
    void _installFirmwareTask(void*);
    void _installSotaTask(void*);
    void _installPotaTask(void*);
    void _sotaDatabaseProgress(uint8_t progress, void* userData);
    void _potaDatabaseProgress(uint8_t progress, void* userData);
    void _setSotaStatus(update::Status value);
    void _setSotaProgress(uint8_t value);
    void _setSotaError(const char* value, const char* logCode);
    void _setPotaStatus(update::Status value);
    void _setPotaProgress(uint8_t value);
    void _setPotaError(const char* value, const char* logCode);
    bool _startTask(TaskFunction_t function, const char* name, uint32_t stackSize, OperationTarget target, update::Status initialStatus);
    bool _readSotaVersionChunk(const uint8_t* data, size_t length, void* userData);
    bool _readSotaVersion(char* version, size_t size);
    bool _downloadSotaCsv(char* version, size_t versionSize, DatabaseSourceInfo& sourceInfo);
    bool _downloadPotaCsv(DatabaseSourceInfo& sourceInfo);

    void _setFirmwareStatus(update::Status value) {
        portENTER_CRITICAL(&_lock);
        _firmwareStatus = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setFirmwareProgress(uint8_t value) {
        if (value > 100) { value = 100; }
        portENTER_CRITICAL(&_lock);
        _firmwareProgress = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setFirmwareError(const char* value, const char* logCode) {
        portENTER_CRITICAL(&_lock);

        text::copy(_firmwareError, sizeof(_firmwareError), value);
        _firmwareStatus = update::Status::ERROR;

        portEXIT_CRITICAL(&_lock);
        if (logCode != nullptr)
            { storage::appendErrorRecord(logCode); }
    }

    bool _openGet(HTTPClient& http, WiFiClientSecure& client, const char* const url, DatabaseSourceInfo* const sourceInfo) {
        client.setCACert(otaRootCA::OTA_ROOT_CA);
        client.setTimeout(HTTP_TIMEOUT_MS);

        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        if (!http.begin(client, url)) {
            storage::appendErrorRecord(
                sourceInfo != nullptr
                    ? "DATABASE_GET_BEGIN_FAILED"
                    : "OTA_GET_BEGIN_FAILED"
            );
            return false;
        }

        if (sourceInfo != nullptr) {
            static const char* HEADERS[] = {"ETag"};
            http.collectHeaders(HEADERS, 1U);
        }

        const int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            char logCode[48];
            snprintf(
                logCode,
                sizeof(logCode),
                sourceInfo != nullptr
                    ? "DATABASE_GET_HTTP_%d"
                    : "OTA_GET_HTTP_%d",
                httpCode
            );
            storage::appendErrorRecord(logCode);
            return false;
        }

        return sourceInfo == nullptr || _readDatabaseSourceInfo(http, *sourceInfo);
    }

    bool _openHead(HTTPClient& http, WiFiClientSecure& client, const char* const url, DatabaseSourceInfo& sourceInfo) {
        client.setCACert(otaRootCA::OTA_ROOT_CA);
        client.setTimeout(HTTP_TIMEOUT_MS);

        http.setConnectTimeout(HTTP_TIMEOUT_MS);
        http.setTimeout(HTTP_TIMEOUT_MS);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        if (!http.begin(client, url)) {
            storage::appendErrorRecord("DATABASE_HEAD_BEGIN_FAILED");
            return false;
        }

        static const char* HEADERS[] = {"ETag"};
        http.collectHeaders(HEADERS, 1U);

        const int httpCode = http.sendRequest("HEAD");
        if (httpCode != HTTP_CODE_OK) {
            char logCode[48];
            snprintf(
                logCode,
                sizeof(logCode),
                "DATABASE_HEAD_HTTP_%d",
                httpCode
            );
            storage::appendErrorRecord(logCode);
            return false;
        }

        return _readDatabaseSourceInfo(http, sourceInfo);
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

        if (!text::copy(
            sourceInfo.etag,
            sizeof(sourceInfo.etag),
            etag.c_str()
        )) {
            sourceInfo = DatabaseSourceInfo {};
            storage::appendErrorRecord("DATABASE_ETAG_TOO_LONG");
            return false;
        }

        return true;
    }

    bool _readResponseBody(HTTPClient& http, char* const buffer, const size_t size, void (*progressCallback)(uint8_t)) {
        if (buffer == nullptr || size < 2U) { return false; }

        buffer[0] = '\0';
        const int announcedSize = http.getSize();
        if (announcedSize >= 0 && static_cast<size_t>(announcedSize) >= size) { return false; }

        WiFiClient* const stream = http.getStreamPtr();
        if (stream == nullptr) { return false; }

        size_t received     = 0U;
        uint32_t lastDataAt = millis();

        if (progressCallback != nullptr && announcedSize > 0) {
            progressCallback(static_cast<uint8_t>(
                (static_cast<uint64_t>(received) * 100ULL) /
                static_cast<size_t>(announcedSize))
            );
        }

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
                const int read = stream->readBytes(
                    reinterpret_cast<uint8_t*>(buffer + received),
                    toRead
                );

                if (read <= 0) {
                    if (!http.connected() ||
                        millis() - lastDataAt > STREAM_TIMEOUT_MS
                    ) {
                        buffer[0] = '\0';
                        return false;
                    }

                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                }

                received += static_cast<size_t>(read);
                lastDataAt = millis();

                if (progressCallback != nullptr && announcedSize > 0) {
                    progressCallback(static_cast<uint8_t>(
                        (static_cast<uint64_t>(received) * 100ULL) /
                        static_cast<size_t>(announcedSize)
                    ));
                }

                if (announcedSize >= 0 &&
                    received == static_cast<size_t>(announcedSize)
                ) { break; }
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

    void _finishTask() {
        portENTER_CRITICAL(&_lock);
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);
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
        const bool manifestRead =_readResponseBody(
            http, manifest, sizeof(manifest), _setFirmwareProgress
        );
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
        uVersion::Comparison comparison;
    
        if (!uVersion::compare(remoteVersion, PROJECT_VERSION, comparison) ||
            size == 0 || !hash::isSha256Text(sha256)
        ) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            return;
        }

        char normalizedSha[hash::SHA256_TEXT_SIZE];
        if (!hash::normalizeSha256Text(sha256, normalizedSha, sizeof(normalizedSha))) {
            _setFirmwareError("Invalid manifest", "OTA_MANIFEST_FIELDS_INVALID");
            return;
        }

        const bool updateAvailable = comparison == uVersion::Comparison::NEWER;
        portENTER_CRITICAL(&_lock);
    
        text::copy(_firmwareLatestVersion,  sizeof(_firmwareLatestVersion),  remoteVersion);
        text::copy(_firmwareExpectedSha256, sizeof(_firmwareExpectedSha256), normalizedSha);

        _firmwareExpectedSize = size;
        _firmwareProgress     = 0;
        _firmwareError[0]     = '\0';
        _firmwareStatus       = updateAvailable
            ? update::Status::AVAILABLE
            : update::Status::UP_TO_DATE;

        portEXIT_CRITICAL(&_lock);
        storage::appendLogRecord(updateAvailable ? "OTA_UPDATE_AVAILABLE" : "OTA_UP_TO_DATE");
    }

    void _checkSotaTask(void*) {
        TaskFinalizer finalizer;

        if (!storage::deleteFile(SOTA_CSV_PATH)) {
            _setSotaError(
                "SOTA temporary file unavailable",
                "SOTA_TEMP_DELETE_FAILED"
            );
            return;
        }

        _setSotaProgress(10U);

        WiFiClientSecure client;
        HTTPClient http;
        DatabaseSourceInfo remote;

        if (!_openHead(http, client, DL_SOTA, remote)) {
            http.end();
            _setSotaError("SOTA check failed", "SOTA_HEAD_FAILED");
            return;
        }

        http.end();
        _setSotaProgress(75U);

        sotaDB::Info installed;
        const bool installedAvailable = sotaDB::info(installed);
        const bool updateAvailable    =
            !installedAvailable                        ||
            !text::equals(installed.etag, remote.etag) ||
            installed.sourceSize != remote.size;

        if (!installedAvailable) {
            sota::begin();
        }

        portENTER_CRITICAL(&_lock);
        _sotaRecords = installedAvailable ? installed.records : 0U;
        text::copy(
            _sotaInstalledVersion,
            sizeof(_sotaInstalledVersion),
            installedAvailable ? installed.version : ""
        );
        _sotaLatestVersion[0] = '\0';
        _sotaProgress         = 100U;
        _sotaStatus           = updateAvailable
            ? update::Status::AVAILABLE
            : update::Status::UP_TO_DATE;
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord(updateAvailable ? "SOTA_UPDATE_AVAILABLE" : "SOTA_UP_TO_DATE");
    }

    void _checkPotaTask(void*) {
        TaskFinalizer finalizer;

        if (!storage::deleteFile(POTA_CSV_PATH)) {
            _setPotaError(
                "POTA temporary file unavailable",
                "POTA_TEMP_DELETE_FAILED"
            );
            return;
        }

        _setPotaProgress(10U);

        WiFiClientSecure client;
        HTTPClient http;
        DatabaseSourceInfo remote;

        if (!_openHead(http, client, DL_POTA, remote)) {
            http.end();
            _setPotaError("POTA check failed", "POTA_HEAD_FAILED");
            return;
        }

        http.end();
        _setPotaProgress(75U);

        potaDB::Info installed;
        const bool installedAvailable = potaDB::info(installed);
        const bool updateAvailable =
            !installedAvailable                        ||
            !text::equals(installed.etag, remote.etag) ||
            installed.sourceSize != remote.size;

        if (!installedAvailable) {
            pota::begin();
        }

        portENTER_CRITICAL(&_lock);
        _potaRecords  = installedAvailable ? installed.records : 0U;
        _potaProgress = 100U;
        _potaStatus   = updateAvailable
            ? update::Status::AVAILABLE
            : update::Status::UP_TO_DATE;
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord(
            updateAvailable
                ? "POTA_UPDATE_AVAILABLE"
                : "POTA_UP_TO_DATE"
        );
    }

    void _installFirmwareTask(void*) {
        TaskFinalizer finalizer;

        uint32_t expectedSize = 0;
        char expectedSha[hash::SHA256_TEXT_SIZE];

        portENTER_CRITICAL(&_lock);
        expectedSize = _firmwareExpectedSize;
        text::copy(expectedSha, sizeof(expectedSha), _firmwareExpectedSha256);
        portEXIT_CRITICAL(&_lock);

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
            if (read <= 0) {
                if (!http.connected() ||
                    millis() - lastDataAt > STREAM_TIMEOUT_MS
                ) {
                    transferOk = false;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

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
            _setFirmwareProgress(static_cast<uint8_t>((static_cast<uint64_t>(received) * 100ULL) / expectedSize));
        }

        http.end();
        if (!transferOk || received != expectedSize) {
            ::Update.abort();
            _setFirmwareError("Download interrupted", "OTA_DOWNLOAD_INCOMPLETE");
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

    void _sotaDatabaseProgress(const uint8_t progress, void*) {
        const uint8_t mapped = static_cast<uint8_t>(
            60U + (static_cast<uint16_t>(progress) * 35U) / 100U
        );
        _setSotaProgress(mapped);
    }

    void _potaDatabaseProgress(const uint8_t progress, void*) {
        const uint8_t mapped = static_cast<uint8_t>(
            60U + (static_cast<uint16_t>(progress) * 35U) / 100U
        );
        _setPotaProgress(mapped);
    }

    void _installSotaTask(void*) {
        TaskFinalizer finalizer;
        WiFiClientSecure client;
        HTTPClient http;
        DatabaseSourceInfo remote;

        if (!_openHead(http, client, DL_SOTA, remote)) {
            http.end();
            _setSotaError("SOTA check failed", "SOTA_HEAD_FAILED");
            return;
        }
        http.end();

        sotaDB::Info current;
        if (sotaDB::info(current)                   &&
            text::equals(current.etag, remote.etag) &&
            current.sourceSize == remote.size
        ) {
            portENTER_CRITICAL(&_lock);
            _sotaRecords = current.records;
            text::copy(
                _sotaInstalledVersion,
                sizeof(_sotaInstalledVersion),
                current.version
            );
            _sotaProgress = 100U;
            _sotaStatus   = update::Status::UP_TO_DATE;
            portEXIT_CRITICAL(&_lock);

            storage::appendLogRecord("SOTA_UP_TO_DATE");
            return;
        }

        if (!storage::deleteFile(SOTA_CSV_PATH)) {
            _setSotaError(
                "SOTA temporary file unavailable",
                "SOTA_TEMP_DELETE_FAILED"
            );
            return;
        }

        char version[update::SOTA_VERSION_SIZE] {};
        DatabaseSourceInfo downloaded;

        _setSotaStatus(update::Status::DOWNLOADING);
        _setSotaProgress(0U);

        if (!_downloadSotaCsv(version, sizeof(version), downloaded)) {
            _setSotaError("SOTA download failed", nullptr);
            return;
        }

        portENTER_CRITICAL(&_lock);
        text::copy(_sotaLatestVersion, sizeof(_sotaLatestVersion), version);

        _sotaProgress = 60U;
        _sotaStatus   = update::Status::INSTALLING;
        portEXIT_CRITICAL(&_lock);

        if (!sotaDB::buildCandidate(
            SOTA_CSV_PATH,
            version,
            downloaded.etag,
            downloaded.size,
            _sotaDatabaseProgress
        )) {
            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            _setSotaError("Invalid SOTA database", "SOTA_BUILD_FAILED");
            return;
        }

        _setSotaProgress(95U);
        _setSotaStatus(update::Status::VERIFYING);

        if (!sotaDB::installCandidate()) {
            sotaDB::discardCandidate();

            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            _setSotaError("SOTA install failed", "SOTA_INSTALL_FAILED");
            return;
        }

        sota::invalidate();

        sotaDB::Info installedInfo;
        if (!sotaDB::info(installedInfo)                       ||
            !text::equals(installedInfo.etag, downloaded.etag) ||
            installedInfo.sourceSize != downloaded.size
        ) {
            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            sota::begin();
            _setSotaError("SOTA validation failed", "SOTA_VALIDATION_FAILED");
            return;
        }

        if (!storage::deleteFile(SOTA_CSV_PATH)) {
            storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
        }

        portENTER_CRITICAL(&_lock);
        _sotaRecords = installedInfo.records;
        text::copy(_sotaInstalledVersion, sizeof(_sotaInstalledVersion), installedInfo.version);

        _sotaProgress = 100U;
        _sotaStatus   = update::Status::SUCCESS;
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord("SOTA_UPDATE_SUCCESS");
    }

    void _installPotaTask(void*) {
        TaskFinalizer finalizer;
        WiFiClientSecure client;
        HTTPClient http;
        DatabaseSourceInfo remote;

        if (!_openHead(http, client, DL_POTA, remote)) {
            http.end();
            _setPotaError("POTA check failed", "POTA_HEAD_FAILED");
            return;
        }

        http.end();

        potaDB::Info current;
        if (
            potaDB::info(current)                   &&
            text::equals(current.etag, remote.etag) &&
            current.sourceSize == remote.size
        ) {
            portENTER_CRITICAL(&_lock);
            _potaRecords  = current.records;
            _potaProgress = 100U;
            _potaStatus   = update::Status::UP_TO_DATE;
            portEXIT_CRITICAL(&_lock);

            storage::appendLogRecord("POTA_UP_TO_DATE");
            return;
        }

        if (!storage::deleteFile(POTA_CSV_PATH)) {
            _setPotaError(
                "POTA temporary file unavailable",
                "POTA_TEMP_DELETE_FAILED"
            );
            return;
        }

        DatabaseSourceInfo downloaded;

        _setPotaStatus(update::Status::DOWNLOADING);
        _setPotaProgress(0U);

        if (!_downloadPotaCsv(downloaded)) {
            _setPotaError("POTA download failed", nullptr);
            return;
        }

        portENTER_CRITICAL(&_lock);
        _potaProgress = 60U;
        _potaStatus   = update::Status::INSTALLING;
        portEXIT_CRITICAL(&_lock);

        if (!potaDB::buildCandidate(
            POTA_CSV_PATH,
            downloaded.etag,
            downloaded.size,
            _potaDatabaseProgress
        )) {
            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            _setPotaError("Invalid POTA database", "POTA_BUILD_FAILED");
            return;
        }

        _setPotaProgress(95U);
        _setPotaStatus(update::Status::VERIFYING);

        if (!potaDB::installCandidate()) {
            potaDB::discardCandidate();

            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            _setPotaError("POTA install failed", "POTA_INSTALL_FAILED");
            return;
        }

        pota::invalidate();

        potaDB::Info installedInfo;
        if (!potaDB::info(installedInfo)                       ||
            !text::equals(installedInfo.etag, downloaded.etag) ||
            installedInfo.sourceSize != downloaded.size
        ) {
            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            pota::begin();
            _setPotaError(
                "POTA validation failed",
                "POTA_VALIDATION_FAILED"
            );
            return;
        }

        if (!storage::deleteFile(POTA_CSV_PATH)) {
            storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
        }

        portENTER_CRITICAL(&_lock);
        _potaRecords  = installedInfo.records;
        _potaProgress = 100U;
        _potaStatus   = update::Status::SUCCESS;
        portEXIT_CRITICAL(&_lock);

        storage::appendLogRecord("POTA_UPDATE_SUCCESS");
    }

    bool _startTask(const TaskFunction_t function, const char* const name, const uint32_t stackSize, const OperationTarget target, const update::Status initialStatus) {
        if (sota::isBusy() || pota::isBusy()) { return false; }

        portENTER_CRITICAL(&_lock);
        if (_taskRunning) {
            portEXIT_CRITICAL(&_lock);
            return false;
        }
        _taskRunning = true;
        portEXIT_CRITICAL(&_lock);

        if (sota::isBusy() || pota::isBusy()) {
            portENTER_CRITICAL(&_lock);
            _taskRunning = false;
            portEXIT_CRITICAL(&_lock);
            return false;
        }

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

        portENTER_CRITICAL(&_lock);
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);

        if (target == OperationTarget::FIRMWARE)
            { _setFirmwareError("Task creation failed", "OTA_TASK_CREATE_FAILED"); }
        else if (target == OperationTarget::SOTA)
            { _setSotaError("Task creation failed", "SOTA_TASK_CREATE_FAILED"); }
        else
            { _setPotaError("Task creation failed", "POTA_TASK_CREATE_FAILED"); }

        return false;
    }

    void _setSotaStatus(const update::Status value) {
        portENTER_CRITICAL(&_lock);
        _sotaStatus = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setSotaProgress(uint8_t value) {
        if (value > 100U) { value = 100U; }
        portENTER_CRITICAL(&_lock);
        _sotaProgress = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setSotaError(const char* const value, const char* const logCode) {
        portENTER_CRITICAL(&_lock);
        text::copy(_sotaError, sizeof(_sotaError), value);
        _sotaStatus = update::Status::ERROR;
        portEXIT_CRITICAL(&_lock);
        if (logCode != nullptr)
            { storage::appendErrorRecord(logCode); }
    }

    void _setPotaStatus(const update::Status value) {
        portENTER_CRITICAL(&_lock);
        _potaStatus = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setPotaProgress(uint8_t value) {
        if (value > 100U) { value = 100U; }
        portENTER_CRITICAL(&_lock);
        _potaProgress = value;
        portEXIT_CRITICAL(&_lock);
    }

    void _setPotaError(const char* const value, const char* const logCode) {
        portENTER_CRITICAL(&_lock);
        text::copy(_potaError, sizeof(_potaError), value);
        _potaStatus = update::Status::ERROR;
        portEXIT_CRITICAL(&_lock);

        if (logCode != nullptr)
            { storage::appendErrorRecord(logCode); }
    }

    bool _readSotaVersionChunk(const uint8_t* const data, const size_t length, void* const userData) {
        if (data == nullptr || userData == nullptr) { return false; }

        SotaVersionContext* const context = static_cast<SotaVersionContext*>(userData);
        if (!context->valid || context->parsed) { return false; }

        for (size_t index = 0U; index < length; ++index) {
            const char character = static_cast<char>(data[index]);

            if (character == '\r') { continue; }
            if (character == '\n') {
                context->line[context->length] = '\0';
                context->parsed = uSota::parseListVersion(context->line, context->version, context->size);
                context->valid  = context->parsed;
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
        if (version == nullptr || size < uSota::VERSION_SIZE) { return false; }

        version[0] = '\0';
        SotaVersionContext context {version, size, {}, 0U, false, true};

        return
            storage::readFileChunks(SOTA_CSV_PATH, _readSotaVersionChunk, &context) &&
            context.valid && context.parsed;
    }

    bool _downloadSotaCsv(char* const version, const size_t versionSize, DatabaseSourceInfo& sourceInfo) {
        sourceInfo = DatabaseSourceInfo {};

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_SOTA, &sourceInfo)) {
            http.end();
            storage::appendErrorRecord("SOTA_HTTP_FAILED");
            return false;
        }

        WiFiClient* const stream = http.getStreamPtr();
        const int announcedSize  = http.getSize();

        if (stream == nullptr) {
            http.end();
            storage::appendErrorRecord("SOTA_STREAM_UNAVAILABLE");
            return false;
        }

        if (!storage::beginFileWrite(SOTA_CSV_PATH)) {
            http.end();
            storage::appendErrorRecord("SOTA_SD_OPEN_FAILED");
            return false;
        }

        uint8_t buffer[DOWNLOAD_BUFFER_SIZE];
        size_t received             = 0U;
        uint32_t lastDataAt         = millis();
        DatabaseDownloadFailure failure = DatabaseDownloadFailure::NONE;

        while (failure == DatabaseDownloadFailure::NONE &&
            (announcedSize < 0 || received < static_cast<size_t>(announcedSize))
        ) {
            const int available = stream->available();

            if (available <= 0) {
                if (!http.connected()) {
                    if (announcedSize < 0) { break; }
                    failure = DatabaseDownloadFailure::STREAM_DISCONNECTED;
                    break;
                }
                if (millis() - lastDataAt > STREAM_TIMEOUT_MS)
                    { failure = DatabaseDownloadFailure::STREAM_TIMEOUT; }
                else { vTaskDelay(pdMS_TO_TICKS(10)); }
                continue;
            }

            size_t length = static_cast<size_t>(available);
            if (length > sizeof(buffer)) { length = sizeof(buffer); }

            if (announcedSize > 0) {
                const size_t remaining =
                    static_cast<size_t>(announcedSize) - received;

                if (length > remaining) {
                    length = remaining;
                }
            }

            const int read = stream->readBytes(buffer, length);
            if (read <= 0) {
                if (!http.connected()) {
                    failure = DatabaseDownloadFailure::STREAM_DISCONNECTED;
                } else if (millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    failure = DatabaseDownloadFailure::STREAM_TIMEOUT;
                } else {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                continue;
            }

            const size_t readLength = static_cast<size_t>(read);
            if (!storage::writeFileChunk(buffer, readLength)) {
                failure = DatabaseDownloadFailure::SD_WRITE;
                break;
            }

            received += readLength;
            lastDataAt = millis();

            if (announcedSize > 0) {
                _setSotaProgress(
                    static_cast<uint8_t>(
                        (static_cast<uint64_t>(received) * 60ULL) /
                        static_cast<size_t>(announcedSize)
                    )
                );
            }
        }

        storage::endFileWrite();
        http.end();

        if (failure != DatabaseDownloadFailure::NONE) {
            switch (failure) {
                case DatabaseDownloadFailure::STREAM_DISCONNECTED:
                    storage::appendErrorRecord("SOTA_STREAM_DISCONNECTED");
                    break;
                case DatabaseDownloadFailure::STREAM_TIMEOUT:
                    storage::appendErrorRecord("SOTA_STREAM_TIMEOUT");
                    break;
                case DatabaseDownloadFailure::SD_WRITE:
                    storage::appendErrorRecord("SOTA_SD_WRITE_FAILED");
                    break;
                case DatabaseDownloadFailure::NONE:
                default:
                    break;
            }
            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            return false;
        }

        if (received == 0U) {
            storage::appendErrorRecord("SOTA_EMPTY_RESPONSE");

            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            return false;
        }
        if (received != sourceInfo.size) {
            storage::appendErrorRecord("SOTA_SIZE_MISMATCH");

            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            return false;
        }
        if (storage::fileSize(SOTA_CSV_PATH) != received) {
            storage::appendErrorRecord("SOTA_FILE_SIZE_MISMATCH");

            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            return false;
        }
        if (!_readSotaVersion(version, versionSize)) {
            storage::appendErrorRecord("SOTA_VERSION_INVALID");

            if (!storage::deleteFile(SOTA_CSV_PATH)) {
                storage::appendErrorRecord("SOTA_TEMP_DELETE_FAILED");
            }

            return false;
        }

        return true;
    }

    bool _downloadPotaCsv(DatabaseSourceInfo& sourceInfo) {
        sourceInfo = DatabaseSourceInfo {};

        WiFiClientSecure client;
        HTTPClient http;

        if (!_openGet(http, client, DL_POTA, &sourceInfo)) {
            http.end();
            storage::appendErrorRecord("POTA_HTTP_FAILED");
            return false;
        }

        WiFiClient* const stream = http.getStreamPtr();
        const int announcedSize  = http.getSize();

        if (stream == nullptr) {
            http.end();
            storage::appendErrorRecord("POTA_STREAM_UNAVAILABLE");
            return false;
        }

        if (!storage::beginFileWrite(POTA_CSV_PATH)) {
            http.end();
            storage::appendErrorRecord("POTA_SD_OPEN_FAILED");
            return false;
        }

        uint8_t buffer[DOWNLOAD_BUFFER_SIZE];
        size_t received                  = 0U;
        uint32_t lastDataAt              = millis();
        DatabaseDownloadFailure failure = DatabaseDownloadFailure::NONE;

        while (
            failure == DatabaseDownloadFailure::NONE &&
            (
                announcedSize < 0 ||
                received < static_cast<size_t>(announcedSize)
            )
        ) {
            const int available = stream->available();

            if (available <= 0) {
                if (!http.connected()) {
                    if (announcedSize < 0) { break; }

                    failure = DatabaseDownloadFailure::STREAM_DISCONNECTED;
                    break;
                }

                if (millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    failure = DatabaseDownloadFailure::STREAM_TIMEOUT;
                } else {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                continue;
            }

            size_t length = static_cast<size_t>(available);
            if (length > sizeof(buffer)) {
                length = sizeof(buffer);
            }

            if (announcedSize > 0) {
                const size_t remaining =
                    static_cast<size_t>(announcedSize) - received;

                if (length > remaining) {
                    length = remaining;
                }
            }

            const int read = stream->readBytes(buffer, length);
            if (read <= 0) {
                if (!http.connected()) {
                    failure = DatabaseDownloadFailure::STREAM_DISCONNECTED;
                } else if (millis() - lastDataAt > STREAM_TIMEOUT_MS) {
                    failure = DatabaseDownloadFailure::STREAM_TIMEOUT;
                } else {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                continue;
            }

            const size_t readLength = static_cast<size_t>(read);
            if (!storage::writeFileChunk(buffer, readLength)) {
                failure = DatabaseDownloadFailure::SD_WRITE;
                break;
            }

            received  += readLength;
            lastDataAt = millis();

            if (announcedSize > 0) {
                _setPotaProgress(
                    static_cast<uint8_t>(
                        (static_cast<uint64_t>(received) * 60ULL) /
                        static_cast<size_t>(announcedSize)
                    )
                );
            }
        }

        storage::endFileWrite();
        http.end();

        if (failure != DatabaseDownloadFailure::NONE) {
            switch (failure) {
                case DatabaseDownloadFailure::STREAM_DISCONNECTED:
                    storage::appendErrorRecord("POTA_STREAM_DISCONNECTED");
                    break;
                case DatabaseDownloadFailure::STREAM_TIMEOUT:
                    storage::appendErrorRecord("POTA_STREAM_TIMEOUT");
                    break;
                case DatabaseDownloadFailure::SD_WRITE:
                    storage::appendErrorRecord("POTA_SD_WRITE_FAILED");
                    break;
                case DatabaseDownloadFailure::NONE:
                default:
                    break;
            }

            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            return false;
        }

        if (received == 0U) {
            storage::appendErrorRecord("POTA_EMPTY_RESPONSE");

            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            return false;
        }
        if (received != sourceInfo.size) {
            storage::appendErrorRecord("POTA_SIZE_MISMATCH");

            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            return false;
        }
        if (storage::fileSize(POTA_CSV_PATH) != received) {
            storage::appendErrorRecord("POTA_FILE_SIZE_MISMATCH");

            if (!storage::deleteFile(POTA_CSV_PATH)) {
                storage::appendErrorRecord("POTA_TEMP_DELETE_FAILED");
            }

            return false;
        }

        return true;
    }
}

void update::begin() {
    if (sota::isBusy() || pota::isBusy()) {
        storage::appendErrorRecord("UPDATE_INIT_BUSY");
        return;
    }

    portENTER_CRITICAL(&_lock);
    if (_taskRunning) {
        portEXIT_CRITICAL(&_lock);
        storage::appendErrorRecord("UPDATE_INIT_BUSY");
        return;
    }
    _taskRunning = true;
    portEXIT_CRITICAL(&_lock);

    if (sota::isBusy() || pota::isBusy()) {
        portENTER_CRITICAL(&_lock);
        _taskRunning = false;
        portEXIT_CRITICAL(&_lock);
        storage::appendErrorRecord("UPDATE_INIT_BUSY");
        return;
    }

    sotaDB::Info sotaInfo;
    potaDB::Info potaInfo;

    const bool storageReady  = storage::isReady();
    const bool sotaInstalled = storageReady && sotaDB::info(sotaInfo);
    const bool potaInstalled = storageReady && potaDB::info(potaInfo);

    if (sotaInstalled) {
        char databaseRecord[96];
        const int databaseWritten = snprintf(
            databaseRecord, sizeof(databaseRecord),
            "SOTA_DATABASE status=ready version=%s records=%lu source_size=%llu",
            sotaInfo.version,
            static_cast<unsigned long>(sotaInfo.records),
            static_cast<unsigned long long>(sotaInfo.sourceSize)
        );

        if (databaseWritten > 0 &&
            static_cast<size_t>(databaseWritten) < sizeof(databaseRecord)
        ) { storage::appendLogRecord(databaseRecord); }
    } else { storage::appendLogRecord("SOTA_DATABASE status=not_installed"); }

    if (potaInstalled) {
        char databaseRecord[96];
        const int databaseWritten = snprintf(
            databaseRecord,
            sizeof(databaseRecord),
            "POTA_DATABASE status=ready records=%lu source_size=%llu",
            static_cast<unsigned long>(potaInfo.records),
            static_cast<unsigned long long>(potaInfo.sourceSize)
        );

        if (databaseWritten > 0 &&
            static_cast<size_t>(databaseWritten) < sizeof(databaseRecord)
        ) { storage::appendLogRecord(databaseRecord); }
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
    _taskRunning               = false;
    portEXIT_CRITICAL(&_lock);

    storage::appendLogRecord("UPDATE_READY");
}

bool update::checkFirmwareUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    if (!wifi::isConnected()) {
        _setFirmwareError("WiFi not connected", "OTA_WIFI_NOT_CONNECTED");
        return false;
    }
    return _startTask(_checkFirmwareTask, "OTA check", 8192, OperationTarget::FIRMWARE, Status::CHECKING);
}

bool update::checkSotaUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    if (!storage::isReady()) {
        _setSotaError("SD card unavailable", "SOTA_SD_UNAVAILABLE");
        return false;
    }

    if (!wifi::isConnected()) {
        _setSotaError("WiFi not connected", "SOTA_WIFI_NOT_CONNECTED");
        return false;
    }

    return _startTask(_checkSotaTask, "SOTA check", 12288, OperationTarget::SOTA, Status::CHECKING);
}

bool update::checkPotaUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    if (!storage::isReady()) {
        _setPotaError("SD card unavailable", "POTA_SD_UNAVAILABLE");
        return false;
    }

    if (!wifi::isConnected()) {
        _setPotaError("WiFi not connected", "POTA_WIFI_NOT_CONNECTED");
        return false;
    }

    return _startTask(_checkPotaTask, "POTA check", 12288, OperationTarget::POTA, Status::CHECKING);
}

bool update::startFirmwareUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    portENTER_CRITICAL(&_lock);
    const bool available =
        !_taskRunning &&
        _firmwareStatus == Status::AVAILABLE;
    portEXIT_CRITICAL(&_lock);

    if (!available) { return false; }

    if (!wifi::isConnected()) {
        _setFirmwareError("WiFi not connected", "OTA_WIFI_NOT_CONNECTED");
        return false;
    }
    return _startTask(_installFirmwareTask, "OTA install", 12288, OperationTarget::FIRMWARE, Status::DOWNLOADING);
}

bool update::startSotaUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    portENTER_CRITICAL(&_lock);
    const bool available =
        !_taskRunning &&
        _sotaStatus == Status::AVAILABLE;
    portEXIT_CRITICAL(&_lock);

    if (!available) { return false; }

    if (!storage::isReady()) {
        _setSotaError("SD card unavailable", "SOTA_SD_UNAVAILABLE");
        return false;
    }

    if (!wifi::isConnected()) {
        _setSotaError("WiFi not connected", "SOTA_WIFI_NOT_CONNECTED");
        return false;
    }
    return _startTask(_installSotaTask, "SOTA install", 12288, OperationTarget::SOTA, Status::DOWNLOADING);
}

bool update::startPotaUpdate() {
    if (isBusy() || sota::isBusy() || pota::isBusy()) {
        return false;
    }

    portENTER_CRITICAL(&_lock);
    const bool available =
        !_taskRunning &&
        _potaStatus == Status::AVAILABLE;
    portEXIT_CRITICAL(&_lock);

    if (!available) { return false; }

    if (!storage::isReady()) {
        _setPotaError("SD card unavailable", "POTA_SD_UNAVAILABLE");
        return false;
    }

    if (!wifi::isConnected()) {
        _setPotaError("WiFi not connected", "POTA_WIFI_NOT_CONNECTED");
        return false;
    }

    return _startTask(
        _installPotaTask,
        "POTA install",
        12288,
        OperationTarget::POTA,
        Status::DOWNLOADING
    );
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
