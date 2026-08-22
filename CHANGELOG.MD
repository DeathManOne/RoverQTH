# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog.
Versioning follows Semantic Versioning.

---

## [1.10.0] - 2026-08-22

### Added

- Stateful display service responsible for display initialization, screen ownership, navigation and touch routing
- Common screen interface for consistent drawing, updating and touch handling
- Dedicated object-oriented screen implementations for boot, main, menu and S/P OTA
- Unified OTA database, utility and service modules for SOTA summits and POTA parks
- Separate JSON reader and writer modules

### Changed

- Migrated the complete screen lifecycle from `core::screenManager` to `services::display`
- Converted the main application screens into stateful classes
- Renamed the SOTA search screen to the generic S/P OTA screen
- Reorganized SOTA and POTA databases under `database::ota`
- Reorganized SOTA and POTA utilities under `utilities::ota`
- Replaced the separate SOTA and POTA services with the unified `services::ota`
- Reorganized JSON utilities into dedicated reader and writer components
- Simplified display initialization by using the configured build parameters internally
- Improved menu field refresh and storage-capacity presentation
- Improved screen-state synchronization and bounds validation

### Fixed

- Fixed boot-screen field refresh relying on pointer comparison
- Fixed empty callsign and Wi-Fi SSID values not being propagated to the touchscreen keyboard
- Fixed possible invalid screen and button state indexing
- Fixed GNSS fields retaining stale placeholder characters after receiving valid values
- Preserved the boot-screen GPS search button during the screen architecture migration

### Removed

- Legacy `core::screenManager` and global screen-state modules
- Legacy `display` manager and screen wrapper modules
- Separate SOTA and POTA database, utility and service interfaces

### Internal

- Centralized display hardware ownership and screen transitions in `services::display`
- Reduced duplicated SOTA and POTA orchestration through a shared OTA snapshot model
- Simplified the application entry point and screen dependencies
- Reduced the codebase by consolidating superseded modules

---

## [1.9.0] - 2026-08-19

### Added

- Complete POTA database update workflow
- Secure HTTPS download of the official POTA park list
- Remote SOTA and POTA database checks using ETag and content length
- CSV validation and conversion to compact binary databases
- Atomic POTA database installation with backup and rollback
- Versioned and validated SOTA, POTA and QTH file formats
- Dedicated S/P OTA search page
- Asynchronous SOTA and POTA searches by proximity
- Prefix-based SOTA and POTA code searches
- Touchscreen keyboard support for code-prefix entry
- Flexible code normalization:
  - Case-insensitive input
  - Optional spaces, slashes and hyphens
  - Compact code entry such as `FLNO096` or `FR10026`
- Result sorting by distance and code
- SOTA result sorting by points
- Paginated search results
- Search cancellation through the `STOP` button
- SOTA/POTA result selection and confirmation
- Persistent SOTA/POTA selection in NVS
- `CLEAR SELECTION` support
- Selected `SOTA: code` or `POTA: code` display on the main screen
- ESP and CPU reset-reason logging at startup
- Detailed SOTA and POTA update diagnostics

### Changed

- Unified the SOTA and POTA database schemas and search interfaces
- Updated the SOTA binary database format to version `3`
- Extended `services::update` to orchestrate firmware, SOTA and POTA updates
- Improved update progress reporting across check, download, installation and verification
- Limited search results to the nearest 64 matching records
- Preserved displayed results until a new search is explicitly started
- Disabled unavailable S/P OTA controls when the SD card or required database is absent
- Removed automatic nearest-SOTA/POTA searching at startup
- Improved SD-card access by keeping large files open during sequential operations
- Updated SDCard integration to use the external PlatformIO dependency
- Improved startup, service and update logging
- Harmonized SOTA and POTA validation, cancellation and error handling

### Fixed

- Fixed SOTA and POTA update tasks remaining blocked after a failed operation
- Fixed incomplete temporary-file cleanup after database operations
- Fixed database installation failures caused by repeated SD-card file opening
- Fixed inconsistent battery readings during intensive SD-card operations
- Fixed stale characters remaining in GNSS fields on the main screen
- Fixed navigation being triggered by touchscreen keyboard input
- Fixed periodic screen refresh drawing over the touchscreen keyboard
- Fixed search results disappearing after editing a code prefix
- Fixed repeated or unintended search cancellation
- Fixed POTA download and database validation failures
- Fixed invalid or incomplete source records being accepted during conversion

### Internal

- Added `database::pota`, `utilities::pota` and `services::pota`
- Added shared OTA code normalization and prefix-matching utilities
- Added asynchronous prefix-search support to SOTA and POTA services
- Added common bounded search-result storage and nearest-result retention
- Added coordinated access between firmware updates, database updates and searches
- Strengthened binary headers, record validation and file-size verification
- Improved symmetry between the SOTA and POTA implementations

---

## [1.8.0] - 2026-08-09

### Added

- Complete SOTA database update workflow
- Secure HTTPS download of the official SOTA summit list
- Remote SOTA database version checking
- SOTA download and installation progress display
- CSV validation and conversion to a compact binary database
- Atomic SOTA database installation with backup and rollback
- Installed SOTA database version and record-count tracking
- Asynchronous nearest-summit search
- Nearest SOTA summit display on the main screen:
  - Summit code
  - Bearing
  - Distance
  - Points and bonus points
  - Altitude
- `Searching` status while the initial nearest-summit search is running
- Cached SOTA result with automatic refresh after one kilometer of movement
- Automatic cache invalidation after a database update
- Dedicated SOTA event and error logging
- Binary chunk reading and persistent file-writing support in the storage service

### Changed

- Extended `services::update` to orchestrate both firmware and SOTA database updates
- Split firmware and SOTA update states into dedicated snapshots
- Added progress reporting during firmware and SOTA checks
- Prevented firmware updates, SOTA updates and SOTA searches from running concurrently
- Moved nearest-summit searches to a background FreeRTOS task
- Updated the SDCard dependency from version `2.0.0` to `2.1.0`
- Improved the service initialization order after SD-card startup

### Fixed

- Fixed user-interface blocking while searching the complete SOTA database
- Fixed menu navigation blocking when a SOTA check was requested during a search
- Fixed repeated search-task creation when no valid SOTA database is installed
- Fixed repeated file opening during large SOTA downloads and binary conversion
- Fixed incomplete SOTA download and SD-write validation
- Fixed temporary-file cleanup after completed or failed SOTA operations
- Preserved the previously installed database when candidate installation fails

### Internal

- Added `utilities::sota` for stateless SOTA parsing and validation
- Added `database::sota` for binary database ownership and queries
- Added `services::sota` for asynchronous search state and result caching
- Added binary format `2` with fixed-size summit records
- Added coordinated access between SOTA searches, storage and update tasks

---

## [1.7.1] - 2026-08-07

### Added

- Embedded TLS root certificates for secure GitHub OTA connections
- Automated certificate-expiration and TLS trust-chain monitoring
- Automated repository license-integrity verification
- GitHub ownership rules for sensitive legal, security and workflow files

### Changed

- Moved centralized date and time management into `services::dtc`
- Improved full callsign handling and display
- Hardened OTA manifest parsing and firmware verification
- Improved GPS initialization and acquisition reliability
- Strengthened NVS, settings and storage error handling
- Added synchronized access to settings and SD-card storage operations

### Fixed

- Fixed OTA TLS failures caused by malformed embedded PEM certificates
- Fixed GPS initialization failures caused by serial-port interference
- Fixed recursive locking deadlocks in settings and storage services

---

## [1.7.0] - 2026-08-04

### Added

- Display shutdown support
- Centralized display power-off through `display::shutdown()`

### Changed

- Display shutdown now delegates to the underlying ST7796S display driver
- Display is cleared before hardware shutdown
- Improved robustness across all `services::*` and `utilities::*` modules

### Internal

- Reviewed and hardened all `services::*` and `utilities::*` components
- Improved defensive programming throughout the project
- Improved error handling across internal modules
- Improved overall code consistency and maintainability
- Prepared the display layer for future power management features

---

## [1.6.2] - 2026-07-25

### Added

- Dedicated `utilities` layer for reusable project components
- Version comparison utilities
- Formatting utilities
- Distance calculation utilities
- Unit conversion utilities
- SHA-256 utilities
- Coordinate utilities
- Maidenhead locator utilities

### Changed

- Replaced the header uptime with a GNSS-synchronized UTC clock
- Refactored clock management into the new `utilities::clock` module
- Refactored coordinate conversions into `utilities::coordinates`
- Refactored Maidenhead locator computations into `utilities::locator`
- Refactored unit conversions into `utilities::units`
- Refactored distance and bearing calculations into `utilities::distance`
- Refactored shared formatting helpers into `utilities::format`
- Refactored SHA-256 handling into `utilities::hash`
- Refactored version parsing and comparison into `utilities::version`
- Centralized and secured UTC time synchronization
- Header time now continues running between GNSS updates
- OTA firmware verification and version handling now use the new `utilities` layer

### Internal

- Simplified shared logic across services, screens and utilities
- Reduced duplicated helper functions throughout the project
- Improved separation of responsibilities between utilities and application services
- Improved overall maintainability and code organization

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
