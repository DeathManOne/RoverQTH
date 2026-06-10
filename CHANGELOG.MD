# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog.
Versioning follows Semantic Versioning.

## [1.0.5] - 2026-06-10

### Added

- Added SD card initialization support.
- Added SD reader hardware reference and pin configuration.

### Changed

- Improved MARK QTH refresh behavior.
- MARK QTH columns are now redrawn independently instead of redrawing the full MARK block on each update.

### Notes

- SD card support is compiled and initialized.
- SD card storage features are planned for a future release.

## [1.0.0] - 2026-06-08

### Added

- Initial public release.
- ESP32 support (Arduino Nano ESP32 target).
- u-blox GNSS integration.
- Maidenhead locator computation and display.
- GPS data display:
  - Latitude
  - Longitude
  - Speed
  - Heading
  - Altitude (ASL)
  - Fix status
  - Satellite count
  - HDOP
- Touchscreen user interface.
- Touch calibration with persistent NVS storage.
- Screen rotation protection.
- Boot screen with module status display.
- GPS acquisition progress indicator.
- Automatic GPS acquisition recovery on timeout.
- Embedded fonts and logo resources.
