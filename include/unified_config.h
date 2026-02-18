#ifndef UNIFIED_CONFIG_H
#define UNIFIED_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "sdkconfig.h"
#include "config_screen.h"
#include "config_advanced.h"
#include "config_wifi.h"
#include "config_fw_update.h"
#include "config_admin.h"
#include "config_gps.h"
#include "config_ubx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTC Memory Regions (ESP32)
 *
 * ESP32 has two separate RTC memory regions that survive deep sleep:
 *
 * **RTC_SLOW_MEM (8KB @ 0x50000000):**
 * - Slower access (~200 cycles/access)
 * - Default location for RTC_DATA_ATTR
 * - Better for infrequently accessed config
 * - Use: `RTC_DATA_ATTR` or `RTC_SLOW_MEM_ATTR`
 *
 * **RTC_FAST_MEM (8KB @ 0x3FF80000):**
 * - Faster access (~50 cycles/access)
 * - Useful for frequently accessed data
 * - Use: `RTC_FAST_MEM_ATTR`
 *
 * **Unified Config Architecture (VERSION-AWARE MODULE STORAGE):**
 *
 * Each submodule is stored independently in persistent storage (NVS by default)
 * with version tracking:
 * - GPS config: Storage key "gps_cfg" with version-aware checksum
 * - UBX config: Storage key "ubx_cfg" with version-aware checksum
 * - Screen config: Storage key "screen_cfg" with version-aware checksum
 * - Main config: Storage key "main_cfg" with version-aware checksum
 * - FW update config: Storage key "fw_cfg" with version-aware checksum
 * - Admin config: Storage key "admin_cfg" with version-aware checksum
 * - Advanced config: Storage key "adv_cfg" with version-aware checksum
 *
 * **Storage abstraction layer (config_storage.c):**
 * - Separates RTC memory management from persistent storage
 * - Supports pluggable backends (NVS default, files/SD possible)
 * - Handles checksums, blob wrapping, and storage I/O
 *
 * **Version-Aware Checksum:**
 * - Checksum calculated on module data EXCLUDING version field
 * - Version field is first 2 bytes of each module struct
 * - Enables migration when module structure changes
 *
 * **Benefits of version-aware storage:**
 * ✓ Module structure changes don't cause corruption
 * ✓ Each module has independent version tracking
 * ✓ Migration preserves valid data
 * ✓ Resilient to firmware updates
 *
 * **RTC Memory usage:**
 * - Unified struct in RTC_SLOW_MEM (~150-200 bytes)
 * - Persistent storage (NVS/files) is primary persistent storage
 * - RTC is backup for deep sleep resilience
 *
 * **Alignment strategy:**
 * - packed + aligned(4) for 32-bit access efficiency
 * - Minimizes NVS storage (packed)
 * - Allows natural float access (aligned(4))
 */

// Forward declaration - full definition in context.h
// This is runtime session state, not config
typedef struct context_rtc_s context_rtc_t;

// Current unified RTC structure version - increment only when RTC struct layout changes
#define UNIFIED_CONFIG_VERSION 1

/**
 * @brief Unified RTC configuration structure (for RTC memory only)
 *
 * This structure consolidates all configuration data that needs to survive
 * deep sleep into RTC memory for fast access during wake-up.
 *
 * **IMPORTANT:** Each submodule is stored SEPARATELY in persistent storage
 * (NVS by default) with version-aware checksums. This structure is for RTC
 * memory convenience; persistent storage is managed by config_storage layer.
 *
 * When modifying a module:
 * 1. Increment the module's version field (first 2 bytes of module struct)
 * 2. Implement migration function in the module's config_manager_*.c file
 * 3. Only increment UNIFIED_CONFIG_VERSION if the RTC struct layout changes
 */
typedef struct unified_rtc_config_s {
    // Version and size for migration support (DO NOT MOVE OR REMOVE)
    uint16_t version;  // Unified RTC structure version
    uint16_t size;     // Size of this structure

    // GPS configuration (version field is first 2 bytes of cfg_gps_t)
    struct cfg_gps_s gps;
    // UBX configuration (version field is first 2 bytes of cfg_ubx_t)
    cfg_ubx_t ubx;
    // Screen configuration (version field is first 2 bytes of cfg_screen_t)
    cfg_screen_t screen;
    // Main configuration (version field is first 2 bytes of cfg_main_t)
    cfg_main_t main;
    // Firmware update configuration (version field is first 2 bytes of cfg_fw_update_t)
    cfg_fw_update_t fw_update;
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
    // Admin configuration (version field is first 2 bytes of cfg_admin_t)
    cfg_admin_t admin;
#endif
    // Advanced configuration (version field is first 2 bytes of cfg_advanced_t)
    cfg_advanced_t advanced;
    // Integrity check (must be last)
    uint32_t checksum;
} unified_rtc_config_t;

/**
 * @brief Default configuration values
 */

#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
#define CFG_ADMIN() .admin = CFG_ADMIN_DEFAULTS(),
#else
#define CFG_ADMIN()
#endif

#define UNIFIED_RTC_CONFIG_DEFAULTS() { \
    .version = UNIFIED_CONFIG_VERSION, \
    .size = sizeof(unified_rtc_config_t), \
    .gps = CFG_GPS_DEFAULTS(), \
    .ubx = CFG_UBX_DEFAULTS(), \
    .screen = CFG_SCREEN_DEFAULTS(), \
    .main = CFG_MAIN_DEFAULTS(), \
    .fw_update = CFG_FW_UPDATE_DEFAULTS(), \
    CFG_ADMIN() \
    .advanced = CFG_ADVANCED_DEFAULTS(), \
    .checksum = 0 \
}

/**
 * @brief Global unified RTC configuration instance
 */
extern unified_rtc_config_t g_rtc_config;

/**
 * @brief Initialize unified RTC configuration
 *
 * Loads configuration from NVS if available, otherwise uses defaults.
 * Validates checksum and repairs if corrupted.
 */
void unified_config_init(void);

/**
 * @brief Save unified configuration to NVS
 *
 * Performs batch write of all configuration data to NVS.
 * Updates checksum before saving.
 */
void unified_config_save(void);

/**
 * @brief Save specific submodule to NVS
 *
 * @param module Submodule group ID (SCFG_GROUP_*)
 */
esp_err_t unified_config_save_by_submodule(int module);

/**
 * @brief Flush configuration changes to NVS
 *
 * Called periodically to ensure configuration changes are persisted.
 * Optimized to only write when changes are detected.
 */
void unified_config_flush_to_nvs(void);

/**
 * @brief Validate configuration integrity
 *
 * @return true if configuration is valid, false if corrupted
 */
bool unified_config_validate(void);

/**
 * @brief Repair corrupted configuration
 *
 * Restores configuration from NVS or uses defaults if NVS is also corrupted.
 */
void unified_config_repair(void);

/**
 * @brief Reset configuration to factory defaults
 */
void unified_config_reset_to_defaults(void);

/**
 * @brief Erase all configuration from persistent storage
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t unified_config_erase_all(void);

/**
 * @brief Load module with version migration support
 *
 * Delegates to config_storage layer for persistence operations.
 *
 * @param nvs_key Storage key for this module (e.g., "gps_cfg")
 * @param data Pointer to module data buffer (must have version field as first 2 bytes)
 * @param data_size Size of module data structure
 * @param migrate_func Migration function pointer (can be NULL)
 * @return ESP_OK if loaded successfully, error code otherwise
 */
esp_err_t unified_config_load_module(const char *nvs_key, void *data, size_t data_size,
                               bool (*migrate_func)(uint16_t from_version, uint16_t to_version,
                                                   const void *old_data, size_t old_size,
                                                   void *new_data, size_t new_size));

/**
 * @brief Save module to persistent storage with version-aware checksum
 *
 * Delegates to config_storage layer for persistence operations.
 *
 * @param nvs_key Storage key for this module (e.g., "gps_cfg")
 * @param data Module data including version field as first 2 bytes
 * @param data_size Size of module data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t unified_config_save_module(const char *nvs_key, const void *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* UNIFIED_CONFIG_H */