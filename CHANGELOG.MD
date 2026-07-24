# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog.
Versioning follows Semantic Versioning.

---

## [1.6.1] - 2026-07-24

### Added

- Monotonic UTC clock synchronization

### Changed

- Replaced the header uptime with a GNSS-synchronized UTC clock
- Refactored clock management into the new `utilities::clock` module
- Centralized and secured UTC time synchronization
- Header time now continues running between GNSS updates

### Internal

- Introduced centralized `utilities::clock` service
- UTC date/time formatting helpers
- UTC epoch conversion utilities
- Moved clock-related helpers from services into `utilities::clock`
- Simplified UTC time handling across the project
- Improved clock synchronization reliability and consistency

---

## [1.6.0] - 2026-07-19

### Added
- Operational WiFi connectivity for online services
- Complete OTA firmware update system
- Remote firmware manifest retrieval
- Firmware version checking from the Updates settings page
- Automatic comparison between installed and available firmware versions
- Interactive firmware update actions:
  - Check for updates
  - Download and install available firmware
- Real-time OTA status display:
  - Checking
  - Up to date
  - Update available
  - Starting
  - Downloading
  - Verifying
  - Installing
  - Restarting
  - Error
- OTA download progress display
- SHA-256 firmware integrity verification
- Firmware size validation before OTA installation
- Automatic restart after a successful firmware update
- Detailed OTA error reporting for:
  - WiFi connection failures
  - Unavailable manifest
  - Invalid manifest
  - Unavailable firmware
  - Invalid firmware size
  - Interrupted downloads
  - SHA-256 mismatch
  - OTA partition errors
  - Firmware installation failures

### Changed
- Reworked the Updates settings page to display:
  - Installed firmware version
  - Available firmware version
  - Firmware update status
  - OTA download progress
- Firmware update actions are now dynamically enabled according to the OTA state
- Firmware checks and downloads now run asynchronously without blocking the main application
- Automatic power shutdown is now postponed while an OTA operation is active
- Firmware and manifest download URLs are now configured through dedicated PlatformIO build flags

### Fixed
- Prevented multiple OTA tasks from being launched simultaneously
- Prevented firmware installation when no update is available
- Prevented incomplete or corrupted firmware downloads from being installed
- Prevented automatic deep sleep from interrupting an active OTA update

### Internal
- Added dedicated `services::update` firmware update service
- Added FreeRTOS background tasks for update checking and firmware installation
- Added centralized OTA status, progress and error management
- Added HTTPS streaming using a fixed-size RAM buffer
- Added firmware manifest parsing and validation
- Added OTA partition streaming through the ESP32 `Update` API
- Added configurable build flags:
  - `DL_FIRMWARE`
  - `DL_FIRMWARE_MANIFEST`
- Added dedicated OTA event and error logging
- Prepared the Updates page architecture for future SOTA and POTA database updates
- Added asynchronous OTA workflow based on FreeRTOS tasks

---

## [1.5.0] - 2026-07-14

### Added
- Complete SD card storage architecture
- Persistent QTH session tracking and recording in JSONL format
- System event logging
- Error logging
- Automatic log timestamping:
  - Boot-relative timestamps before GNSS synchronization
  - ISO 8601 UTC timestamps after GNSS synchronization
- Automatic storage directory creation
- Temporary in-memory log buffering before SD initialization
- Automatic flushing of buffered logs after SD initialization
- Complete QTH record structure:
  - Start and stop positions
  - UTC timestamps
  - Maidenhead locators
  - Altitude
  - Duration
  - Air-line distance

### Changed
- Refactored storage from a generic file API to a record-oriented API
- QTH recording now automatically discards recordings shorter than the configured minimum duration
- Refactored SD storage into a dedicated storage service
- Simplified storage API using high-level record functions
- Storage service now automatically manages:
  - SD availability
  - Log formatting
  - Timestamp generation
  - Automatic log buffering
- Reworked boot sequence to support persistent logging
- Improved GNSS clock integration with system logging

### Removed
- Generic public file manipulation API from the storage service

### Internal
- Added automatic `/RoverQTH` directory tree creation
- Added dedicated storage layout:
  - `/database`
  - `/export`
  - `/logs`
  - `/qth`
  - `/tmp`
- Added centralized record formatting inside the storage service
- Added automatic SD failure detection during writes
- Improved separation between storage implementation and application logic

---

## [1.4.0] - 2026-07-10

### Added
- Added Network settings page
- Added shared menu page helpers:
  - Field management
  - Layout helpers
  - Drawing helpers
  - Touch helpers

### Changed
- Refactored all settings pages to use the common `Page` architecture:
  - General
  - Display
  - Updates
  - Storage
  - Battery
  - Network
  - About
- Simplified menu routing using polymorphism
- Centralized common menu rendering and touch handling
- Reduced duplicated code across all settings pages
- Improved maintainability and extensibility of the settings system

### Internal
- Added common `Page` base class for all settings screens
- Reworked settings menu architecture
- Centralized page state management
- Simplified menu page integration

---

## [1.3.0] - 2026-07-09

### Added
- Added configurable battery settings
- Added battery capacity, minimal, nominal and maximal voltage settings
- Added Ratio High / Ratio Low battery voltage divider configuration
- Added GNSS-based clock service preparation

### Changed
- Battery percentage and warning thresholds now use configurable voltage values
- Battery voltage calculation now uses configurable ratio settings
- Battery settings page is now functional

---

## [1.2.1] - 2026-07-05

### Fixed
- Updated library.json version to 1.2.1
- Updated PROJECT_VERSION build flag to 1.2.1
- Synchronized project versioning across all files

---

## [1.2.0] - 2026-07-05

### Added
- Metric/Imperial unit system support
- Independent touch calibration profiles for NORMAL and REVERSED rotations
- Screen rotation management from General settings
- Touch recalibration from General settings

### Changed
- Moved touch calibration loading from services::touch to display::TLoad()
- Refactored display initialization and touch calibration workflow
- Automatic touch calibration reload after screen rotation
- Automatic touch calibration reload after recalibration
- Distance, altitude and speed formatting now respect selected units
- General settings menu fully updated

### Removed
- Removed services::touch layer

### Fixed
- Fixed touch calibration persistence across screen rotations
- Fixed field redraw after rotation and recalibration
- Fixed imperial distance formatting for short distances
- Fixed CWE-562 warning caused by callsignField.value referencing a local buffer

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
