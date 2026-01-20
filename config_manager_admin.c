#include <string.h>
#include "common_private.h"

#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
static const char *TAG = "config_admin";

const char * const config_admin_items[] = { CFG_ADMIN_ITEM_LIST(STRINGIFY) };
const size_t config_admin_item_count = sizeof(config_admin_items) / sizeof(config_admin_items[0]);

uint8_t get_admin_item_values(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_admin_admin_username:
        case cfg_admin_admin_password:
            strbf_puts(sb, ",\"depends\":\"admin_auth\"");
            return 0;
            break;
        default:
            return 0;
    }
}

bool get_admin_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_admin_admin_auth:
            strbf_puts(sb, "Enable or disable admin authentication. 0 = disabled, 1 = enabled.");
            break;
        case cfg_admin_admin_username:
            strbf_puts(sb, "Username for admin access.");
            break;
        case cfg_admin_admin_password:
            strbf_puts(sb, "Password for admin access.");
            break;
        default:
            return false;
    }
    return true;
}

bool config_admin_value_str(size_t index, struct strbf_s *sb, uint8_t* type) {
    if(!sb || !type) return false;
    switch(index) {
        case cfg_admin_admin_auth: // auth
            strbf_putul(sb, g_rtc_config.admin.admin_auth);
            *type = SCONFIG_ITEM_TYPE_BOOL;
            break;
        case cfg_admin_admin_username: // username
            insert_json_string_value(sb, g_rtc_config.admin.admin_username);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_admin_admin_password: // password
            insert_json_string_value(sb, g_rtc_config.admin.admin_password);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        default:
            return false;
    }
    return true;
}

// Admin group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_admin_get_item(size_t index, config_item_info_t *info) {
    if (!info) return false;
    info->name = config_admin_items[index];
    info->pos = index;
    // Handle different types and set descriptions
    // printf("get cfg item: %s at %u.\n", info->name, index);
    switch (index) {
        case cfg_admin_admin_auth: // auth
            info->value = g_rtc_config.admin.admin_auth;
            info->desc = info->value ? "enabled" : "disabled";
            return true;
        case cfg_admin_admin_username: // username
            info->value = (uintptr_t)&g_rtc_config.admin.admin_username[0];
            info->desc = "admin username";
            return true;
        case cfg_admin_admin_password: // password
            info->value = (uintptr_t)&g_rtc_config.admin.admin_password[0];
            info->desc = "admin password";
            return true;
        default:
            break;
    }
    return true;
}

static bool config_admin_set_item_impl(size_t index, uint16_t val, const char *value) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }
    uint8_t changed = 255;
    switch(index) {
        case cfg_admin_admin_auth:
            if(g_rtc_config.admin.admin_auth != val) {
                FUNC_ENTRY_ARGSD(TAG, "Admin auth changed to %s", val ? "enabled" : "disabled");
                g_rtc_config.admin.admin_auth = val;
                changed = index;
            }
            break;
        case cfg_admin_admin_username:
            if(set_string_from_json(g_rtc_config.admin.admin_username, value)) {
                FUNC_ENTRY_ARGSD(TAG, "Admin username changed to: %s", value);
                changed = index;
            }
            break;
        case cfg_admin_admin_password:
            if(set_string_from_json(g_rtc_config.admin.admin_password, value)) {
                FUNC_ENTRY_ARGSD(TAG, "Admin password changed to: %s", value);
                changed = index;
            }
            break;
        default:
            config_unlock();
            return true;
    }
    if (changed != 255) {
        unified_config_save_by_submodule(SCFG_GROUP_ADMIN);
        // Notify all observers of change
        config_observer_notify(SCFG_GROUP_ADMIN, index);
    }
    config_unlock();
    return true;
}

bool config_admin_set_item(size_t index, const char *value) {
    if (!value) {
        return false;
    }
    // Parse string to number (except for ubx_file which needs the string)
    uint16_t val = (index == cfg_admin_admin_auth) ? strtoul(value, 0, 10) : 0;
    return config_admin_set_item_impl(index, val, value);
}
#endif