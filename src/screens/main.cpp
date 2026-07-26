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

#include "screens/main.h"
#include "screens/main/datas.h"
#include "screens/main/locator.h"
#include "screens/main/title.h"
#include "services/battery.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/settings.h"
#include "ui/mockup/buttons.h"
#include "ui/settings/themes/defaults.h"
#include "utilities/clock.h"
#include "utilities/coordinates.h"
#include "utilities/format.h"
#include "utilities/locator.h"
#include "utilities/text.h"

namespace main        = screens::main;
namespace datas       = screens::main::datas;
namespace locator     = screens::main::locator;
namespace title       = screens::main::title;
namespace battery     = services::battery;
namespace gps         = services::gps;
namespace navigation  = services::navigation;
namespace settings    = services::settings;
namespace buttons     = ui::mockup::buttons;
namespace theme       = ui::settings::themes::defaults;
namespace uClock      = utilities::clock;
namespace coordinates = utilities::coordinates;
namespace format      = utilities::format;
namespace uLocator    = utilities::locator;
namespace text        = utilities::text;

namespace {
    void _getFormattedPosition(
        char* const latitude,  const size_t latitudeSize,
        char* const longitude, const size_t longitudeSize,
        char* const qth,       const size_t qthSize
    ) {
        double rawLatitude  = 0.0;
        double rawLongitude = 0.0;
        bool latitudeOk     = false;
        bool longitudeOk    = false;
        gps::getPosition(rawLatitude, rawLongitude);

        switch (settings::getCoordinateFormat()) {
            case settings::CoordinateFormat::DD:
                latitudeOk  = coordinates::formatDD(rawLatitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDD(rawLongitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
            case settings::CoordinateFormat::DMS:
                latitudeOk  = coordinates::formatDMS(rawLatitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDMS(rawLongitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
            case settings::CoordinateFormat::DDM:
            default:
                latitudeOk  = coordinates::formatDDM(rawLatitude,  coordinates::Axis::LATITUDE,  latitude,  latitudeSize);
                longitudeOk = coordinates::formatDDM(rawLongitude, coordinates::Axis::LONGITUDE, longitude, longitudeSize);
                break;
        }

        if (!latitudeOk)  { text::copy(latitude, latitudeSize, "--"); }
        if (!longitudeOk) { text::copy(longitude, longitudeSize, "--"); }
        if (!uLocator::fromCoordinates(rawLatitude, rawLongitude, qth, qthSize)) { text::copy(qth, qthSize, "--"); }
    }
}

void main::preload() {
    char battery[8] = {};

    title::getBatteryLevel(battery, sizeof(battery));
    title::setBattery(battery);

    preloadGPS();
    preloadSOTA();
    preloadMARK();
}

void main::preloadGPS() {
    const bool imperial = settings::getUnits() == settings::Units::IMPERIAL;

    double masl, hdg, speed, hdop, vdop, pdop;
    int satFix, satCount;

    char date[16];
    char time[16];
    char gpsTime[16];
    char latitude[32];
    char longitude[32];
    char qth[32];
    char speedBuffer[16];
    char hdgBuffer[16];
    char aslBuffer[16];
    char gpsStatus[32];
    char callsign[32];

    uint8_t gpsHour   = 0;
    uint8_t gpsMinute = 0;
    uint8_t gpsSecond = 0;

    uClock::getDate(date, sizeof(date));
    uClock::getTime(time, sizeof(time));

    const bool gpsTimeValid = gps::getTime(gpsHour, gpsMinute, gpsSecond);
    uClock::formatTime(gpsHour, gpsMinute, gpsSecond, gpsTimeValid, gpsTime, sizeof(gpsTime));

    gps::getPrecision(masl, hdg, speed);
    gps::getDOP(hdop, vdop, pdop);
    gps::getSat(satFix, satCount);
    _getFormattedPosition(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));

    format::heading (hdg,   hdgBuffer,             sizeof(hdgBuffer));
    format::speed   (speed, imperial, speedBuffer, sizeof(speedBuffer));
    format::altitude(masl,  imperial, aslBuffer,   sizeof(aslBuffer));
    snprintf(
        gpsStatus, sizeof(gpsStatus), "FIX %s  SAT %02d  HDOP %.1f",
        satFix >= 3 ? "3D" : satFix == 2 ? "2D" : "--", satCount, hdop
    );

    if (!settings::getFullCallsign(callsign, sizeof(callsign)))
        { text::copy(callsign, sizeof(callsign), "ERROR"); }
    title::setCallsign   (callsign);
    title::setDate       (date);
    title::setTime       (time);
    //title::setBattery  ("");

    datas::setLatitude   (latitude);
    datas::setLongitude  (longitude);
    datas::setSpeed      (speedBuffer);
    datas::setHeading    (hdgBuffer);
    datas::setASL        (aslBuffer);
    datas::setUpdate     (gpsTime);

    //locator::setStatusTop  ("");
    locator::setLocator      (qth);
    locator::setStatusBottom (gpsStatus);
}

void main::preloadSOTA() {
    const bool imperial = settings::getUnits() == settings::Units::IMPERIAL;

    if (!navigation::hasSOTA()) {
        locator::setSOTABearing("---");
        locator::setSOTADistance("---");
        locator::setSOTAPoints("---");
        locator::setSOTAAltitude("---");
        locator::setSOTACode("---");
        return;
    }

    char distance[16];
    char bearing[16];
    char points[16];
    char altitude[16];
    char code[24];

    format::altitude(navigation::getSOTAAltitude(), imperial, altitude, sizeof(altitude));
    format::distance(navigation::sotaDistanceKm(),  imperial, distance, sizeof(distance));
    format::bearing (navigation::sotaBearingDeg(),  bearing,            sizeof(bearing));
    navigation::getSOTACode(code, sizeof(code));

    snprintf(points, sizeof(points), "%d (+1)", navigation::getSOTAPoints());

    locator::setSOTABearing(bearing);
    locator::setSOTADistance(distance);
    locator::setSOTAPoints(points);
    locator::setSOTAAltitude(altitude);
    locator::setSOTACode(code);
}

void main::preloadMARK() {
    const bool imperial = settings::getUnits() == settings::Units::IMPERIAL;

    if (!services::navigation::hasMark()) {
        locator::setMarkLocator("---");
        locator::setMarkBearing("---");
        locator::setMarkDistance("---");
        locator::setMarkTimer("-- : -- : --");
        return;
    }

    char locator[32];
    char distance[16];
    char bearing[16];
    char timer[16];

    format::distance       (navigation::markCurrentDistanceKm(), imperial, distance, sizeof(distance));
    format::bearing        (navigation::markCurrentBearingDeg(), bearing,            sizeof(bearing));
    format::durationCompact(navigation::markElapsedSeconds(),    timer,              sizeof(timer));
    navigation::getMarkStartLocator(locator, sizeof(locator));

    locator::setMarkLocator(locator);
    locator::setMarkBearing(bearing);
    locator::setMarkDistance(distance);
    locator::setMarkTimer(timer);
}

void main::update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) {
    const bool imperial = settings::getUnits() == settings::Units::IMPERIAL;
    nextRefreshIn = 1000;

    char battery[8] = {};
    title::getBatteryLevel(battery, sizeof(battery));
    title::updateBattery(tft, battery);

    double masl, hdg, speed, hdop, vdop, pdop;
    int satFix, satCount;

    char date[16];
    char time[16];
    char gpsTime[16];
    char latitude[32];
    char longitude[32];
    char qth[32];
    char speedBuffer[16];
    char hdgBuffer[16];
    char aslBuffer[16];
    char gpsStatus[32];

    uint8_t gpsHour   = 0;
    uint8_t gpsMinute = 0;
    uint8_t gpsSecond = 0;

    uClock::getDate(date, sizeof(date));
    uClock::getTime(time, sizeof(time));

    const bool gpsTimeValid = gps::getTime(gpsHour, gpsMinute, gpsSecond);
    uClock::formatTime(gpsHour, gpsMinute, gpsSecond, gpsTimeValid, gpsTime, sizeof(gpsTime));

    gps::getPrecision(masl, hdg, speed);
    gps::getDOP(hdop, vdop, pdop);
    gps::getSat(satFix, satCount);
    _getFormattedPosition(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));

    format::heading (hdg,   hdgBuffer,             sizeof(hdgBuffer));
    format::speed   (speed, imperial, speedBuffer, sizeof(speedBuffer));
    format::altitude(masl,  imperial, aslBuffer,   sizeof(aslBuffer));
    snprintf(
        gpsStatus, sizeof(gpsStatus), "FIX %s  SAT %02d  HDOP %.1f",
        satFix >= 3 ? "3D" : satFix == 2 ? "2D" : "--", satCount, hdop
    );

    title::updateDate     (tft, date);
    title::updateTime     (tft, time);
    datas::updateLatitude (tft, latitude);
    datas::updateLongitude(tft, longitude);
    datas::updateSpeed    (tft, speedBuffer);
    datas::updateHeading  (tft, hdgBuffer);
    datas::updateASL      (tft, aslBuffer);
    datas::updateUpdate   (tft, gpsTime);
    locator::updateLocator(tft, qth);

    locator::updateStatusBottom(tft, gpsStatus);
    updateMARK(tft);
}

void main::updateMARK(ST7796S::MSP4021 &tft) {
    const bool imperial = settings::getUnits() == settings::Units::IMPERIAL;

    if (!navigation::hasMark() ||
        (navigation::markState() == navigation::MarkState::READY_TO_SAVE && navigation::markTotalDistanceKm() <= 0.0)
    ) {
        locator::updateMarkLocator(tft, "---");
        locator::updateMarkBearing(tft, "---");
        locator::updateMarkDistance(tft, "---");
        locator::updateMarkTimer(tft, "-- : -- : --");
        return;
    }

    char locator[32];
    char distance[16];
    char bearing[16];
    char timer[16];

    format::distance       (navigation::markCurrentDistanceKm(), imperial, distance, sizeof(distance));
    format::bearing        (navigation::markCurrentBearingDeg(), bearing,            sizeof(bearing));
    format::durationCompact(navigation::markElapsedSeconds(),    timer,              sizeof(timer));
    navigation::getMarkStartLocator(locator, sizeof(locator));

    locator::updateMarkLocator(tft, locator);
    locator::updateMarkBearing(tft, bearing);
    locator::updateMarkDistance(tft, distance);
    locator::updateMarkTimer(tft, timer);
}

void main::draw(ST7796S::MSP4021 &tft) {
    tft.fillScreen(theme::BLACK);

    title::draw(tft);
    locator::draw(tft);
    datas::draw(tft);

    buttons::draw(tft);
}
