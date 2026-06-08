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
* Touchscreen calibration
* Persistent calibration storage using NVS
* Optimized landscape interface
* Embedded fonts and graphical resources

### System Features

* Boot status screen
* Screen rotation protection
* Persistent configuration storage
* ESP32-based architecture

---

## Hardware Requirements

### Supported Controller

* Arduino Nano ESP32

### Supported GNSS Module

* u-blox GNSS receiver

### Supported Display

* ST7796S 480×320 touchscreen display (clone)
* MSP4020 / MSP4021 compatible hardware

---

## Tested Hardware

The following hardware was used during RoverQTH development and testing.

Using the same components is recommended to ensure identical behavior and compatibility.

### Controller

- Arduino Nano ESP32
- Amazon: <link>

### GNSS Receiver

- u-blox GNSS module
- Amazon: <link>

### Display

- MSP4021 4-inch 480×320 ST7796S touchscreen display
- Amazon: <link>

### Storage

- SDCard Reader
- Amazon: <link>

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

### SDCard

Version:

* v1.5.1

Repository:

* https://github.com/DeathManOne/SDCard

Status:

* Integrated but currently disabled in version 1.0.0.
* Planned for future releases.

---

## Installation

### PlatformIO

Add the library to your project and install the required dependencies.

Example:

```ini
lib_deps =
    https://github.com/DeathManOne/RoverQTH#1.0.0
```

---

## Configuration

The project uses compile-time configuration through PlatformIO build flags.

Example configuration:

```ini
-D GPS_RX=2
-D GPS_TX=3
-D GPS_BAUD=38400

-D TFT_TOUCH_CS=8
-D TFT_SCREEN_CS=9
-D TFT_SCREEN_DC=10
-D TFT_SCREEN_RST=-1

-D TFT_MOSI=11
-D TFT_MISO=12
-D TFT_CLK=13

-D TFT_WIDTH=480
-D TFT_HEIGHT=320
-D TFT_ROTATION=3
```

---

## Usage

Initialize the RoverQTH library from your application and call the main processing loop continuously.

```cpp
#include <RoverQTH.h>

void setup() {
    RoverQTH.begin();
}

void loop() {
    RoverQTH.loop();
}
```

---

## Current Status

### Version 1.0.0

Implemented:

* GNSS positioning
* Maidenhead locator display
* GPS information display
* Touchscreen interface
* Touch calibration
* Boot status screen
* NVS configuration storage

Not enabled in version 1.0.0:

* WiFi connectivity
* OTA firmware updates
* SD card features (integration present but currently disabled)
* MARK navigation
* SOTA navigation
* GSM emergency features

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
