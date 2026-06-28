# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog.
Versioning follows Semantic Versioning.

---

## [1.1.0] - 2026-06-28

### Added
- Placeholder pages for:
  - General
  - Update
  - Storage
  - Battery

### Internal
- Added settings page architecture:
  - `screens/menu/general.*`
  - `screens/menu/updates.*`
  - `screens/menu/storage.*`
  - `screens/menu/battery.*`
- Renamed `update.*` page implementation to `updates.*` to avoid namespace conflicts
- Improved menu extensibility for future settings implementation

---

## [1.0.7] - 2026-06-27

### Added
- Settings menu categories:
  - General
  - Update
  - Storage
  - Battery
  - About
- About screen
- Project information display:
  - Name
  - Version
  - Author
  - License
  - GitHub repository
- Automatic reset to `General` when leaving the settings menu

### Changed
- Simplified settings navigation structure
- Navigation item count now uses `Item::COUNT`
- Added compile-time validation for settings navigation rows
- Improved menu touch handling and screen redraw behavior and logic
- Refactored About screen layout using reusable `drawLine(...)` helpers

---

## [1.0.6] - 2026-06-13

### Added

- Added battery monitoring service.
- Added battery voltage measurement support.
- Added battery percentage estimation.
- Added battery presence detection.
- Added battery display integration in the title bar.
- Added power sense input support.
- Added clean shutdown preparation before deep sleep.

### Changed

- Added battery configuration support through compile-time build flags.

---

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

---

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
