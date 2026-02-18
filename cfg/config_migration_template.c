/**
 * @file config_migration_template.c
 * @brief Configuration migration template
 * 
 * Template for implementing module migration functions.
 * Copy and adapt for each module that needs migration.
 */

#include "common_private.h"
#include "config_gps.h"  // Replace with your module header
#include <string.h>

static const char *TAG = "config_migrate";

/**
 * @brief Example: GPS module migration from v1 to v2
 * 
 * This is a template. Replace with actual migration logic for your module.
 * 
 * Migration rules:
 * 1. Always add new fields at the END of the struct
 * 2. Initialize new fields with sensible defaults
 * 3. Copy existing fields unchanged
 * 4. Return true if migration successful
 */
bool config_gps_migrate(uint16_t from_version, uint16_t to_version,
                       const void *old_data, size_t old_size,
                       void *new_data, size_t new_size) {

    if (from_version == 1 && to_version == 2) {
        // Example: Migrating from GPS v1 to v2
        // Assume v1 struct didn't have a new_field

        if (old_size >= sizeof(cfg_gps_v1_t) && new_size >= sizeof(cfg_gps_t)) {
            const cfg_gps_v1_t *v1 = (const cfg_gps_v1_t *)old_data;
            cfg_gps_t *v2 = (cfg_gps_t *)new_data;

            // Copy unchanged fields
            v2->version = 2;  // Update version
            v2->speed_unit = v1->speed_unit;
            v2->file_date_time = v1->file_date_time;
            v2->log_enables = v1->log_enables;
            v2->stat_screens = v1->stat_screens;
            v2->timezone = v1->timezone;
            strncpy(v2->ubx_file, v1->ubx_file, sizeof(v2->ubx_file));

            // Initialize new field added in v2
            // v2->new_field = DEFAULT_VALUE;

            ESP_LOGI(TAG, "GPS migrated from v%d to v%d", from_version, to_version);
            return true;
        }
    }

    // Add more migration paths as needed
    // if (from_version == 2 && to_version == 3) { ... }

    ESP_LOGW(TAG, "GPS migration from v%d to v%d not implemented", from_version, to_version);
    return false;
}

/**
 * @brief Simple migration helper for minor version changes
 * 
 * Use this when only default values or validation rules change,
 * not the struct layout.
 */
bool config_module_simple_migrate(uint16_t from_version, uint16_t to_version,
                                 const void *old_data, size_t old_size,
                                 void *new_data, size_t new_size) {

    // For simple migrations (same struct layout), just copy the data
    if (old_size == new_size) {
        memcpy(new_data, old_data, old_size);

        // Update version field (must be first 2 bytes)
        *(uint16_t *)new_data = to_version;

        ESP_LOGI(TAG, "Simple migration from v%d to v%d", from_version, to_version);
        return true;
    }

    return false;
}

/**
 * @brief Migration for adding a new field at the end
 * 
 * Use this when adding new fields to the struct.
 * Assumes new struct is larger than old struct.
 */
bool config_module_add_field_migrate(uint16_t from_version, uint16_t to_version,
                                    const void *old_data, size_t old_size,
                                    void *new_data, size_t new_size,
                                    size_t old_struct_size, size_t new_struct_size) {

    if (new_size < new_struct_size) {
        ESP_LOGE(TAG, "New buffer too small for migration");
        return false;
    }

    // Copy old data
    size_t copy_size = old_size < old_struct_size ? old_size : old_struct_size;
    memcpy(new_data, old_data, copy_size);

    // Update version
    *(uint16_t *)new_data = to_version;

    // Zero out new fields (they come after old_struct_size)
    uint8_t *new_bytes = (uint8_t *)new_data;
    size_t new_fields_start = old_struct_size;
    size_t new_fields_size = new_struct_size - old_struct_size;

    if (new_fields_size > 0 && new_fields_start < new_size) {
        size_t zero_size = new_fields_size;
        if (new_fields_start + zero_size > new_size) {
            zero_size = new_size - new_fields_start;
        }
        memset(new_bytes + new_fields_start, 0, zero_size);
    }

    ESP_LOGI(TAG, "Added field migration from v%d to v%d", from_version, to_version);
    return true;
}

/**
 * @brief No-op migration (for modules that don't need migration yet)
 * 
 * Use this as a placeholder until actual migration is needed.
 */
bool config_module_no_migration(uint16_t from_version, uint16_t to_version,
                               const void *old_data, size_t old_size,
                               void *new_data, size_t new_size) {
    // Always return false to trigger defaults
    return false;
}