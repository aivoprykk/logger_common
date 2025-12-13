#include "common_private.h"

static const char *TAG = "config_main";

const char * const config_main_items[] = { CFG_MAIN_ITEM_LIST(STRINGIFY) };
const size_t config_main_item_count = sizeof(config_main_items) / sizeof(config_main_items[0]);

const uint8_t group = SCFG_GROUP_MAIN;

bool get_main_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_main_bar_length:
            strbf_puts(sb, "Bar length in meters used for distance bar calculations.");
            break;
        case cfg_main_ssid:
            strbf_puts(sb, "WiFi SSID for station mode (first network).");
            break;
        case cfg_main_password:
            strbf_puts(sb, "WiFi password for station mode (first network).");
            break;
        case cfg_main_ssid1:
            strbf_puts(sb, "WiFi SSID for station mode (second network).");
            break;
        case cfg_main_password1:
            strbf_puts(sb, "WiFi password for station mode (second network).");
            break;
        case cfg_main_ssid2:
            strbf_puts(sb, "WiFi SSID for station mode (third network).");
            break;
        case cfg_main_password2:
            strbf_puts(sb, "WiFi password for station mode (third network).");
            break;
        case cfg_main_ssid3:
            strbf_puts(sb, "WiFi SSID for station mode (fourth network).");
            break;
        case cfg_main_password3:
            strbf_puts(sb, "WiFi password for station mode (fourth network).");
            break;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_main_gpio12_screens:
            strbf_puts(sb, "Bitfield for screens to cycle through using GPIO12 button.");
            break;
#endif
        case cfg_main_sleep_info:
            strbf_puts(sb, "Information string to display during sleep mode.");
            break;
        default:
            return false;
    }
    return true;
}

bool get_main_item_values(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_main_bar_length:
            // No specific values array
            break;
        case cfg_main_ssid:
        case cfg_main_password:
        case cfg_main_ssid1:
        case cfg_main_password1:
        case cfg_main_ssid2:
        case cfg_main_password2:
        case cfg_main_ssid3:
        case cfg_main_password3:
            // No specific values array
            break;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_main_gpio12_screens:
            // No specific values array
            add_toggles_array(sb, screen_gpio12_items, screen_gpio12_item_count, 0);
            break;
#endif
        case cfg_main_sleep_info:
            // No specific values array
            break;
        default:
            return false;
    }
    return true;
}

bool config_main_value_str(size_t index, struct strbf_s *sb, uint8_t* type) {
    if(!sb || !type) return false;
    switch(index) {
        case cfg_main_bar_length: // bar_length
            strbf_putul(sb, g_rtc_config.main.bar_length);
            *type = SCONFIG_ITEM_TYPE_UINT16;
            break;
        case cfg_main_ssid: // ssid
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[0].ssid);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_password: // password
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[0].password);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_ssid1: // ssid1
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[1].ssid);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_password1: // password1
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[1].password);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_ssid2: // ssid2
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[2].ssid);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_password2: // password2
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[2].password);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_ssid3: // ssid3
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[3].ssid);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        case cfg_main_password3: // password3
            insert_json_string_value(sb, g_rtc_config.main.wifi_sta[3].password);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_main_gpio12_screens: // gpio12_screens
            strbf_putul(sb, g_rtc_config.main.gpio12_screens);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
#endif
        case cfg_main_sleep_info: // sleep_info
            insert_json_string_value(sb, g_rtc_config.main.sleep_info);
            *type = SCONFIG_ITEM_TYPE_STRING;
            break;
        default:
            return false;
    }
    return true;
}
// Main config group (system + wifi settings)
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_main_get_item(size_t index, config_item_info_t *info) {
    if (!info) return false;

    info->name = config_main_items[index];
    info->pos = index;

    // O(1) lookup: index -> enum via main_group_items array
    
    // O(1) lookup: enum -> RTC field via switch
    static char desc_buf[32];
    // printf("get cfg item: %s at %u.\n", info->name, index);
    switch (index) {
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        case cfg_main_speed_large_font:
            info->value = g_rtc_config.main.speed_large_font;
            info->desc = info->value ? "large" : "normal";
            break;
#endif
        case cfg_main_bar_length:
            info->value = g_rtc_config.main.bar_length;
            sprintf(desc_buf, "%lu m", (unsigned long)info->value);
            info->desc = desc_buf;
            break;

        case cfg_main_ssid:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[0].ssid[0];
            info->desc = "ssid";
            break;
            
        case cfg_main_password:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[0].password[0];
            info->desc = "password";
            break;
            
        case cfg_main_ssid1:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[1].ssid[0];
            info->desc = "ssid1";
            break;
            
        case cfg_main_password1:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[1].password[0];
            info->desc = "password1";
            break;
            
        case cfg_main_ssid2:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[2].ssid[0];
            info->desc = "ssid2";
            break;
            
        case cfg_main_password2:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[2].password[0];
            info->desc = "password2";
            break;
            
        case cfg_main_ssid3:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[3].ssid[0];
            info->desc = "ssid3";
            break;
            
        case cfg_main_password3:
            info->value = (uintptr_t)&g_rtc_config.main.wifi_sta[3].password[0];
            info->desc = "password3";
            break;
            
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_main_gpio12_screens:
            info->value = g_rtc_config.main.gpio12_screens;
            info->desc = "gpio_screens";
            break;
#endif
            
        case cfg_main_sleep_info:
            info->value = (uintptr_t)&g_rtc_config.main.sleep_info[0];
            info->desc = "sleep info";
            break;
            
        default:
            return false;
    }

    return true;
}

static bool config_main_set_item_impl(size_t index, uint16_t val, const char *value) {
    FUNC_ENTRY_ARGSD(TAG, "[%s] index=%u", __func__, index);
    if (!value) return false;
    
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }
    
    // O(1) lookup: index -> enum via main_group_items array
    // O(1) lookup: enum -> RTC field via switch
    uint8_t changed = 255;
    switch (index) {
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        case cfg_main_speed_large_font:
            if(g_rtc_config.main.speed_large_font != val) {
                g_rtc_config.main.speed_large_font = val;
                changed = cfg_main_speed_large_font;
            }
            break;
#endif
        case cfg_main_bar_length:
            if(g_rtc_config.main.bar_length != val) {
                g_rtc_config.main.bar_length = val;
                changed = cfg_main_bar_length;
            }
            break;
            
        case cfg_main_ssid:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[0].ssid[0], value)) {
                changed = cfg_main_ssid;
            }
            break;
        case cfg_main_password:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[0].password[0], value)) {
                changed = cfg_main_password;
            }
            break;
            
        case cfg_main_ssid1:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[1].ssid[0], value)) {
                changed = cfg_main_ssid1;
            }
            break;
            
        case cfg_main_password1:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[1].password[0], value)) {
                changed = cfg_main_password1;
            }
            break;
            
        case cfg_main_ssid2:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[2].ssid[0], value)) {
                changed = cfg_main_ssid2;
            }
            break;
            
        case cfg_main_password2:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[2].password[0], value)) {
                changed = cfg_main_password2;
            }
            break;
            
        case cfg_main_ssid3:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[3].ssid[0], value)) {
                changed = cfg_main_ssid3;
            }
            break;
            
        case cfg_main_password3:
            if(set_string_from_json(&g_rtc_config.main.wifi_sta[3].password[0], value)) {
                changed = cfg_main_password3;
            }
            break;
            
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_main_gpio12_screens:
            if(g_rtc_config.main.gpio12_screens != *(uint16_t*)value) {
                g_rtc_config.main.gpio12_screens = *(uint16_t*)value;
                changed = cfg_main_gpio12_screens;
            }
            break;
#endif
            
        case cfg_main_sleep_info:
            if(set_string_from_json(&g_rtc_config.main.sleep_info[0], value)) {
                changed = cfg_main_sleep_info;
            }
            break;
    
        default:
            config_unlock();
            return false;
    }
    if (changed != 255) {
        unified_config_save();
        // Notify all observers of change
        config_observer_notify(group, index);
    }
    config_unlock();
    return true;
}

bool config_main_set_item(size_t index, const char *value) {
    if (!value) {
        return false;
    }
    // Parse string to number (except for ubx_file which needs the string)
    uint16_t val = (index == cfg_gps_ubx_file) ? 0 : strtoul(value, 0, 10);
    return config_main_set_item_impl(index, val, value);
}
