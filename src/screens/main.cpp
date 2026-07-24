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

namespace main       = screens::main;
namespace datas      = screens::main::datas;
namespace locator    = screens::main::locator;
namespace title      = screens::main::title;
namespace battery    = services::battery;
namespace gps        = services::gps;
namespace navigation = services::navigation;
namespace settings   = services::settings;
namespace buttons    = ui::mockup::buttons;
namespace theme      = ui::settings::themes::defaults;
namespace uClock     = utilities::clock;

namespace {
    void _formatSpeed   (double kmh,       char* buffer, size_t size);
    void _formatAltitude(double meters,    char* buffer, size_t size);
    void _formatDuration(uint32_t seconds, char* buffer, size_t size);

    void _formatDuration(uint32_t seconds, char* buffer, size_t size) {
        const uint32_t hours = seconds / 3600;
        seconds %= 3600;

        const uint32_t minutes = seconds / 60;
        seconds %= 60;

        if (hours > 0) {
            snprintf(buffer, size, "%luh %02lum", hours, minutes);
            return;
        }
        snprintf(buffer, size, "%lum %02lus", minutes, seconds);
    }

    void _formatSpeed(double kmh, char* buffer, size_t size) {
        if (settings::getUnits() == settings::Units::IMPERIAL) {
            std::snprintf(buffer, size, "%.1f mph", kmh * 0.621371);
            return;
        }
        std::snprintf(buffer, size, "%.1f km/h", kmh);
    }

    void _formatAltitude(double meters, char* buffer, size_t size) {
        if (settings::getUnits() == settings::Units::IMPERIAL) {
            std::snprintf(buffer, size, "%.0f ft", meters * 3.28084);
            return;
        }
        std::snprintf(buffer, size, "%.0f m", meters);
    }

    void _getFormattedPosition(char* latitude, size_t latitudeSize, char* longitude, size_t longitudeSize, char* qth, size_t qthSize) {
        switch (settings::getCoordinateFormat()) {
            case settings::CoordinateFormat::DD:
                gps::getDD(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize);
                break;
            case settings::CoordinateFormat::DMS:
                gps::getDMS(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize);
                break;
            case settings::CoordinateFormat::DDM:
            default:
                gps::getDDM(latitude, latitudeSize, longitude, longitudeSize, qth, qthSize);
                break;
        }
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

    uClock::getDate(date, sizeof(date));
    uClock::getTime(time, sizeof(time));
    gps::getTime(gpsTime, sizeof(gpsTime));
    gps::getPrecision(masl, hdg, speed);
    gps::getDOP(hdop, vdop, pdop);
    gps::getSat(satFix, satCount);
    _getFormattedPosition(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));

    if (isnan(hdg)) { snprintf(hdgBuffer, sizeof(hdgBuffer), ""); }
    else { snprintf(hdgBuffer, sizeof(hdgBuffer), "%.0f° %s", hdg, gps::headingToCardinal(hdg)); }

    _formatSpeed(speed, speedBuffer, sizeof(speedBuffer));
    _formatAltitude(masl, aslBuffer, sizeof(aslBuffer));
    snprintf(
        gpsStatus, sizeof(gpsStatus), "FIX %s  SAT %02d  HDOP %.1f",
        satFix >= 3 ? "3D" : satFix == 2 ? "2D" : "--", satCount, hdop
    );

    if (!settings::getFullCallsign(callsign, sizeof(callsign)))
        { strcpy(callsign, "ERROR"); }
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

    navigation::formatDistance(navigation::sotaDistanceKm(), distance, sizeof(distance));
    navigation::formatBearing(navigation::sotaBearingDeg(), bearing, sizeof(bearing));
    navigation::getSOTACode(code, sizeof(code));

    snprintf(points, sizeof(points), "%d (+1)", navigation::getSOTAPoints());
    _formatAltitude(navigation::getSOTAAltitude(), altitude, sizeof(altitude));

    locator::setSOTABearing(bearing);
    locator::setSOTADistance(distance);
    locator::setSOTAPoints(points);
    locator::setSOTAAltitude(altitude);
    locator::setSOTACode(code);
}

void main::preloadMARK() {
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

    navigation::getMarkStartLocator(locator, sizeof(locator));
    navigation::formatDistance(navigation::markCurrentDistanceKm(), distance, sizeof(distance));
    navigation::formatBearing(navigation::markCurrentBearingDeg(), bearing, sizeof(bearing));
    _formatDuration(navigation::markElapsedSeconds(), timer, sizeof(timer));

    locator::setMarkLocator(locator);
    locator::setMarkBearing(bearing);
    locator::setMarkDistance(distance);
    locator::setMarkTimer(timer);
}

void main::update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) {
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

    uClock::getDate(date, sizeof(date));
    uClock::getTime(time, sizeof(time));
    gps::getTime(gpsTime, sizeof(gpsTime));
    gps::getPrecision(masl, hdg, speed);
    gps::getDOP(hdop, vdop, pdop);
    gps::getSat(satFix, satCount);
    _getFormattedPosition(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));

    if (isnan(hdg)) { snprintf(hdgBuffer, sizeof(hdgBuffer), ""); }
    else { snprintf(hdgBuffer, sizeof(hdgBuffer), "%.0f° %s", hdg, gps::headingToCardinal(hdg)); }

    _formatSpeed(speed, speedBuffer, sizeof(speedBuffer));
    _formatAltitude(masl, aslBuffer, sizeof(aslBuffer));
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
    if (!navigation::hasMark() ||
    (navigation::markState() == navigation::MarkState::READY_TO_SAVE && navigation::markTotalDistanceKm() <= 0.0)) {
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

    navigation::getMarkStartLocator(locator, sizeof(locator));
    navigation::formatDistance(navigation::markCurrentDistanceKm(), distance, sizeof(distance));
    navigation::formatBearing(navigation::markCurrentBearingDeg(), bearing, sizeof(bearing));
    _formatDuration(navigation::markElapsedSeconds(), timer, sizeof(timer));

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
