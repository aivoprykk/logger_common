/**
 * @file config_template.c
 * @brief Template for configuration module implementation
 * 
 * This template provides a standardized implementation for configuration modules.
 * 
 * Usage:
 * 1. Copy this file to config_<module>.c
 * 2. Replace all <MODULE> placeholders with actual module name
 * 3. Replace all <module> placeholders with lowercase module name
 * 4. Implement the required functions
 * 5. Add to CMakeLists.txt
 * 
 * @note Module name should be in lowercase (e.g., "gps", "screen", "ubx")
 */

#include "common_private.h"
#include "config_template.h"  // Or your module's header
#include "unified_config.h"
#include <string.h>

// ============================================================================
// Module Configuration
// ============================================================================

static const char *TAG = "config_<module>";

// Item names array (must match CFG_<MODULE>_ITEM_LIST order)
const char * const config_<module>_items[] = { 
    CFG_<MODULE>_ITEM_LIST(STRINGIFY) 
};
const size_t config_<module>_item_count = 
    sizeof(config_<module>_items) / sizeof(config_<module>_items[0]);

// Option arrays for enum values (if needed)
// Example:
// const char * const <module>_options[] = {OPTION_LIST(STRINGIFY)};
// const size_t <module>_options_count = sizeof(<module>_options) / sizeof(<module>_options[0]);

// ============================================================================
// Required API Implementation
// ============================================================================

bool config_<module>_get_item(size_t index, config_item_info_t *info) {
    if (!info || index >= config_<module>_item_count) {
        return false;
    }

    info->name = config_<module>_items[index];
    info->pos = index;

    // Get value from RTC config
    cfg_<module>_t *cfg = &g_rtc_config.<module>;

    switch (index) {
        // Handle each configuration item
        // Example:
        // case cfg_<module>_field1:
        //     info->value = cfg->field1;
        //     info->desc = "Description of field1";
        //     break;
        // case cfg_<module>_field2:
        //     info->value = cfg->field2;
        //     info->desc = "Description of field2";
        //     break;
        default:
            info->value = 0;
            info->desc = not_set;
            break;
    }

    return true;
}

bool config_<module>_set_item(size_t index, const char *value) {
    if (!value || index >= config_<module>_item_count) {
        return false;
    }

    // Acquire configuration lock
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }

    bool changed = false;
    cfg_<module>_t *cfg = &g_rtc_config.<module>;

    // Parse value based on item type
    switch (index) {
        // Handle each configuration item
        // Example:
        // case cfg_<module>_field1: {
        //     uint8_t new_value = (uint8_t)strtoul(value, NULL, 10);
        //     if (cfg->field1 != new_value) {
        //         cfg->field1 = new_value;
        //         changed = true;
        //     }
        //     break;
        // }
        // case cfg_<module>_string_field: {
        //     if (set_string_from_json(cfg->string_field, value)) {
        //         changed = true;
        //     }
        //     break;
        // }
        default:
            config_unlock();
            return false;
    }

    if (changed) {
        // Save to NVS
        unified_config_save_by_submodule(SCFG_GROUP_<MODULE>);

        // Notify observers
        config_observer_notify(SCFG_GROUP_<MODULE>, index);

        ESP_LOGD(TAG, "Item %s changed to %s", config_<module>_items[index], value);
    }

    config_unlock();
    return changed;
}

bool config_<module>_value_str(size_t index, strbf_t *sb, uint8_t *type) {
    if (!sb || !type || index >= config_<module>_item_count) {
        return false;
    }

    cfg_<module>_t *cfg = &g_rtc_config.<module>;

    switch (index) {
        // Handle each configuration item
        // Example:
        // case cfg_<module>_field1:
        //     strbf_putul(sb, cfg->field1);
        //     *type = SCONFIG_ITEM_TYPE_UINT8;
        //     break;
        // case cfg_<module>_field2:
        //     strbf_putul(sb, cfg->field2);
        //     *type = SCONFIG_ITEM_TYPE_UINT16;
        //     break;
        // case cfg_<module>_string_field:
        //     insert_json_string_value(sb, cfg->string_field);
        //     *type = SCONFIG_ITEM_TYPE_STRING;
        //     break;
        default:
            return false;
    }

    return true;
}

bool get_<module>_item_descriptions(size_t index, strbf_t *sb) {
    if (!sb || index >= config_<module>_item_count) {
        return 0;
    }

    switch (index) {
        // Provide descriptions for each item
        // Example:
        // case cfg_<module>_field1:
        //     strbf_puts(sb, "Description of field1");
        //     break;
        // case cfg_<module>_field2:
        //     strbf_puts(sb, "Description of field2");
        //     break;
        default:
            return 0;
    }

    return true;
}

uint8_t get_<module>_item_values(size_t index, strbf_t *sb) {
    if (!sb || index >= config_<module>_item_count) {
        return false;
    }

    switch (index) {
        // Add values array for enum/option items
        // Example:
        // case cfg_<module>_field1:
        //     add_values_array(sb, <module>_options, NULL, <module>_options_count, 0);
        //     break;
        // case cfg_<module>_field2:
        //     // For bitfield toggles:
        //     add_toggles_array(sb, <module>_options, NULL, <module>_options_count, 0);
        //     break;
        default:
            return false;
    }

    return true;
}

// ============================================================================
// Optional API Implementation
// ============================================================================

uint8_t config_<module>_get_next_value(size_t index) {
    if (index >= config_<module>_item_count) {
        return 0;
    }

    cfg_<module>_t *cfg = &g_rtc_config.<module>;
    uint8_t current = 0;
    uint8_t next = 0;

    switch (index) {
        // Implement cycling logic for each item
        // Example:
        // case cfg_<module>_field1:
        //     current = cfg->field1;
        //     next = (current + 1) % <module>_options_count;
        //     break;
        // case cfg_<module>_bool_field:
        //     current = cfg->bool_field ? 1 : 0;
        //     next = !current;
        //     break;
        default:
            break;
    }

    return next;
}

bool config_<module>_set_next_value(size_t index) {
    if (index >= config_<module>_item_count) {
        return false;
    }

    uint8_t next_value = config_<module>_get_next_value(index);
    char value_str[12];
    snprintf(value_str, sizeof(value_str), "%u", next_value);

    return config_<module>_set_item(index, value_str);
}

bool config_<module>_validate(void) {
    cfg_<module>_t *cfg = &g_rtc_config.<module>;
    bool valid = true;

    // Validate each field
    // Example:
    // if (cfg->field1 > MAX_VALUE) {
    //     ESP_LOGW(TAG, "Invalid field1 value: %u", cfg->field1);
    //     valid = false;
    // }
    // if (strlen(cfg->string_field) >= sizeof(cfg->string_field)) {
    //     ESP_LOGW(TAG, "String field too long");
    //     valid = false;
    // }

    return valid;
}

void config_<module>_repair(void) {
    cfg_<module>_t *cfg = &g_rtc_config.<module>;
    bool repaired = false;

    // Repair invalid values
    // Example:
    // if (cfg->field1 > MAX_VALUE) {
    //     ESP_LOGW(TAG, "Repairing field1 from %u to default", cfg->field1);
    //     cfg->field1 = DEFAULT_VALUE;
    //     repaired = true;
    // }
    // if (strlen(cfg->string_field) >= sizeof(cfg->string_field)) {
    //     ESP_LOGW(TAG, "Repairing string field");
    //     cfg->string_field[0] = '\0';
    //     repaired = true;
    // }

    if (repaired) {
        unified_config_save_by_submodule(SCFG_GROUP_<MODULE>);
        ESP_LOGI(TAG, "Configuration repaired");
    }
}

bool config_<module>_migrate(uint16_t from_version, uint16_t to_version) {
    cfg_<module>_t *cfg = &g_rtc_config.<module>;

    // Handle migration between versions
    // Example:
    // if (from_version == 1 && to_version == 2) {
    //     // Convert field from old format to new format
    //     cfg->new_field = cfg->old_field * CONVERSION_FACTOR;
    //     return true;
    // }

    ESP_LOGW(TAG, "Migration from v%u to v%u not implemented", from_version, to_version);
    return false;
}

// ============================================================================
// Module-specific Helper Functions
// ============================================================================

bool config_<module>_is_dirty(void) {
    // Implement dirty flag tracking if needed
    // Could compare with a shadow copy or checksum
    return false;
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

/**
 * @brief Internal implementation for setting values
 * 
 * This separates parsing from the actual setting logic.
 */
static bool config_<module>_set_item_impl(size_t index, uint32_t numeric_value, const char *string_value) {
    if (index >= config_<module>_item_count) {
        return false;
    }

    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }

    bool changed = false;
    cfg_<module>_t *cfg = &g_rtc_config.<module>;

    switch (index) {
        // Same logic as config_<module>_set_item but with parsed values
        // Example:
        // case cfg_<module>_field1: {
        //     if (cfg->field1 != (uint8_t)numeric_value) {
        //         cfg->field1 = (uint8_t)numeric_value;
        //         changed = true;
        //     }
        //     break;
        // }
        // case cfg_<module>_string_field: {
        //     if (string_value && set_string_from_json(cfg->string_field, string_value)) {
        //         changed = true;
        //     }
        //     break;
        // }
        default:
            config_unlock();
            return false;
    }

    if (changed) {
        unified_config_save_by_submodule(SCFG_GROUP_<MODULE>);
        config_observer_notify(SCFG_GROUP_<MODULE>, index);
    }

    config_unlock();
    return changed;
}