#include "common_private.h"

static const char *TAG = "config_fw_update";

const char * const config_fw_update_channels[] = {CFG_FW_UPDATE_CHANNEL_ITEM_LIST(STRINGIFY)};
const size_t config_fw_update_channels_count = sizeof(config_fw_update_channels) / sizeof(config_fw_update_channels[0]);

const char * const config_fw_update_items[] = { CFG_FW_UPDATE_ITEM_LIST(STRINGIFY) };
const size_t config_fw_update_item_count = sizeof(config_fw_update_items) / sizeof(config_fw_update_items[0]);

bool get_fw_update_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_fw_update_update_enabled:
            strbf_puts(sb, "Enable or disable firmware updates. 0 = disabled, 1 = enabled.");
            break;
        case cfg_fw_update_update_channel:
            strbf_puts(sb, "Firmware update channel selection. 0 = stable, 1 = unstable.");
            break;
        default:
            return false;
    }
    return true;
}

bool get_fw_update_item_values(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_fw_update_update_channel:
            strbf_puts(sb, ",\"depends\":\"update_enabled\"");
            add_values_array(sb, config_fw_update_channels, 0, config_fw_update_channels_count, 0);
            break;
        default:
            return false;
    }
    return true;
}

bool config_fw_update_value_str(size_t index, struct strbf_s *sb, uint8_t* type) {
    if(!sb || !type) return false;
    switch(index) {
        case cfg_fw_update_update_enabled: // enabled
            strbf_putul(sb, g_rtc_config.fw_update.update_enabled);
            *type = SCONFIG_ITEM_TYPE_BOOL;
            break;
        case cfg_fw_update_update_channel: // channel
            strbf_putul(sb, g_rtc_config.fw_update.update_channel);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
        default:
            return false;
    }
    return true;
}

// Firmware update group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_fw_update_get_item(size_t index, config_item_info_t *info) {
    if (!info) return false;
    info->name = config_fw_update_items[index];
    info->pos = index;
    // O(1) lookup: index -> enum via screen_group_items array
    // printf("get cfg item: %s at %u.\n", info->name, index);
    switch (index) {
        case cfg_fw_update_update_enabled: // enabled
            info->value = g_rtc_config.fw_update.update_enabled;
            info->desc = info->value ? "yes" : "no";
            break;
        case cfg_fw_update_update_channel: // channel
            info->value = g_rtc_config.fw_update.update_channel;
            info->desc = config_fw_update_channels[info->value];
            break;
        default:
            info->desc = not_set;
            break;
    }
    return true;
}

static bool config_fw_update_set_item_impl(size_t index, uint16_t val) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u, value: %hhu", index, val);
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }
    
    // O(1) lookup: index -> enum via screen_group_items array
    // O(1) lookup: enum -> RTC field via switch
    uint8_t changed = 255;
    switch (index) {
        case cfg_fw_update_update_enabled: // enabled
            if(g_rtc_config.fw_update.update_enabled != val) {
                FUNC_ENTRY_ARGSD(TAG, "Firmware update enabled changed to %hhu", val);
                g_rtc_config.fw_update.update_enabled = val;
                changed = index;
            }
            break;
        case cfg_fw_update_update_channel: // channel
            if(g_rtc_config.fw_update.update_channel != val) {
                FUNC_ENTRY_ARGSD(TAG, "Firmware update channel changed to %hhu", val);
                g_rtc_config.fw_update.update_channel = val;
                changed = index;
            }
            break;
        default:
            config_unlock();
            return false;
    }
    if (changed != 255) {
        unified_config_save();
        // Notify all observers of change
        config_observer_notify(SCFG_GROUP_FW_UPDATE, index);
    }
    config_unlock();
    return true;
}

bool config_fw_update_set_item(size_t index, const char *value) {
    if (!value) {
        return false;
    }
    uint16_t val = strtoul(value, 0, 10);
    return config_fw_update_set_item_impl(index, val);
}

uint8_t config_fw_update_get_next_value(size_t index) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
    uint8_t new_value = 0;
    switch (index) {
        case cfg_fw_update_update_channel:
            new_value = g_rtc_config.fw_update.update_channel ? 0 : 1;
            break;
        case cfg_fw_update_update_enabled:
            new_value = g_rtc_config.fw_update.update_enabled ? 0 : 1;
            break;
        default:
            break;
    }
    return new_value;
}

bool config_fw_update_set_next_value(size_t index) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
    if(config_fw_update_set_item_impl(index, config_fw_update_get_next_value(index))) {
        return true;
    }
    return false;
}