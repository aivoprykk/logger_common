#include "common_private.h"

const char * const config_screen_items[] = { CFG_SCREEN_ALL_ITEM_LIST(STRINGIFY) };
const size_t config_screen_item_count = sizeof(config_screen_items) / sizeof(config_screen_items[0]);
const char * const config_speed_field_items[] = { SPEED_SCREEN_UPPER_FIELD_ITEM_LIST(STRINGIFY) };
const size_t config_speed_field_item_count = sizeof(config_speed_field_items) / sizeof(config_speed_field_items[0]);
const char * const bat_views[] = { BAT_VIEW_LIST(STRINGIFY) };
const size_t bat_views_count = sizeof(bat_views) / sizeof(bat_views[0]);
const char * const board_logos[] = {BOARD_LOGO_ITEM_LIST(STRINGIFY)};
const size_t board_logos_count = sizeof(board_logos) / sizeof(board_logos[0]);
const char * const sail_logos[] = {SAIL_LOGO_ITEM_LIST(STRINGIFY)};
const size_t sail_logos_count = sizeof(sail_logos) / sizeof(sail_logos[0]);
const char * const screen_rotations[] = {SCREEN_ROTATION_ITEM_LIST(STRINGIFY)};
const size_t screen_rotations_count = sizeof(screen_rotations) / sizeof(screen_rotations[0]);

static const char *TAG = "config_screen";

bool get_screen_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_screen_speed_field:
            strbf_puts(sb, "Choice for first field in speed screen.");
            break;
        case cfg_screen_board_logo:
            strbf_puts(sb, "Choice for board logo displayed on screen.");
            break;
        case cfg_screen_sail_logo:
            strbf_puts(sb, "Choice for sail logo displayed on screen.");
            break;
        case cfg_screen_screen_rotation:
            strbf_puts(sb, "Screen rotation setting.");
            break;
        case cfg_screen_bat_view:
            strbf_puts(sb, "Battery view mode on the display.");
            break;
        default:
            return false;
    }
    return true;
}

bool get_screen_item_values(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_screen_speed_field:
            add_values_array(sb, config_speed_field_items, 0, config_speed_field_item_count, 0);
            break;
        case cfg_screen_board_logo:
            add_values_array(sb, board_logos, 0, board_logos_count, 1);
            break;
        case cfg_screen_sail_logo:
            add_values_array(sb, sail_logos, 0, sail_logos_count, 1);
            break;
        case cfg_screen_screen_rotation:
            add_values_array(sb, screen_rotations, 0, screen_rotations_count, 0);
            break;
        case cfg_screen_bat_view:
            add_values_array(sb, bat_views, 0, bat_views_count, 0);
            break;
        default:
            return false;
    }
    return true;
}

bool config_screen_value_str(size_t index, struct strbf_s *sb, uint8_t* type) {
    if(!sb || !type) return false;
    switch(index) {
        case cfg_screen_speed_field: // speed_field
            strbf_putul(sb, g_rtc_config.screen.speed_field);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            return true;
        case cfg_screen_board_logo: // board_logo
            strbf_putul(sb, g_rtc_config.screen.board_logo);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            return true;
        case cfg_screen_sail_logo: // sail_logo
            strbf_putul(sb, g_rtc_config.screen.sail_logo);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            return true;
        case cfg_screen_screen_rotation: // screen_rotation
            strbf_putn(sb, g_rtc_config.screen.screen_rotation);
            *type = SCONFIG_ITEM_TYPE_INT8;
            return true;
        case cfg_screen_bat_view: // bat_view
            strbf_putul(sb, g_rtc_config.screen.bat_view);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            return true;
#if !defined(CONFIG_LCD_IS_EPD)
        case cfg_screen_screen_brightness:
            strbf_putul(sb, g_rtc_config.screen.screen_brightness);
            *type = SCONFIG_ITEM_TYPE_INT8;
            return true;
#endif
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        case cfg_screen_speed_large_font:
            strbf_putul(sb, g_rtc_config.screen.speed_large_font);
            *type = SCONFIG_ITEM_TYPE_BOOL;
            return true;
#endif
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_screen_gpio12_screens:
            strbf_putul(sb, g_rtc_config.screen.gpio12_screens);
            *type = SCONFIG_ITEM_TYPE_UINT16;
            return true;
#endif
        default:
            return false;
    }
}

// Screen settings group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_screen_get_item(size_t index, config_item_info_t *info) {
    if (!info) return false;
    info->name = config_screen_items[index];
    info->pos = index;
    // O(1) lookup: index -> enum via screen_group_items array
    // O(1) lookup: enum -> RTC field via switch
    // printf("get cfg item: %s at %u.\n", info->name, index);
    switch (index) {
        case cfg_screen_speed_field: // speed_field
            info->value = g_rtc_config.screen.speed_field;
            info->desc = config_speed_field_items[info->value];
            break;
        case cfg_screen_board_logo: // board_logo
            info->value = g_rtc_config.screen.board_logo;
            if (info->value > 0 && info->value <= board_logos_count) {
                info->desc = board_logos[info->value - 1];
            } else {
                info->desc = not_set;
            }
            break;
        case cfg_screen_sail_logo: // sail_logo
            info->value = g_rtc_config.screen.sail_logo;
            if (info->value > 0 && info->value <= sail_logos_count) {
                info->desc = sail_logos[info->value - 1];
            } else {
                info->desc = not_set;
            }
            break;
        case cfg_screen_screen_rotation: // screen_rotation
            info->value = g_rtc_config.screen.screen_rotation;
            if (info->value < screen_rotations_count) {
                info->desc = screen_rotations[info->value];
            } else {
                info->desc = not_set;
            }
            break;
        case cfg_screen_bat_view: // bat_view
            info->value = g_rtc_config.screen.bat_view;
            if (info->value < bat_views_count) {
                info->desc = bat_views[info->value];
            } else {
                info->desc = not_set;
            }
            break;
#if !defined(CONFIG_LCD_IS_EPD)
        case cfg_screen_screen_brightness:
            info->value = g_rtc_config.screen.screen_brightness;
            info->desc = not_set;
            break;
#endif
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        case cfg_screen_speed_large_font:
            info->value = g_rtc_config.screen.speed_large_font;
            info->desc = not_set;
            break;
#endif
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_screen_gpio12_screens:
            info->value = g_rtc_config.screen.gpio12_screens;
            info->desc = not_set;
            break;
#endif
        default:
            info->desc = not_set;
            break;
    }
    return true;
}

// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
static bool config_screen_set_item_impl(size_t index, uint16_t val) {
    FUNC_ENTRY_ARGSD(TAG, "index=%u, value=%u", index, val);
    if (!config_lock(-1)) {
        ELOG(TAG, "Failed to acquire config lock");
        return false;
    }

    // O(1) lookup: index -> enum via screen_group_items array
    uint8_t changed = 255;
    // O(1) lookup: enum -> RTC field via switch
    switch (index) {
        case cfg_screen_speed_field:
            if(g_rtc_config.screen.speed_field != val) {
                FUNC_ENTRY_ARGSD(TAG, "Speed field changed to %u", val);
                g_rtc_config.screen.speed_field = val;
                changed = index;
            }
            break;
        case cfg_screen_board_logo:
            if(g_rtc_config.screen.board_logo != val) {
                FUNC_ENTRY_ARGSD(TAG, "Board logo changed to %u", val);
                g_rtc_config.screen.board_logo = val;
                changed = index;
            }
            break;
        case cfg_screen_sail_logo:
            if(g_rtc_config.screen.sail_logo != val) {
                FUNC_ENTRY_ARGSD(TAG, "Sail logo changed to %u", val);
                g_rtc_config.screen.sail_logo = val;
                changed = index;
            }
            break;
        case cfg_screen_screen_rotation:
            if(g_rtc_config.screen.screen_rotation != val) {
                FUNC_ENTRY_ARGSD(TAG, "Screen rotation changed to %u", val);
                g_rtc_config.screen.screen_rotation = val;
                changed = index;
            }
            break;
        case cfg_screen_bat_view:
            if(g_rtc_config.screen.bat_view != val) {
                FUNC_ENTRY_ARGSD(TAG, "Battery view changed to %u", val);
                g_rtc_config.screen.bat_view = val;
                changed = index;
            }
            break;
#if !defined(CONFIG_LCD_IS_EPD)
        case cfg_screen_screen_brightness:
            if(g_rtc_config.screen.screen_brightness != val) {
                FUNC_ENTRY_ARGSD(TAG, "Screen brightness changed to %u", val);
                g_rtc_config.screen.screen_brightness = val;
                changed = index;
            }
            break;
#endif
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        case cfg_screen_speed_large_font:
            if(g_rtc_config.screen.speed_large_font != val) {
                FUNC_ENTRY_ARGSD(TAG, "Speed large font changed to %u", val);
                g_rtc_config.screen.speed_large_font = val;
                changed = index;
            }
            break;
#endif
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        case cfg_screen_gpio12_screens:
            if(g_rtc_config.screen.gpio12_screens != val) {
                FUNC_ENTRY_ARGSD(TAG, "GPIO12 screens changed to %u", val);
                g_rtc_config.screen.gpio12_screens = val;
                changed = index;
            }
            break;
#endif
        default:
            config_unlock();
            return false;
    }
    
    // Save to NVS for persistence
    if (changed != 255) {
        unified_config_save();
        // Notify all observers of change
        config_observer_notify(SCFG_GROUP_SCREEN, index);
    }
    config_unlock();
    return true;
}

bool config_screen_set_item(size_t index, const char *value) {
    if (!value) {
        return false;
    }
    // Parse string to number (except for ubx_file which needs the string)
    uint16_t val = strtoul(value, 0, 10);
    return config_screen_set_item_impl(index, val);
}

uint8_t config_screen_get_next_value(size_t index) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
    uint8_t new_value = 0, current = 0;
    switch (index) {
        case cfg_screen_speed_field: {
            // Cycle through speed field options
            current = g_rtc_config.screen.speed_field;
            new_value = (current >= config_speed_field_item_count - 1) ? 0 : current + 1;
            break;
        }
        case cfg_screen_board_logo: {
            // Cycle through board logos
            current = g_rtc_config.screen.board_logo;
            new_value = (current >= board_logos_count) ? 1 : current + 1;
            break;
        }
        case cfg_screen_sail_logo: {
            current = g_rtc_config.screen.sail_logo;
            // Cycle through sail logos
            new_value = (current >= sail_logos_count) ? 1 : current + 1;
            break;
        }
        case cfg_screen_screen_rotation: {
            int8_t current = g_rtc_config.screen.screen_rotation;
            // Cycle through screen rotations
            new_value = (current >= screen_rotations_count - 1) ? 0 : current + 1;
            break;
        }
        case cfg_screen_bat_view: {
            current = g_rtc_config.screen.bat_view;
            // Cycle through battery view modes
            new_value = (current >= bat_views_count - 1) ? 0 : current + 1;
            break;
        }
#if !defined(CONFIG_LCD_IS_EPD)
        case cfg_screen_screen_brightness:
            current = g_rtc_config.screen.screen_brightness;
            if(current == 100) new_value = 80;
            else if(current == 80) new_value = 60;
            else if(current == 60) new_value = 40;
            else if(current == 40) new_value = 20;
            else if(current == 20) new_value = 5;
            else new_value = 100;
            break;
#endif
        default:
            break;
    }
    return new_value;
}

bool config_screen_set_next_value(size_t index) {
    FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
    if (config_screen_set_item_impl(index, config_screen_get_next_value(index))) {
        // DLOG(TAG, "settings screen change saved");
        return true;
    }
    return false;
}
