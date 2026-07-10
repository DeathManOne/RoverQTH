# RoverQTH

**Track your QSO - anywhere**

RoverQTH is an autonomous GNSS assistant designed for amateur radio rover operations. Built around the ESP32 platform, it provides a dedicated touchscreen interface displaying real-time GPS information and Maidenhead locator data, allowing operators to quickly determine their current operating position in the field.

The project is designed to run on compact portable hardware using a u-blox GNSS receiver and a 4-inch ST7796S touchscreen display (clone).

---

## Features

### GNSS Positioning

* u-blox GNSS receiver support
* Automatic GPS acquisition during startup
* GPS acquisition progress indicator
* Automatic acquisition recovery on timeout
* Real-time position updates

### Maidenhead Locator

* Maidenhead locator computation
* Large locator display optimized for field use
* Automatic locator updates from GNSS position

### GPS Information Display

* Latitude
* Longitude
* Speed
* Heading
* Altitude (ASL)
* Fix status
* Satellite count
* HDOP precision indicator

### Touchscreen Interface

* 4-inch ST7796S display support (clone)
* Persistent touchscreen calibration
* Independent NORMAL and REVERSED calibration profiles
* Runtime screen rotation support
* On-device recalibration from the Settings menu
* Optimized landscape interface
* Embedded fonts and graphical resources
* On-device display configuration

### System Features

* Boot status screen
* Settings menu
* Modular settings page framework
* Network configuration page (WiFi / Bluetooth preparation)
* Persistent configuration storage
* SD card initialization
* Battery monitoring
* Battery profile configuration
* GNSS-based clock service preparation

### Power Management

* Battery voltage monitoring
* Battery percentage estimation
* Battery presence detection
* Configurable battery capacity
* Configurable minimal, nominal and maximal battery voltage
* Configurable voltage divider ratio (High / Low)
* Dynamic low and critical battery thresholds

---

## Hardware Requirements

### Supported Controller

* Arduino Nano ESP32

### Supported GNSS Module

* u-blox GNSS receiver

### Supported Display

* ST7796S 480×320 touchscreen display (clone)
* MSP4020 / MSP4021 compatible hardware

### Supported SD Reader

* SPI SD card reader

### Supported Battery

* 1S LiPo battery (3.7 V nominal, 4.2 V max)

### Battery Monitoring

* Analog battery monitoring input
* External voltage divider required

---

## Tested Hardware

The following hardware was used during RoverQTH development and testing.

Using the same components is recommended to ensure identical behavior and compatibility.

### Controller

- Arduino Nano ESP32
- Kubii: [Arduino Nano ESP32](https://www.kubii.com/fr/cartes-micro-controleurs/4174-arduino-nano-esp32-avec-header-3272496324718.html)

### GNSS Receiver

- u-blox GNSS module
- Amazon: [GPS-GPSV3 NEO-M8N](https://www.amazon.fr/dp/B0F98669J9)

### Display

- MSP4021 4-inch 480×320 ST7796S touchscreen display
- Amazon: [4" LCD TFT Touch Display Binghe](https://www.amazon.fr/dp/B0CZRV3Q8T)

### SD Reader
- SPI Memory card reader
- Amazon: [Memory Card Module Shield](https://www.amazon.fr/dp/B077MB17JB)

### Battery

- 1S LiPo 3.7V 3000mAh
- Kubii: [Batterie 3000mAh Li-Po](https://www.kubii.com/fr/alimentations-protections/4913-batterie-3000mah-li-po-3272496324541.html)

---

## Dependencies

### GNSS

SparkFun u-blox GNSS Arduino Library

Version:

* v2.2.28

Repository:

* https://github.com/sparkfun/SparkFun_u-blox_GNSS_Arduino_Library

### Display

ST7796 MSP4020/MSP4021 Display Library

Version:

* v1.3.8

Repository:

* https://github.com/DeathManOne/Screen-4inch-320x480-ST7796---MSP4020-and-MSP4021

### SD Card

SDCard Library

Version:

* v2.0.0

Repository:

* https://github.com/DeathManOne/SDCard.git#2.0.0

---

## Installation

### PlatformIO

Add the library to your project and install the required dependencies.

Example:

```ini
lib_deps =
    https://github.com/DeathManOne/RoverQTH
```

---

## Configuration

The project uses compile-time configuration through PlatformIO build flags.

Example configuration:

```ini
-D GPS_RX=2
-D GPS_TX=3
-D GPS_BAUD=38400

-D SD_CS=4
-D SD_MOSI=5
-D SD_MISO=6
-D SD_CLK=7

-D TFT_TOUCH_CS=8
-D TFT_SCREEN_CS=9
-D TFT_SCREEN_DC=10
-D TFT_SCREEN_RST=-1

-D TFT_MOSI=11
-D TFT_MISO=12
-D TFT_CLK=13

-D TFT_WIDTH=480
-D TFT_HEIGHT=320

-D BATT_PIN=17
```

---

## Usage

Initialize the RoverQTH library from your application and call the main processing loop continuously.

```cpp
#include <RoverQTH.h>

void setup() {
    RoverQTH::setup();
}

void loop() {
    RoverQTH::loop();
}
```

---

## Current Development Status

Implemented:

* GNSS positioning
* Maidenhead locator display
* GPS information display
* Touchscreen interface
* Persistent touchscreen calibration
* Dual calibration profiles (NORMAL / REVERSED)
* Runtime screen rotation support
* Callsign customization
* Callsign suffix support
* Metric / Imperial units support
* Boot status screen
* NVS configuration storage
* SD card initialization
* Battery voltage monitoring
* Battery percentage estimation
* Battery presence detection
* Settings menu
* Configurable battery profile
* Battery voltage divider configuration
* Dynamic battery low/critical thresholds
* GNSS-based clock service preparation
* Network settings page

Not yet implemented:

* WiFi communication
* OTA firmware updates
* Full MARK navigation/storage workflow
* SOTA navigation
* GSM emergency features
* Storage actions (export, clear logs)
* GNSS UTC time display

---

## License

This project is licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later).

See the LICENSE file for complete license information.

---

## Author

DeathManOne

GitHub:
https://github.com/DeathManOne

Project:
https://github.com/DeathManOne/RoverQTH
