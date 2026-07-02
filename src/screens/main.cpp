/*
 * screens/main.cpp
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
#include "screens/mockup/buttons.h"
#include "services/battery.h"
#include "services/gps.h"
#include "services/navigation.h"
#include "services/settings.h"
#include "ui/settings/themes/defaults.h"

namespace screens::main {
    namespace {
        void formatDuration(uint32_t seconds, char* buffer, size_t size) {
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
    }

    void preload() {
        char battery[8] = {};
        screens::main::title::getBatteryLevel(battery, sizeof(battery));
        screens::main::title::setBattery(battery);

        preloadGPS();
        preloadSOTA();
        preloadMARK();
    }

    void preloadGPS() {
        double masl, hdg, speed, hdop, vdop, pdop, gpsLatitude, gpsLongitude;
        int satFix, satCount;

        char date[16];
        char time[16];
        char latitude[32];
        char longitude[32];
        char qth[32];
        char speedBuffer[16];
        char hdgBuffer[16];
        char aslBuffer[16];
        char uptime[16];
        char gpsStatus[32];

        char callsign[32];
        screens::main::title::getUptime(uptime, sizeof(uptime));

        services::gps::getDate(date, sizeof(date));
        services::gps::getTime(time, sizeof(time));
        services::gps::getDDM(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));
        services::gps::getPrecision(masl, hdg, speed);
        services::gps::getDOP(hdop, vdop, pdop);
        services::gps::getSat(satFix, satCount);
        services::gps::getPosition(gpsLatitude, gpsLongitude);

        services::navigation::setCurrentPosition(gpsLatitude, gpsLongitude);

        if (isnan(hdg)) { snprintf(hdgBuffer, sizeof(hdgBuffer), ""); }
        else { snprintf(hdgBuffer, sizeof(hdgBuffer), "%.0f° %s", hdg, services::gps::headingToCardinal(hdg)); }

        snprintf(
            speedBuffer, sizeof(speedBuffer), "%.1f km/h",
            speed
        );
        snprintf(
            aslBuffer, sizeof(aslBuffer), "%.0f m",
            masl
        );
        snprintf(
            gpsStatus, sizeof(gpsStatus), "FIX %s  SAT %02d  HDOP %.1f",
            satFix >= 3 ? "3D" : satFix == 2 ? "2D" : "--", satCount, hdop
        );

        if (!services::settings::getFullCallsign(callsign, sizeof(callsign)))
            { strcpy(callsign, "ERROR"); }
        screens::main::title::setCallsign   (callsign);
        screens::main::title::setDate       (date);
        screens::main::title::setTime       (uptime);
        //screens::main::title::setBattery  ("");

        screens::main::datas::setLatitude   (latitude);
        screens::main::datas::setLongitude  (longitude);
        screens::main::datas::setSpeed      (speedBuffer);
        screens::main::datas::setHeading    (hdgBuffer);
        screens::main::datas::setASL        (aslBuffer);
        screens::main::datas::setUpdate     (time);

        //screens::main::locator::setStatusTop  ("");
        screens::main::locator::setLocator      (qth);
        screens::main::locator::setStatusBottom (gpsStatus);
    }

    void preloadSOTA() {
        if (!services::navigation::hasSOTA()) {
            screens::main::locator::setSOTABearing("---");
            screens::main::locator::setSOTADistance("---");
            screens::main::locator::setSOTAPoints("---");
            screens::main::locator::setSOTAAltitude("---");
            screens::main::locator::setSOTACode("---");
            return;
        }

        char distance[16];
        char bearing[16];
        char points[16];
        char altitude[16];
        char code[24];

        services::navigation::formatDistance(services::navigation::sotaDistanceKm(), distance, sizeof(distance));
        services::navigation::formatBearing(services::navigation::sotaBearingDeg(), bearing, sizeof(bearing));
        services::navigation::getSOTACode(code, sizeof(code));

        snprintf(points, sizeof(points), "%d (+1)", services::navigation::getSOTAPoints());
        snprintf(altitude, sizeof(altitude), "%d m", services::navigation::getSOTAAltitude());

        screens::main::locator::setSOTABearing(bearing);
        screens::main::locator::setSOTADistance(distance);
        screens::main::locator::setSOTAPoints(points);
        screens::main::locator::setSOTAAltitude(altitude);
        screens::main::locator::setSOTACode(code);
    }

    void preloadMARK() {
        if (!services::navigation::hasMark()) {
            screens::main::locator::setMarkLocator("---");
            screens::main::locator::setMarkBearing("---");
            screens::main::locator::setMarkDistance("---");
            screens::main::locator::setMarkTimer("-- : -- : --");
            return;
        }

        char locator[32];
        char distance[16];
        char bearing[16];
        char timer[16];

        services::navigation::getMarkStartLocator(locator, sizeof(locator));
        services::navigation::formatDistance(services::navigation::markCurrentDistanceKm(), distance, sizeof(distance));
        services::navigation::formatBearing(services::navigation::markCurrentBearingDeg(), bearing, sizeof(bearing));
        formatDuration(services::navigation::markElapsedSeconds(), timer, sizeof(timer));

        screens::main::locator::setMarkLocator(locator);
        screens::main::locator::setMarkBearing(bearing);
        screens::main::locator::setMarkDistance(distance);
        screens::main::locator::setMarkTimer(timer);
    }

    void update(ST7796S::MSP4021 &tft, uint32_t &nextRefreshIn) {
        nextRefreshIn = 1000;

        char battery[8] = {};
        screens::main::title::getBatteryLevel(battery, sizeof(battery));
        screens::main::title::updateBattery(tft, battery);

        double masl, hdg, speed, hdop, vdop, pdop, gpsLatitude, gpsLongitude;
        int satFix, satCount;

        char date[16];
        char time[16];
        char latitude[32];
        char longitude[32];
        char qth[32];
        char speedBuffer[16];
        char hdgBuffer[16];
        char aslBuffer[16];
        char uptime[16];
        char gpsStatus[32];

        screens::main::title::getUptime(uptime, sizeof(uptime));

        services::gps::getDate(date, sizeof(date));
        services::gps::getTime(time, sizeof(time));
        services::gps::getDDM(latitude, sizeof(latitude), longitude, sizeof(longitude), qth, sizeof(qth));
        services::gps::getPrecision(masl, hdg, speed);
        services::gps::getDOP(hdop, vdop, pdop);
        services::gps::getSat(satFix, satCount);
        services::gps::getPosition(gpsLatitude, gpsLongitude);
        services::navigation::setCurrentPosition(gpsLatitude, gpsLongitude);

        if (isnan(hdg)) { snprintf(hdgBuffer, sizeof(hdgBuffer), ""); }
        else { snprintf(hdgBuffer, sizeof(hdgBuffer), "%.0f° %s", hdg, services::gps::headingToCardinal(hdg)); }

        snprintf(
            speedBuffer, sizeof(speedBuffer), "%.1f km/h",
            speed
        );
        snprintf(
            aslBuffer, sizeof(aslBuffer), "%.0f m",
            masl
        );
        snprintf(
            gpsStatus, sizeof(gpsStatus), "FIX %s  SAT %02d  HDOP %.1f",
            satFix >= 3 ? "3D" : satFix == 2 ? "2D" : "--", satCount, hdop
        );

        screens::main::title::updateDate     (tft, date);
        screens::main::title::updateTime     (tft, uptime);
        screens::main::datas::updateLatitude (tft, latitude);
        screens::main::datas::updateLongitude(tft, longitude);
        screens::main::datas::updateSpeed    (tft, speedBuffer);
        screens::main::datas::updateHeading  (tft, hdgBuffer);
        screens::main::datas::updateASL      (tft, aslBuffer);
        screens::main::datas::updateUpdate   (tft, time);
        screens::main::locator::updateLocator(tft, qth);

        screens::main::locator::updateStatusBottom(tft, gpsStatus);
        updateMARK(tft);
    }

    void updateMARK(ST7796S::MSP4021 &tft) {
        if (!services::navigation::hasMark() ||
        (services::navigation::markState() == services::navigation::MarkState::READY_TO_SAVE && services::navigation::markTotalDistanceKm() <= 0.0)) {
            screens::main::locator::updateMarkLocator(tft, "---");
            screens::main::locator::updateMarkBearing(tft, "---");
            screens::main::locator::updateMarkDistance(tft, "---");
            screens::main::locator::updateMarkTimer(tft, "-- : -- : --");
            return;
        }

        char locator[32];
        char distance[16];
        char bearing[16];
        char timer[16];

        services::navigation::getMarkStartLocator(locator, sizeof(locator));
        services::navigation::formatDistance(services::navigation::markCurrentDistanceKm(), distance, sizeof(distance));
        services::navigation::formatBearing(services::navigation::markCurrentBearingDeg(), bearing, sizeof(bearing));
        formatDuration(services::navigation::markElapsedSeconds(), timer, sizeof(timer));

        screens::main::locator::updateMarkLocator(tft, locator);
        screens::main::locator::updateMarkBearing(tft, bearing);
        screens::main::locator::updateMarkDistance(tft, distance);
        screens::main::locator::updateMarkTimer(tft, timer);
    }

    void draw(ST7796S::MSP4021 &tft) {
        tft.fillScreen(ui::settings::themes::defaults::BLACK);

        screens::main::title::draw(tft);
        screens::main::locator::draw(tft);
        screens::main::datas::draw(tft);

        screens::mockup::buttons::draw(tft);
    }
}