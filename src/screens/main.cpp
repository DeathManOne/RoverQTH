/*
 * src/screens/main.cpp
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

#include <cmath>
#include <cstdio>
#include <limits>

#include "screens/main.h"
#include "screens/main/datas.h"
#include "screens/main/locator.h"
#include "screens/header.h"
#include "services/dtc.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/ota.h"
#include "services/settings.h"
#include "ui/mockup/buttons.h"
#include "ui/settings/themes/defaults.h"
#include "utilities/coordinates.h"
#include "utilities/dtc.h"
#include "utilities/format.h"
#include "utilities/locator.h"
#include "utilities/text.h"

using screens::Main;
namespace datas       = screens::main::datas;
namespace locator     = screens::main::locator;
namespace title       = screens::header;
namespace dtc         = services::dtc;
namespace gps         = services::gps;
namespace navigation  = services::navigation;
namespace ota         = services::ota;
namespace settings    = services::settings;
namespace buttons     = ui::mockup::buttons;
namespace theme       = ui::settings::themes::defaults;
namespace coordinates = utilities::coordinates;
namespace uDtc        = utilities::dtc;
namespace format      = utilities::format;
namespace uLocator    = utilities::locator;
namespace text        = utilities::text;

namespace {
    struct MarkView {
        char locator[32]  = "---";
        char distance[16] = "---";
        char bearing[16]  = "---";
        char timer[16]    = "-- : -- : --";
    };

    struct GPSView {
        char date[16]      = {};
        char time[16]      = {};
        char gpsTime[16]   = {};
        char latitude[32]  = {};
        char longitude[32] = {};
        char locator[32]   = {};
        char speed[16]     = {};
        char heading[16]   = {};
        char altitude[16]  = {};
        char status[32]    = {};
    };

    void _getFormattedPosition(const gps::Snapshot &snapshot,
        const settings::CoordinateFormat coordinateFormat,
        char* const latitude,  const size_t latitudeSize,
        char* const longitude, const size_t longitudeSize,
        char* const qth,       const size_t qthSize
    );
    void _prepareGPSView(GPSView &view);
    void _prepareMarkView(MarkView &view);
    void _updateOTAStatus(ST7796S::MSP4021 &tft, const char* status);
    void _formatGPSStatus(const gps::Snapshot &snapshot, char* buffer, size_t size);

    void _prepareGPSView(GPSView &view) {
        const settings::General configuration = settings::general();
        const bool imperial =
            configuration.units == settings::Units::IMPERIAL;

        gps::Snapshot snapshot {};
        gps::getSnapshot(snapshot);

        dtc::getDate(view.date, sizeof(view.date));
        dtc::getTime(view.time, sizeof(view.time));

        if (!uDtc::formatTime(
            snapshot.hour,
            snapshot.minute,
            snapshot.second,
            snapshot.timeValid,
            view.gpsTime,
            sizeof(view.gpsTime)
        )) {
            text::copy(view.gpsTime, sizeof(view.gpsTime), "-- : -- : --");
        }

        _getFormattedPosition(
            snapshot,
            configuration.coordinateFormat,
            view.latitude,
            sizeof(view.latitude),
            view.longitude,
            sizeof(view.longitude),
            view.locator,
            sizeof(view.locator)
        );

        const bool moving = snapshot.speed >= gps::MIN_HEADING_SPEED_KMH;

        const double heading = moving
            ? snapshot.heading
            : std::numeric_limits<double>::quiet_NaN();

        const double speed = moving ? snapshot.speed : 0.0;

        if (!format::heading(
            heading,
            view.heading,
            sizeof(view.heading)
        )) {
            text::copy(view.heading, sizeof(view.heading), "--");
        }

        if (!format::speed(
            speed,
            imperial,
            view.speed,
            sizeof(view.speed)
        )) {
            text::copy(view.speed, sizeof(view.speed), "--");
        }

        if (
            !snapshot.positionValid ||
            !format::altitude(
                snapshot.altitude,
                imperial,
                view.altitude,
                sizeof(view.altitude)
            )
        ) {
            text::copy(view.altitude, sizeof(view.altitude), "--");
        }

        _formatGPSStatus(snapshot, view.status, sizeof(view.status));
    }

    void _prepareMarkView(MarkView &view) {
        navigation::MarkDisplaySnapshot mark {};

        if (!navigation::getMarkDisplaySnapshot(mark)) { return; }

        const settings::General configuration = settings::general();
        const bool imperial =
            configuration.units == settings::Units::IMPERIAL;

        if (!uLocator::fromCoordinates(
            mark.start.latitude,
            mark.start.longitude,
            view.locator,
            sizeof(view.locator)
        )) {
            text::copy(view.locator, sizeof(view.locator), "---");
        }

        if (!format::distance(
            mark.distanceKm,
            imperial,
            view.distance,
            sizeof(view.distance)
        )) {
            text::copy(view.distance, sizeof(view.distance), "---");
        }

        if (!format::bearing(
            mark.bearingDeg,
            view.bearing,
            sizeof(view.bearing)
        )) {
            text::copy(view.bearing, sizeof(view.bearing), "---");
        }

        if (!format::durationCompact(
            mark.elapsedSeconds,
            view.timer,
            sizeof(view.timer)
        )) {
            text::copy(view.timer, sizeof(view.timer), "-- : -- : --");
        }
    }

    void _getFormattedPosition(const gps::Snapshot &snapshot,
        const settings::CoordinateFormat coordinateFormat,
        char* const latitude,  const size_t latitudeSize,
        char* const longitude, const size_t longitudeSize,
        char* const qth,       const size_t qthSize
    ) {
        if (!snapshot.positionValid) {
            text::copy(latitude,  latitudeSize,  "--");
            text::copy(longitude, longitudeSize, "--");
            text::copy(qth,       qthSize,       "--");
            return;
        }

        bool latitudeOk     = false;
        bool longitudeOk    = false;

        switch (coordinateFormat) {
            case settings::CoordinateFormat::DD:
                latitudeOk  = coordinates::formatDD(snapshot.latitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDD(snapshot.longitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
            case settings::CoordinateFormat::DMS:
                latitudeOk  = coordinates::formatDMS(snapshot.latitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDMS(snapshot.longitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
            case settings::CoordinateFormat::DDM:
            default:
                latitudeOk  = coordinates::formatDDM(snapshot.latitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDDM(snapshot.longitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
        }

        if (!latitudeOk)  { text::copy(latitude,  latitudeSize,  "--"); }
        if (!longitudeOk) { text::copy(longitude, longitudeSize, "--"); }
        if (!uLocator::fromCoordinates(snapshot.latitude, snapshot.longitude, qth, qthSize))
            { text::copy(qth, qthSize, "--"); }
    }

    void _updateOTAStatus(ST7796S::MSP4021 &tft, const char* status) {
        locator::updateOTABearing (tft, "---");
        locator::updateOTADistance(tft, "---");
        locator::updateOTAPoints  (tft, "---");
        locator::updateOTAAltitude(tft, "---");
        locator::updateOTACode    (tft, status);
    }

    void _formatGPSStatus(const gps::Snapshot &snapshot, char* const buffer, const size_t size) {
        const char* fix = "--";

        if (snapshot.fixValid) {
            if (snapshot.fixType >= 3U)      { fix = "3D"; }
            else if (snapshot.fixType == 2U) { fix = "2D"; }
        }

        const bool hdopValid =
            std::isfinite(snapshot.hdop) &&
            snapshot.hdop > 0.0;

        const int written = hdopValid
            ? std::snprintf(
                buffer,
                size,
                "FIX %s  SAT %02u  HDOP %.1f",
                fix,
                static_cast<unsigned>(snapshot.satellites),
                snapshot.hdop
            )
            : std::snprintf(
                buffer,
                size,
                "FIX %s  SAT %02u  HDOP --",
                fix,
                static_cast<unsigned>(snapshot.satellites)
            );

        if (
            written < 0 ||
            static_cast<size_t>(written) >= size
        ) {
            text::copy(buffer, size, "FIX --  SAT --  HDOP --");
        }
    }
}

void Main::preload() {
    char battery[8] = {};

    title::getBatteryLevel(battery, sizeof(battery));
    title::setBattery(battery);

    _preloadGPS();
    _preloadOTA();
    _preloadMARK();
}

void Main::_preloadGPS() {
    GPSView gpsView {};
    _prepareGPSView(gpsView);

    char callsign[settings::FULL_CALLSIGN_SIZE];

    if (!settings::getFullCallsign(callsign, sizeof(callsign)))
        { text::copy(callsign, sizeof(callsign), "ERROR"); }

    title::setCallsign(callsign);
    title::setDate(gpsView.date);
    title::setTime(gpsView.time);

    datas::setLatitude(gpsView.latitude);
    datas::setLongitude(gpsView.longitude);
    datas::setSpeed(gpsView.speed);
    datas::setHeading(gpsView.heading);
    datas::setASL(gpsView.altitude);
    datas::setUpdate(gpsView.gpsTime);

    locator::setLocator(gpsView.locator);
    locator::setStatusBottom(gpsView.status);
}

void Main::_preloadOTA() {
    _otaSelection = settings::OtaSelection {};
    settings::getOtaSelection(_otaSelection);

    locator::setOTABearing ("---");
    locator::setOTADistance("---");
    locator::setOTAPoints  ("---");
    locator::setOTAAltitude("---");
    locator::setOTACode    ("---");
}

void Main::_preloadMARK() {
    MarkView mark {};
    _prepareMarkView(mark);

    locator::setMarkLocator(mark.locator);
    locator::setMarkBearing(mark.bearing);
    locator::setMarkDistance(mark.distance);
    locator::setMarkTimer(mark.timer);
}

void Main::update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) {
    nextRefreshIn = 1000U;

    char battery[8] = {};
    title::getBatteryLevel(battery, sizeof(battery));
    title::updateBattery(tft, battery);

    GPSView gpsView {};
    _prepareGPSView(gpsView);

    title::updateDate(tft, gpsView.date);
    title::updateTime(tft, gpsView.time);

    datas::updateLatitude(tft, gpsView.latitude);
    datas::updateLongitude(tft, gpsView.longitude);
    datas::updateSpeed(tft, gpsView.speed);
    datas::updateHeading(tft, gpsView.heading);
    datas::updateASL(tft, gpsView.altitude);
    datas::updateUpdate(tft, gpsView.gpsTime);

    locator::updateLocator(tft, gpsView.locator);
    locator::updateStatusBottom(tft, gpsView.status);

    _updateOTA(tft);
    updateMARK(tft);
}

void Main::_updateOTA(ST7796S::MSP4021 &tft) {
    gps::Snapshot gpsData {};
    if (!gps::getSnapshot(gpsData) || !gpsData.positionValid) {
        _updateOTAStatus(tft, "---");
        return;
    }

    if (
        (
            _otaSelection.type != settings::OtaType::SOTA &&
            _otaSelection.type != settings::OtaType::POTA
        ) || _otaSelection.code[0] == '\0'
    ) {
        _updateOTAStatus(tft, "---");
        return;
    }

    const bool isPota = _otaSelection.type == settings::OtaType::POTA;
    const ota::Type selectedType = isPota
        ? ota::Type::PARKS
        : ota::Type::SUMMITS;

    ota::requestByCode(
        selectedType,     _otaSelection.code,
        gpsData.latitude, gpsData.longitude
    );

    const ota::Snapshot result = ota::snapshot(selectedType);

    if (!text::equals(result.reference.code, _otaSelection.code)) {
        const char* const status =
            result.status == ota::Status::SEARCHING
                ? "Searching"
                : "---";

        _updateOTAStatus(tft, status);
        return;
    }

    const settings::General configuration = settings::general();
    const bool imperial = configuration.units == settings::Units::IMPERIAL;

    char bearing[16];
    char distance[16];

    if (!format::bearing(result.bearingDeg, bearing, sizeof(bearing)))
        { text::copy(bearing, sizeof(bearing), "---"); }
    if (!format::distance(result.distanceKm, imperial, distance, sizeof(distance)))
        { text::copy(distance, sizeof(distance), "---"); }

    char points[16]   = "---";
    char altitude[16] = "---";
    char code[32];

    if (!isPota) {
        if (!format::altitude(
            result.reference.altitude,
            imperial,
            altitude,
            sizeof(altitude)
        )) {
            text::copy(altitude, sizeof(altitude), "---");
        }

        const int pointsWritten = result.reference.bonus > 0U
            ? std::snprintf(
                points,
                sizeof(points),
                "%u+%u",
                static_cast<unsigned int>(result.reference.points),
                static_cast<unsigned int>(result.reference.bonus)
            )
            : std::snprintf(
                points,
                sizeof(points),
                "%u",
                static_cast<unsigned int>(result.reference.points)
            );

        if (
            pointsWritten < 0 ||
            static_cast<size_t>(pointsWritten) >= sizeof(points)
        ) {
            text::copy(points, sizeof(points), "---");
        }
    }

    const char* const typeLabel = isPota ? "POTA" : "SOTA";

    const int codeWritten = std::snprintf(
        code,
        sizeof(code),
        "%s: %s",
        typeLabel,
        result.reference.code
    );

    if (
        codeWritten < 0 ||
        static_cast<size_t>(codeWritten) >= sizeof(code)
    ) {
        text::copy(
            code,
            sizeof(code),
            isPota ? "POTA: ---" : "SOTA: ---"
        );
    }

    locator::updateOTABearing (tft, bearing);
    locator::updateOTADistance(tft, distance);
    locator::updateOTAPoints  (tft, points);
    locator::updateOTAAltitude(tft, altitude);
    locator::updateOTACode    (tft, code);
}

void Main::updateMARK(ST7796S::MSP4021 &tft) {
    MarkView mark {};
    _prepareMarkView(mark);

    locator::updateMarkLocator(tft, mark.locator);
    locator::updateMarkBearing(tft, mark.bearing);
    locator::updateMarkDistance(tft, mark.distance);
    locator::updateMarkTimer(tft, mark.timer);
}

void Main::draw(ST7796S::MSP4021 &tft) {
    tft.fillScreen(theme::BLACK);

    title::draw(tft);
    locator::draw(tft);
    datas::draw(tft);

    buttons::draw(tft);
}

bool Main::handleTouch(ST7796S::MSP4021 &, const int, const int) {
    return false;
}
