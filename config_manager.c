// Config Manager Implementation
//
// API Design Notes:
// - Group-specific functions (config_{group}_get_item/set_item) take group-relative indices (0, 1, 2, ...)
// - NEW: Enum-based functions (config_{group}_get_item_by_enum) take sconfig_item_enum_t values
// - NEW: Global functions (config_get_item_by_enum) take any sconfig_item_enum_t value
// - Global enum values (sconfig_item_enum_t) can be used directly in the new enum-based functions
// - Use the unified functions config_get_item(group, index) for programmatic access
// - get_sconfig_item(group, index) handles the mapping from group+relative_index to global config array

#include "config.h"
#include "unified_config.h"  // For unified_config_init/save/repair
#include <string.h>
#include "esp_log.h"

static const char * TAG = "config_manager";

// Shared arrays for item descriptions
const char * const not_set = "not set";
static const char * const _sconfig_group_names[] = {
    SCFG_GROUP_LIST(STRINGIFY)
    SCFG_GROUP_OTHER_LIST(STRINGIFY)
};
const char * sconfig_group_names(sconfig_group_t id) {
   return (id >= 0 && id < SCFG_GROUP_COUNT) ? _sconfig_group_names[id] : "unknown";
};

bool config_manager_is_group_default_hidden(sconfig_group_t group) {
    switch(group) {
        case SCFG_GROUP_ADVANCED:
        case SCFG_GROUP_FW_UPDATE:
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
        case SCFG_GROUP_ADMIN:
#endif
        case SCFG_GROUP_WIFI:
        case SCFG_GROUP_SCREEN:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// Function pointer table for group operations (optimization to eliminate switches)
// ============================================================================

typedef struct {
    bool (*get_item)(size_t index, config_item_info_t *info);
    bool (*set_item)(size_t index, const char *value);
    uint8_t (*get_values)(size_t index, strbf_t *sb);
    bool (*get_descriptions)(size_t index, strbf_t *sb);
    bool (*value_str)(size_t index, strbf_t *sb, uint8_t *type);
    size_t item_count;
    const char * const *item_names;
} config_group_ops_t;

static config_group_ops_t group_ops_table[SCFG_GROUP_COUNT];
static bool group_ops_initialized = false;

static void init_group_ops_table(void) {
    if (group_ops_initialized) return;
    
    group_ops_table[SCFG_GROUP_SCREEN] = (config_group_ops_t){
        .get_item = config_screen_get_item,
        .set_item = config_screen_set_item,
        .get_values = get_screen_item_values,
        .get_descriptions = get_screen_item_descriptions,
        .value_str = config_screen_value_str,
        .item_count = config_screen_item_count,
        .item_names = config_screen_items
    };
    group_ops_table[SCFG_GROUP_WIFI] = (config_group_ops_t){
        .get_item = config_main_get_item,
        .set_item = config_main_set_item,
        .get_values = get_main_item_values,
        .get_descriptions = get_main_item_descriptions,
        .value_str = config_main_value_str,
        .item_count = config_main_item_count,
        .item_names = config_main_items
    };
    group_ops_table[SCFG_GROUP_GPS] = (config_group_ops_t){
        .get_item = config_gps_get_item,
        .set_item = config_gps_set_item,
        .get_values = get_gps_item_values,
        .get_descriptions = get_gps_item_descriptions,
        .value_str = config_gps_value_str,
        .item_count = config_gps_item_count,
        .item_names = config_gps_items
    };
    group_ops_table[SCFG_GROUP_UBX] = (config_group_ops_t){
        .get_item = config_ubx_get_item,
        .set_item = config_ubx_set_item,
        .get_values = get_ubx_item_values,
        .get_descriptions = get_ubx_item_descriptions,
        .value_str = config_ubx_value_str,
        .item_count = config_ubx_item_count,
        .item_names = config_ubx_items
    };
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
    group_ops_table[SCFG_GROUP_ADMIN] = (config_group_ops_t){
        .get_item = config_admin_get_item,
        .set_item = config_admin_set_item,
        .get_values = get_admin_item_values,
        .get_descriptions = get_admin_item_descriptions,
        .value_str = config_admin_value_str,
        .item_count = config_admin_item_count,
        .item_names = config_admin_items
    };
#endif
    group_ops_table[SCFG_GROUP_FW_UPDATE] = (config_group_ops_t){
        .get_item = config_fw_update_get_item,
        .set_item = config_fw_update_set_item,
        .get_values = get_fw_update_item_values,
        .get_descriptions = get_fw_update_item_descriptions,
        .value_str = config_fw_update_value_str,
        .item_count = config_fw_update_item_count,
        .item_names = config_fw_update_items
    };
    group_ops_table[SCFG_GROUP_ADVANCED] = (config_group_ops_t){
        .get_item = config_advanced_get_item,
        .set_item = config_advanced_set_item,
        .get_values = get_advanced_item_values,
        .get_descriptions = get_advanced_item_descriptions,
        .value_str = config_advanced_value_str,
        .item_count = config_advanced_item_count,
        .item_names = config_advanced_items
    };
    
    group_ops_initialized = true;
}

bool insert_json_string_value(struct strbf_s *sb, const char * str) {
    // printf("[%s] str: %s\n", __func__, str ? str : "-");
    if(!sb || !str) return false;
    strbf_putc(sb,'"');
    uint8_t len = strlen(str);
    for(uint8_t i = 0; i < len; ++i) {
        if(*(str + i) == '"' && (i == 0 || *(str + i - 1) != '\\')) strbf_putc(sb, '\\');
        strbf_putc(sb, *(str+i));
    }
    strbf_putc(sb,'"');
    return true;
}

bool set_string_from_json(char * str, const char * json) {
    // printf("[%s] json: %s, str: %s\n", __func__, json ? json : "-", str ? str : "-");
    if(!str || !json) return false;
    while(*json == '"') ++json;
    const char * end = json;
    while(end && (*end != '"' || (end > json && *end == '"' && *(end-1) == '\\'))) ++end;
    uint8_t len = end - json, slen = strlen(str);
    // printf("test if str not equal to json: %hhu != %hhu\n", len, slen);
    if(!strncmp(json, str, (len > slen ? len : slen))) return false;
    if(len) memcpy(str, json, len);
    *(str+(len)) = '\0';
    // printf("set string: %s from %s\n", str, json);
    return true;
}

// Helper function to add values array to JSON (matches original logger_config format)
void add_values_array(strbf_t *sb, const char * const *options, const uint8_t * option_values, size_t count, uint8_t init_value) {
    if (!options || count == 0) return;
    
    strbf_puts(sb, ",\"values\":[");
    for (size_t i = 0; i < count; i++) {
        if (i > 0) strbf_putc(sb, ',');
        strbf_puts(sb, "{\"value\":");
        if(option_values) {
            strbf_putul(sb, option_values[i]);
        } else {
            strbf_putul(sb, i + init_value);
        }
        strbf_puts(sb, ",\"title\":\"");
        strbf_puts(sb, options[i]);
        strbf_puts(sb, "\"}");
    }
    strbf_putc(sb, ']');
}

// Helper function to add values array to JSON (matches original logger_config format)
void add_toggles_array(strbf_t *sb, const char * const *options, const uint8_t * option_values, size_t count, uint8_t init_value) {
    if (!options || count == 0) return;
    
    strbf_puts(sb, ",\"toggles\":[");
    uint16_t j = 1;
    for(size_t i= 0, k = count; i < k; i++, j <<= 1) {
        if (i > 0) strbf_putc(sb, ',');
        strbf_puts(sb, "{\"pos\":");
        if(option_values) {
            strbf_putul(sb, option_values[i]);
        } else {
            strbf_putul(sb, i + init_value);
        }
        strbf_puts(sb, ",\"title\":\"");
        strbf_puts(sb, options[i]);
        strbf_puts(sb, "\",\"value\":");
        strbf_putn(sb, j);
        strbf_puts(sb, "}");
    }
    strbf_putc(sb, ']');
}

// Helper function to get type string from sconfig type
static const char* get_type_string(sconfig_item_type_t type) {
    switch (type) {
        case SCONFIG_ITEM_TYPE_BOOL:
            return "bool";
        case SCONFIG_ITEM_TYPE_STRING:
            return "str";
        default:
            return "int";
    }
}

bool config_get_item_description(sconfig_group_t group, size_t index, struct strbf_s *sb) {
    if (group >= SCFG_GROUP_COUNT || !group_ops_table[group].get_descriptions) return false;
    return group_ops_table[group].get_descriptions(index, sb);
}

uint8_t config_get_item_values(sconfig_group_t group, size_t index, strbf_t *sb) {
    if (group >= SCFG_GROUP_COUNT || !group_ops_table[group].get_values) return false;
    return group_ops_table[group].get_values(index, sb);
}

bool config_get_item_value_str(uint8_t group_id, size_t idx, struct strbf_s *sb, uint8_t* type) {
    if (group_id >= SCFG_GROUP_COUNT || !group_ops_table[group_id].value_str) return false;
    group_ops_table[group_id].value_str(idx, sb, type);
    return true;
}

// Unified functions

bool config_get_cycle_item(sconfig_cycle_group_t group, size_t index, struct m_config_item_s *item) {
    if (group >= SCFG_CYCLE_GROUP_COUNT || !item) {
        return false;
    }
    // Route to appropriate group handler
    config_item_info_t info;
    switch (group) {
        case SCFG_CYCLE_GROUP_SCREEN:
            config_screen_get_item(index, &info);
            break;
        case SCFG_CYCLE_GROUP_FW_UPDATE:
            config_fw_update_get_item(index, &info);
            break;
        case SCFG_CYCLE_GROUP_UBX:
            config_ubx_get_item(index, &info);
            break;
        case SCFG_CYCLE_GROUP_GPS:
            config_gps_get_item(index, &info);
            break;
        case SCFG_CYCLE_GROUP_STAT_SCREENS:
            config_stat_screen_get_item(index, &info);
            break;
        default:
            return false;
    }
    // Populate m_config_item_s structure
    item->name = info.name;
    item->pos = info.pos;
    item->value = info.value;
    item->desc = info.desc;
    return true;
}

bool config_set_item(sconfig_group_t group, size_t index, const char *value) {
    if (group >= SCFG_GROUP_COUNT || !value || !group_ops_table[group].set_item) return false;
    return group_ops_table[group].set_item(index, value);
}

uint16_t config_get_next_value(sconfig_cycle_group_t group, size_t index) {
    if (group >= SCFG_CYCLE_GROUP_COUNT) {
        return 0;
    }
    uint16_t next_value = 0;
    // Route to appropriate group handler's cycle function
    switch (group) {
        case SCFG_CYCLE_GROUP_UBX:
            return config_ubx_get_next_value(index);
        case SCFG_CYCLE_GROUP_GPS:
            return config_gps_get_next_value(index);
        case SCFG_CYCLE_GROUP_SCREEN:
            return config_screen_get_next_value(index);
        case SCFG_CYCLE_GROUP_FW_UPDATE:
            return config_fw_update_get_next_value(index);
        case SCFG_CYCLE_GROUP_STAT_SCREENS:
            return config_stat_screen_get_next_value(index);
        default:
            ESP_LOGW(TAG, "Get next not implemented for group %d", group);
            return 0;
    }
}

bool config_get_next_cycle_idx(enum sconfig_cycle_group_e group, size_t index, size_t *next_index) {
    if (group >= SCFG_CYCLE_GROUP_COUNT || !next_index) {
        return false;
    }
    switch(group) {
        case SCFG_CYCLE_GROUP_UBX:
            if(++index >= config_ubx_item_count)
                index = 0;
            break;
        case SCFG_CYCLE_GROUP_GPS:
            if(++index == cfg_gps_stat_screens) ++index; // skip stat screens pseudo-item
            if(index == cfg_gps_ubx_file) ++index;
            if(index >= config_gps_item_count)
                index = 0;
            break;
        case SCFG_CYCLE_GROUP_SCREEN:
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1) || defined(CONFIG_UBUTTON_GPIO_1)
            if(++index >= cfg_screen_gpio12_screens) ++index;    // GPIO12 screens
#else       
            ++index;
#endif
            if(index >= config_screen_item_count)
                index = 0;
            break;
        case SCFG_CYCLE_GROUP_FW_UPDATE:
            if(++index >= 2)
                index = 0;
            break;
        case SCFG_CYCLE_GROUP_STAT_SCREENS:
            if(++index >= gps_stat_screen_item_count)
                index = 0;
            break;
        default:
            ESP_LOGW(TAG, "Get next index not implemented for group %d", group);
            return false;
    }
    *next_index = index;
    return true;
}

bool config_set_cycle_item(sconfig_cycle_group_t group, size_t index) {
    if (group >= SCFG_CYCLE_GROUP_COUNT) {
        return false;
    }
    // Route to appropriate group handler's cycle function
    switch (group) {
        case SCFG_CYCLE_GROUP_UBX:
            return config_ubx_set_next_value(index);
        case SCFG_CYCLE_GROUP_GPS:
            return config_gps_set_next_value(index);
        case SCFG_CYCLE_GROUP_SCREEN:
            return config_screen_set_next_value(index);
        case SCFG_CYCLE_GROUP_FW_UPDATE:
            return config_fw_update_set_next_value(index);
        case SCFG_CYCLE_GROUP_STAT_SCREENS:
            return config_stat_screen_set_next_value(index);
        default:
            ESP_LOGW(TAG, "Cycle not implemented for group %d", group);
            return false;
    }
}

size_t config_get_group_size(sconfig_group_t group) {
    if (group >= SCFG_GROUP_COUNT) return 0;
    return group_ops_table[group].item_count;
}

bool config_get_group_data(uint8_t group_id, const char * const ** names, uint8_t *count) {
    if (group_id >= SCFG_GROUP_COUNT) {
        *count = 0;
        return false;
    }
    *count = group_ops_table[group_id].item_count;
    *names = group_ops_table[group_id].item_names;
    return true;
}

bool config_get_by_name(const char *name, uint8_t * group_id, uint8_t * index) {
    if (!name) return false;
    
    // Directly iterate ops table instead of calling config_get_group_data
    for (uint8_t group = 0; group < SCFG_GROUP_COUNT; ++group) {
        const config_group_ops_t *ops = &group_ops_table[group];
        const char * const *item_names = ops->item_names;
        
        if (!item_names) continue;
        
        for (uint8_t i = 0; i < ops->item_count; ++i) {
            if (!strcmp(item_names[i], name)) {
                *index = i;
                *group_id = group;
                return true;
            }
        }
    }
    return false;
}

// Unified get item function - dispatches to group-specific get_item functions
bool config_get_item(sconfig_group_t group, size_t index, config_item_info_t *info) {
    if (!info || group >= SCFG_GROUP_COUNT || !group_ops_table[group].get_item) return false;
    return group_ops_table[group].get_item(index, info);
}

bool config_set_item_by_name(const char *name, const char *value) {
    if (!name || !value) return false;
    uint8_t group_id, index;
    if (!config_get_by_name(name, &group_id, &index)) return false;
    return config_set_item(group_id, index, value);
}

// Get JSON for a specific configuration item by group and index
int config_manager_get_item_json(uint8_t group, size_t index, strbf_t *sb) {
    config_item_info_t info;
    if (!config_get_item(group, index, &info)) return 0;
    size_t start = sb->cur - sb->start;
    strbf_puts(sb, "{\"name\":\"");
    strbf_puts(sb, info.name);
    strbf_puts(sb, "\",\"value\":");
    strbf_putul(sb, info.value);
    strbf_puts(sb, ",\"desc\":\"");
    if (info.desc) strbf_puts(sb, info.desc);
    strbf_puts(sb, "\"}");
    return sb->cur - sb->start - start;
}

// Optimized version using pre-fetched ops table (avoids 3 extra table lookups)
static int config_manager_get_item_json_with_ops(const config_group_ops_t *ops, const char *name, size_t idx, strbf_t *sb) {
    config_item_info_t info = {0};
    if (!ops->get_item || !ops->get_item(idx, &info)) 
        return 0;
    size_t start_len = sb->cur - sb->start;
    strbf_puts(sb, "{\"name\":\"");
    strbf_puts(sb, name);
    strbf_puts(sb, "\",\"value\":");
    uint8_t type = SCONFIG_ITEM_TYPE_MAX;

    // item value - direct call instead of table lookup
    if (ops->value_str) {
        ops->value_str(idx, sb, &type);
    }

    // Add info field with specific description - direct call
    strbf_puts(sb, ",\"info\":\"");
    if(!ops->get_descriptions || !ops->get_descriptions(idx, sb)) {
        strbf_puts(sb,"configuration item");
    }
    strbf_puts(sb, "\"");

    uint8_t values = 0;
    if (ops->get_values) {
        values = ops->get_values(idx, sb);
    }
    // Add type field
    strbf_puts(sb, ",\"type\":\"");
    strbf_puts(sb, get_type_string(type));
    strbf_puts(sb, "\"");
    // Add values array if applicable - direct call
    if(!values && (info.min || info.max)) {
        strbf_puts(sb, ",\"min\":");
        strbf_putul(sb, info.min);
        strbf_puts(sb, ",\"max\":");
        strbf_putul(sb, info.max);
    }
    strbf_puts(sb, "}");
    return (sb->cur - sb->start) - start_len; // bytes written
}

// Get configuration item by name (appends JSON to strbf)
int config_manager_get_item_json_l(const char * name, uint8_t group_id, uint8_t idx, strbf_t *sb) {
    if (group_id >= SCFG_GROUP_COUNT) return 0;
    return config_manager_get_item_json_with_ops(&group_ops_table[group_id], name, idx, sb);
}

int config_manager_get_item_by_name(const char * name, strbf_t *sb) {
    if (!name) return 0;
    
    uint8_t group_id, index;
    if (!config_get_by_name(name, &group_id, &index)) return 0;
    
    // Directly use ops table instead of going through config_manager_get_item_json_l
    // which would just do another bounds check
    const config_group_ops_t *ops = &group_ops_table[group_id];
    return config_manager_get_item_json_with_ops(ops, name, index, sb);
}

int config_manager_get_item_by_group_idx(uint8_t group, size_t index, strbf_t *sb) {
    if (group >= SCFG_GROUP_COUNT) return 0;
    const config_group_ops_t *ops = &group_ops_table[group];
    
    // Single table lookup, then direct function calls
    config_item_info_t info;
    if (!ops->get_item || !ops->get_item(index, &info)) return 0;
    
    return config_manager_get_item_json_with_ops(ops, info.name, index, sb);
}

// Get all configuration items as JSON array (appends items with commas)
void config_manager_get_all_items(strbf_t *sb, bool *first) {
    for (sconfig_group_t group = 0; group < SCFG_GROUP_COUNT; group++) {
        size_t size = config_get_group_size(group);
        for (size_t index = 0; index < size; index++) {
            // Use the new config_manager_get_item_json function instead of obsolete get_sconfig_item
            config_manager_get_item_json(group, index, sb);
            // Append comma if not the last item
            if (group != SCFG_GROUP_COUNT - 1 || index != size - 1) {
                strbf_putc(sb, ',');
            }
        }
    }
}

// Manager functions
void config_manager_init(void) {
    // Initialize function pointer table
    init_group_ops_table();
    // Initialize sconfig system first (initializes NVS)
    sconfig_init();
    // Then initialize unified RTC config (loads from NVS, validates checksums)
    unified_config_init();
}

/**
 * @brief Map a config item name to its submodule group
 * @param name Config item name (e.g., "ubx_output_rate", "gps_timezone")
 * @return Submodule group, or SCFG_GROUP_COUNT if not found
 * 
 * Note: STAT_SCREENS items are part of GPS config in unified storage
 */
static sconfig_group_t config_get_submodule_from_item_name(const char *name) {
    if (!name) return SCFG_GROUP_COUNT;
    
    // GPS items (includes stat_screens)
    if (strstr(name, "gps_") == name || strstr(name, "stat_") == name) 
        return SCFG_GROUP_GPS;
    
    // UBX items
    if (strstr(name, "ubx_") == name) return SCFG_GROUP_UBX;
    
    // Screen items
    if (strstr(name, "screen_") == name) return SCFG_GROUP_SCREEN;
    
    // WiFi items
    if (strstr(name, "ssid") || strstr(name, "password") || 
        strstr(name, "bar_length") || strstr(name, "gpio") || 
        strstr(name, "sleep_info")) return SCFG_GROUP_WIFI;
    
    // FW update items
    if (strstr(name, "fw_") == name || strstr(name, "ota_") == name) 
        return SCFG_GROUP_FW_UPDATE;
    
    // Admin items
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
    if (strstr(name, "admin_") == name) return SCFG_GROUP_ADMIN;
#endif
    // Advanced items (brightness, move offset)
    if (strstr(name, "brightness") || strstr(name, "move_offset"))
        return SCFG_GROUP_ADVANCED;
    
    return SCFG_GROUP_COUNT;  // Unknown
}

bool config_manager_save_submodule(int submodule_int) {
    sconfig_group_t submodule = (sconfig_group_t)submodule_int;
    // Save only the specified submodule to NVS
    // Supported submodules for unified config
    switch(submodule) {
        case SCFG_GROUP_GPS:
        case SCFG_GROUP_UBX:
        case SCFG_GROUP_SCREEN:
        case SCFG_GROUP_WIFI:
        case SCFG_GROUP_FW_UPDATE:
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
        case SCFG_GROUP_ADMIN:
#endif
        case SCFG_GROUP_ADVANCED:
            unified_config_save_by_submodule(submodule);
            return sconfig_commit() == 0;
        default:
            ESP_LOGW(TAG, "Cannot save unknown submodule %d", submodule);
            return false;
    }
}

bool config_manager_save_by_item_name(const char *name) {
    // Determine which submodule contains this item and save only that
    uint8_t submodule, index;
    if (!config_get_by_name(name, &submodule, &index)) return 0;
    // sconfig_group_t submodule = config_get_submodule_from_item_name(name);
    if (submodule == SCFG_GROUP_COUNT) {
        ESP_LOGW(TAG, "Could not determine submodule for item: %s", name);
        return false;
    }
    ESP_LOGD(TAG, "Saving item '%s' to submodule %d", name, submodule);
    return config_manager_save_submodule(submodule);
}

bool config_manager_save(void) {
    // Save unified config to NVS
    unified_config_save();
    // Commit sconfig changes
    return sconfig_commit() == 0;
}

bool config_manager_load(void) {
    // unified_config already loaded in init
    // sconfig automatically loads defaults on init
    return true;
}

bool config_manager_reset(void) {
    // Reset both systems to factory defaults
    unified_config_reset_to_defaults();  // Force defaults and save
    return sconfig_reset() == 0;
}

bool config_manager_repair(void) {
    // Repair corrupted config (tries NVS restore first, defaults as fallback)
    unified_config_repair();
    // sconfig doesn't need repair - it auto-loads on init
    return true;
}