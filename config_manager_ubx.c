#include "config.h"
#include "unified_config.h"
#include "esp_log.h"

static const char *TAG = "config_ubx";
#define L(x) x,
static const uint8_t sample_rate_values[] = {SAMPLE_RATE_VAL_LIST(L)};

const char * const config_ubx_items[] = { CFG_UBX_ITEM_LIST(STRINGIFY) };
const size_t config_ubx_item_count = sizeof(config_ubx_items) / sizeof(config_ubx_items[0]);

const char * const dynamic_models[] = {DYNAMIC_MODEL_ITEM_LIST(STRINGIFY)};
const size_t dynamic_models_items_count = sizeof(dynamic_models) / sizeof(dynamic_models[0]);
const char * const gnss_desc[] = {GNSS_DESC_ITEM_LIST(STRINGIFY)};
const size_t gnss_desc_items_count = sizeof(gnss_desc) / sizeof(gnss_desc[0]);
const uint8_t gnss_desc_val[] = {GNSS_DESC_VAL_LIST(L)};

const char * const sample_rates[] = {SAMPLE_RATE_ITEM_LIST(STRINGIFY)};
const size_t sample_rates_items_count = sizeof(sample_rates) / sizeof(sample_rates[0]);
#undef L
#define L(x,y) y,
const uint8_t dynamic_model_values[] = {UBX_NAV_MODE_LIST(L)};
#undef L

bool get_ubx_item_descriptions(size_t index, struct strbf_s *sb) {
    switch(index) {
        case cfg_ubx_ubx_output_rate:
            // return "GPS sample rate in Hz (1, 5, or 10)";
            strbf_puts(sb, "UBX sample rate (output rate) in Hz.");
            break;
        case cfg_ubx_ubx_nav_mode:
            // return "UBX navigation mode (dynamic model)";
            strbf_puts(sb, "UBX navigation mode (dynamic model).");
            break;
        case cfg_ubx_ubx_gnss:
            // return "GNSS constellation selection";
            strbf_puts(sb, "Enabled GNSS systems bitfield.");
            break;
        default:
            return false;
    }
    return true;
}

bool get_ubx_item_values(size_t index, strbf_t *sb) {
    switch(index) {
        case cfg_ubx_ubx_gnss:
            add_values_array(sb, gnss_desc, &gnss_desc_val[0], gnss_desc_items_count, 0);
            break;
        case cfg_ubx_ubx_output_rate:
            add_values_array(sb, sample_rates, &sample_rate_values[0], sample_rates_items_count, 0);
            break;
        case cfg_ubx_ubx_nav_mode:
            add_values_array(sb, dynamic_models, &dynamic_model_values[0], dynamic_models_items_count, 0);
            break;
        default:
            return false;
    }
    return true;
}

bool config_ubx_value_str(size_t index, strbf_t *sb, uint8_t* type) {
    switch(index) {
        case cfg_ubx_ubx_output_rate: // ubx_output_rate (sample_rate)
            if(sb)
                strbf_putl(sb, g_rtc_config.ubx.output_rate);
            *type = SCONFIG_ITEM_TYPE_INT8;
            break;
        case cfg_ubx_ubx_nav_mode: // ubx_nav_mode (dynamic_model)
            if(sb)
                strbf_putl(sb, g_rtc_config.ubx.nav_mode);
            *type = SCONFIG_ITEM_TYPE_INT8;
            break;
        case cfg_ubx_ubx_gnss: // ubx_gnss
            if(sb)
                strbf_putul(sb, g_rtc_config.ubx.gnss);
            *type = SCONFIG_ITEM_TYPE_UINT8;
            break;
#if defined(CFG_UBX_INCLUDE_PRIVATE_ITEMS)
        case cfg_ubx_ubx_msgout_sat: // ubx_msgout_sat
            if(sb)
                strbf_putul(sb, g_rtc_config.ubx.msgout_sat);
            *type = SCONFIG_ITEM_TYPE_BOOL;
            break;
#endif
        default:
            return false;
    }
    return true;
}

// UBX config group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t value
bool config_ubx_get_item(size_t index, config_item_info_t *info) {
    ESP_LOGI(TAG, "[%s] index: %u", __func__, index);
    if (!info) return false;
    info->name = config_ubx_items[index];
    info->pos = index;
    // printf("get cfg item: %s at %u.\n", info->name, index);
    // Handle different types and set descriptions
    switch (index) {
        case 0: // ubx_output_rate (sample_rate)
            info->value = g_rtc_config.ubx.output_rate;
            uint8_t rate_index;
            switch (info->value) {
                case 1: rate_index = 0; break;
                case 2: rate_index = 1; break;
                case 5: rate_index = 2; break;
                case 10: rate_index = 3; break;
                case 16: rate_index = 4; break;
                case 20: rate_index = 5; break;
                default: rate_index = 0; break; // Default to 1Hz
            }
            if (rate_index < sample_rates_items_count) {
                info->desc = sample_rates[rate_index];
            } else {
                info->desc = not_set;
            }
            break;
        
        case cfg_ubx_ubx_gnss: // gnss
            info->value =g_rtc_config.ubx.gnss;
            for (size_t i = 0; i < gnss_desc_items_count; i++) {
                if (info->value == gnss_desc_val[i]) {
                    info->desc = gnss_desc[i];
                    break;
                }
            }
            if (!info->desc) {
                info->desc = not_set;
            }
            break;
        case cfg_ubx_ubx_nav_mode: // dynamic_model
            info->value = g_rtc_config.ubx.nav_mode;
            uint8_t model_index;
            switch (info->value) {
                case 0: model_index = 0; break; // Portable
                case 3: model_index = 1; break; // Pedestrian
                case 4: model_index = 2; break; // Automotive
                case 5: model_index = 3; break; // Sea
                default: model_index = 1; break; // Default to Pedestrian
            }
            if (model_index < dynamic_models_items_count) {
                info->desc = dynamic_models[model_index];
            } else {
                info->desc = not_set;
            }
            break;
#if defined(CFG_UBX_INCLUDE_PRIVATE_ITEMS)
        case cfg_ubx_ubx_msgout_sat: // ubx_msgout_sat
            info->value = g_rtc_config.ubx.msgout_sat;
            info->desc = "UBX NAV-SAT message output enable";
            break;
#endif
        default:
            info->desc = not_set;
            break;
    }

    return true;
}

// Internal implementation that takes numeric value directly
static bool config_ubx_set_item_impl(size_t index, uint16_t val) {
    if (!config_lock(-1)) {
        ESP_LOGE(TAG, "Failed to acquire config lock");
        return false;
    }
    
    uint8_t changed = 255;
    int val2;
    switch (index) {
        case 0: {// ubx_output_rate (sample_rate)
            if(g_rtc_config.ubx.output_rate != val) {
                ESP_LOGI(TAG, "Sample rate config changed from %d to %d, reinit UBX.", g_rtc_config.ubx.output_rate, val);
                g_rtc_config.ubx.output_rate = val;
                changed = cfg_ubx_ubx_output_rate;
            }
            break;
        }
        case cfg_ubx_ubx_gnss: // gnss
            if(g_rtc_config.ubx.gnss != val) {
                ESP_LOGI(TAG, "GNSS config changed from %u to %u, reinit UBX.", g_rtc_config.ubx.gnss, val);
                g_rtc_config.ubx.gnss = val;
                changed = cfg_ubx_ubx_gnss;
            }
            break;
        case cfg_ubx_ubx_nav_mode: // dynamic_model
            if(g_rtc_config.ubx.nav_mode != val) {
                ESP_LOGI(TAG, "Dynamic model changed from %d to %d, reinit UBX.", g_rtc_config.ubx.nav_mode, val);
                g_rtc_config.ubx.nav_mode = val;
                changed = cfg_ubx_ubx_nav_mode;
            }
            break;
#if defined(CFG_UBX_INCLUDE_PRIVATE_ITEMS)
        case cfg_ubx_ubx_msgout_sat: // ubx_msgout_sat
            if(g_rtc_config.ubx.msgout_sat != val) {
                ESP_LOGI(TAG, "UBX NAV-SAT message output enable changed from %d to %d, reinit UBX.", g_rtc_config.ubx.msgout_sat, val);
                g_rtc_config.ubx.msgout_sat = val;
                changed = cfg_ubx_ubx_msgout_sat;
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
        config_observer_notify(SCFG_GROUP_UBX, index);
    }
    config_unlock();
    return true;
}

bool config_ubx_set_item(size_t index, const char *value) {
    ESP_LOGI(TAG, "[%s] index: %lu value: %s", __func__, index, value);
    if (!value) {
        return false;
    }
    uint16_t val = strtoul(value, 0, 10);
    return config_ubx_set_item_impl(index, val);
}

// Cycle to next value (for UI button presses)
uint8_t config_ubx_get_next_value(size_t index) {
    ESP_LOGI(TAG, "[%s] index: %lu", __func__, index);
    uint8_t new_value = 0;
    switch (index) {
        case cfg_ubx_ubx_output_rate: {
            // Cycle through sample rates: 1->2->5->10->16->20->1
            uint8_t current = g_rtc_config.ubx.output_rate;
            switch (current) {
                case 1:  new_value = 2; break;
                case 2:  new_value = 5; break;
                case 5:  new_value = 10; break;
                case 10: new_value = 16; break;
                case 16: new_value = 20; break;
                case 20: new_value = 1; break;
                default: new_value = 5; break; // Default to 5Hz
            }
            break;
        }
        case cfg_ubx_ubx_gnss: {
            // Cycle through GNSS combinations: 111->107->103->47->99->43->39->111
            uint8_t current = g_rtc_config.ubx.gnss;
            switch (current) {
                case 111: new_value = 107; break; // G+E+B+R -> G+B+R
                case 107: new_value = 103; break; // G+B+R -> G+E+R
                case 103: new_value = 47; break;  // G+E+R -> G+E+B
                case 47:  new_value = 99; break;  // G+E+B -> G+R
                case 99:  new_value = 43; break;  // G+R -> G+B
                case 43:  new_value = 39; break;  // G+B -> G+E
                case 39:  new_value = 111; break; // G+E -> G+E+B+R
                default:  new_value = 111; break; // Default to all systems
            }
            break;
        }
        case cfg_ubx_ubx_nav_mode: {
            // Cycle through nav modes: Portable->Pedestrian->Automotive->Sea->Portable
            uint8_t current = g_rtc_config.ubx.nav_mode;
            switch (current) {
                case 0: new_value = 3; break; // Portable -> Pedestrian
                case 3: new_value = 4; break; // Pedestrian -> Automotive
                case 4: new_value = 5; break; // Automotive -> Sea
                case 5: new_value = 0; break; // Sea -> Portable
                default: new_value = 3; break; // Default to Pedestrian
            }
            break;
        }
        default:
            break;
    }
    return new_value;
}

bool config_ubx_set_next_value(size_t index) {
    ESP_LOGI(TAG, "[%s] index: %lu", __func__, index);
    uint8_t new_value = 0;
    if(config_ubx_set_item_impl(index, config_ubx_get_next_value(index))) {
        return true;
    }
    return false;
}

struct m_config_item_s * get_ubx_cfg_item(int num, struct m_config_item_s *item) {
    // printf("num:%d", num);
    if(!item) return 0;
    if(num < 0 || num >= config_ubx_item_count) num = 0;
    config_item_info_t info;
    if (config_ubx_get_item(num, &info)) {
        item->name = info.name;
        item->pos = num;
        item->value = info.value;
        item->desc = info.desc;
    } else {
        item->name = "unknown";
        item->desc = not_set;
        item->pos = num;
        item->value = 0;
    }
    // esp_event_post(GPS_LOG_EVENT, GPS_LOG_EVENT_CFG_GET, &item, sizeof(item), timeout_max);
    return item;
}

// int set_ubx_cfg_item(int num, bool skip_done_msg) {
//     printf("num:%d", num);
//     if(num < 0 || num >= config_ubx_item_count) num = 0;
//     return config_ubx_cycle_item(num) ? num : -1;
    // Check if this item affects UBX (nav_sat message enable)
    // if (num == gps_cfg_log_ubx_nav_sat && changed) {
    //     needs_ubx_update = true;
    // }

    // }
        
    // // Apply UBX hardware updates if needed
    // if (needs_ubx_update && gps && gps->ubx_device && gps->ubx_device->initialized) {
    //     config_item_info_t ubx_gnss, ubx_rate, ubx_nav_mode;
        
    //     if (num == gps_cfg_dynamic_model) {
    //         // Update nav mode only
    //         if (config_ubx_get_item(cfg_ubx_ubx_nav_mode, &ubx_nav_mode)) {
    //             ubx_set_nav_mode(gps->ubx_device, ubx_nav_mode.value);
    //         }
    //     } else {
    //         // Update GNSS and rate
    //         if (config_ubx_get_item(cfg_ubx_ubx_gnss, &ubx_gnss) && 
    //             config_ubx_get_item(cfg_ubx_ubx_output_rate, &ubx_rate)) {
    //             ubx_set_ggnss_and_rate(gps->ubx_device, ubx_gnss.value, ubx_rate.value);
    //             ILOG(TAG, "[%s] set gnss: %d, rate: %d", __func__, ubx_gnss.value, ubx_rate.value);
    //         }
    //     }
    // }
// }
