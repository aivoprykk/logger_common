#include "common_private.h"

static const char *TAG = "config_advanced";

const char * const config_advanced_items[] = { CFG_ADVANCED_ITEM_LIST(STRINGIFY) };
const size_t config_advanced_item_count = sizeof(config_advanced_items) / sizeof(config_advanced_items[0]);

#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
const char * const seconds_list[] = {"1 sec", "2 sec", "3 sec", "4 sec", "5 sec"};
#endif

bool get_advanced_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_advanced_hostname:
            strbf_puts(sb, "Hostname for the device.");
            break;
#if defined(CFG_ADVANCED_INCLUDE_PRIVATE_ITEMS)
        case cfg_advanced_screen_move_offset:
            strbf_puts(sb, "EPD screen move offset.");
            break;
        case cfg_advanced_speed_field_count:
            strbf_puts(sb, "Number of speed fields to display.");
            break;
#endif
        case cfg_advanced_archive_days:
            strbf_puts(sb, "Number of days to keep logs before archiving.");
            break;
        case cfg_advanced_stat_speed:
            strbf_puts(sb, "Maximum speed in m/s for displaying statistical screens.");
            break;
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
        case cfg_advanced_stat_screens_time:
            strbf_puts(sb, "Time interval between switching statistical screens.");
            break;
#endif
        default:
            return false;
    }
    return true;
}
bool get_advanced_item_values(size_t index, struct strbf_s *sb) {
    switch(index) {
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
        case cfg_advanced_stat_screens_time:
            add_values_array(sb, seconds_list, SECONDS_LIST_COUNT, 1);
            break;
#endif       
        default:
            return false;
    }
    return true;
}

bool config_advanced_value_str(size_t index, struct strbf_s *sb, uint8_t* type) {
    if(!sb || !type) return false;
    switch(index) {
        case cfg_advanced_hostname: // hostname
            insert_json_string_value(sb, &g_rtc_config.advanced.hostname[0]);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
#if defined(CFG_ADVANCED_INCLUDE_PRIVATE_ITEMS)
        case cfg_advanced_screen_move_offset: // screen_move_offset
            strbf_putul(sb, g_rtc_config.advanced.screen_move_offset);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
        case cfg_advanced_speed_field_count: // speed_field_count
            strbf_putul(sb, g_rtc_config.advanced.speed_field_count);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
#endif
        case cfg_advanced_archive_days: // archive_days
            strbf_putul(sb, g_rtc_config.advanced.archive_days);
            *type = SCONFIG_ITEM_TYPE_UINT16;
            break;
        case cfg_advanced_stat_speed: // stat_screen_speed
            strbf_putul(sb, g_rtc_config.advanced.stat_screen_speed);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
        case cfg_advanced_stat_screens_time: // stat_screens_time
            strbf_putul(sb, g_rtc_config.advanced.stat_screens_time);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
#endif
        default:
            return false;
    }
    return true;
}

// Advanced config group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_advanced_get_item(size_t index, config_item_info_t *info) {
    if (!info) return false;
    info->name = config_advanced_items[index];
    info->pos = index;
    // O(1) lookup: index -> enum via advanced_group_items array
    // O(1) lookup: enum -> RTC field via switch
    // printf("get cfg item: %s at %u.\n", info->name, index);
    switch (index) {
        case cfg_advanced_hostname: // hostname
            info->value = (uintptr_t)&g_rtc_config.advanced.hostname[0];
            info->desc = "hostname";
            break;
#if defined(CFG_ADVANCED_INCLUDE_PRIVATE_ITEMS)
        case cfg_advanced_screen_move_offset: // screen_move_offset
            info->value = g_rtc_config.advanced.screen_move_offset;
            info->desc = "screen_move_offset";
            break;
        case cfg_advanced_speed_field_count: // speed_field_count
            info->value = g_rtc_config.advanced.speed_field_count;
            info->desc = "speed field count";
            break;
#endif
        case cfg_advanced_archive_days: // archive_days
            info->value = g_rtc_config.advanced.archive_days;
            info->desc = "Archive logs in days";
            break;
        case cfg_advanced_stat_speed: // stat_screen_speed
            info->value = g_rtc_config.advanced.stat_screen_speed;
            info->desc = "Show stat screen speed up to (m/s)";
            break;
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
        case cfg_advanced_stat_screens_time: // stat_screens_time
            info->value = g_rtc_config.advanced.stat_screens_time;
            info->desc = not_set;
            break;
#endif
        default:
            info->desc = not_set;
            break;
    }
    return true;
}

static bool config_advanced_set_item_impl(size_t index, uint16_t val) {
    FUNC_ENTRY_ARGSD(TAG, "index=%u, value=%hu", index, val);
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }
    
    // O(1) lookup: index -> enum via advanced_group_items array
    // O(1) lookup: enum -> RTC field via switch
    uint8_t changed = 255;
    switch (index) {
#if defined(CFG_ADVANCED_INCLUDE_PRIVATE_ITEMS)
        case cfg_advanced_screen_move_offset:
            if(g_rtc_config.advanced.screen_move_offset != val) {
                FUNC_ENTRY_ARGSD(TAG, "Screen move offset changed to %hhu", val);
                g_rtc_config.advanced.screen_move_offset = val;
                changed = index;
            }
            break;
        case cfg_advanced_speed_field_count:
            if(g_rtc_config.advanced.speed_field_count != val) {
                FUNC_ENTRY_ARGSD(TAG, "Speed field count changed to %hhu", val);
                g_rtc_config.advanced.speed_field_count = val;
                changed = index;
            }
            break;
#endif
        case cfg_advanced_archive_days:
            if(g_rtc_config.advanced.archive_days != val) {
                FUNC_ENTRY_ARGSD(TAG, "Archive days changed to %hhu", val);
                g_rtc_config.advanced.archive_days = val;
                changed = index;
            }
            break;
        case cfg_advanced_stat_speed:
            if(g_rtc_config.advanced.stat_screen_speed != val) {
                FUNC_ENTRY_ARGSD(TAG, "Stat screen speed changed to %hhu", val);
                g_rtc_config.advanced.stat_screen_speed = val;
                changed = index;
            }
            break;
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
        case cfg_advanced_stat_screens_time:
            if(g_rtc_config.advanced.stat_screens_time != val) {
                FUNC_ENTRY_ARGSD(TAG, "Stat screens time changed to %hhu", val);
                g_rtc_config.advanced.stat_screens_time = val;
                changed = index;
            }
            break;
#endif
        default:
            config_unlock();
            return false;
    }
    if (changed != 255) {
        unified_config_save();
        // Notify all observers of change
        config_observer_notify(SCFG_GROUP_ADVANCED, index);
    }
    config_unlock();
    return true;
}

bool config_advanced_set_item(size_t index, const char *value) {
    if (!value) {
        return false;
    }
    uint16_t val = strtoul(value, 0, 10);
    return config_advanced_set_item_impl(index, val);
}

// Unified functions for advanced group
bool config_get_advanced_item(size_t index, config_item_info_t *info) {
    return config_advanced_get_item(index, info);
}