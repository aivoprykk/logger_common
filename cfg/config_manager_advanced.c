#include "common_private.h"

static const char *TAG = "config_advanced";

const char *const config_advanced_items[] = {CFG_ADVANCED_ITEM_LIST(STRINGIFY)};
const size_t config_advanced_item_count =
	sizeof(config_advanced_items) / sizeof(config_advanced_items[0]);

#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
const char *const seconds_list[] = {"1 sec", "2 sec", "3 sec", "4 sec",
									"5 sec"};
#endif

bool get_advanced_item_descriptions(size_t index, struct strbf_s *sb) {
	switch (index) {
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
		strbf_puts(sb,
				   "Maximum speed in m/s for displaying statistical screens.");
		break;
	case cfg_advanced_experimental_features:
		strbf_puts(sb, "Enable or disable experimental features.");
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

uint8_t get_advanced_item_values(size_t index, struct strbf_s *sb) {
	switch (index) {
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
	case cfg_advanced_stat_screens_time:
		add_values_array(sb, seconds_list, SECONDS_LIST_COUNT, 1);
		return 1;
#endif
	default:
		return 0;
	}
}

bool config_advanced_value_str(size_t index, struct strbf_s *sb,
							   uint8_t *type) {
	if (!sb || !type)
		return false;
	switch (index) {
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
	case cfg_advanced_experimental_features: // experimental_features
		strbf_putul(sb, g_rtc_config.advanced.experimental_features);
		*type = SCONFIG_ITEM_TYPE_BOOL;
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
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t
// value
bool config_advanced_get_item(size_t index, config_item_info_t *info) {
	if (!info)
		return false;
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
		info->min = 0;
		info->max = 365;
		break;
	case cfg_advanced_stat_speed: // stat_screen_speed
		info->value = g_rtc_config.advanced.stat_screen_speed;
		info->desc = "Show stat screen speed up to (m/s)";
		info->min = 0;
		info->max = 5;
		break;
	case cfg_advanced_experimental_features: // experimental_features
		info->value = g_rtc_config.advanced.experimental_features ? 1 : 0;
		info->desc = "Enable experimental features";
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

static bool config_advanced_set_item_impl(size_t index, void *_val) {
	FUNC_ENTRY_ARGSD(TAG, "index=%zu", index);
	if (!config_lock(-1)) {
		ELOG(TAG, "Failed to acquire config lock");
		return false;
	}

	// O(1) lookup: index -> enum via advanced_group_items array
	// O(1) lookup: enum -> RTC field via switch
	uint8_t changed = 255;
	uint16_t val = (uint16_t)(uintptr_t)_val;
	switch (index) {
#if defined(CFG_ADVANCED_INCLUDE_PRIVATE_ITEMS)
	case cfg_advanced_screen_move_offset:
		if (g_rtc_config.advanced.screen_move_offset != val) {
			FUNC_ENTRY_ARGSD(TAG, "Screen move offset changed to %" PRIu8 "",
							 val);
			g_rtc_config.advanced.screen_move_offset = val;
			changed = index;
		}
		break;
	case cfg_advanced_speed_field_count:
		if (g_rtc_config.advanced.speed_field_count != val) {
			FUNC_ENTRY_ARGSD(TAG, "Speed field count changed to %" PRIu8 "",
							 val);
			g_rtc_config.advanced.speed_field_count = val;
			changed = index;
		}
		break;
#endif
	case cfg_advanced_archive_days:
		if (g_rtc_config.advanced.archive_days != val) {
			FUNC_ENTRY_ARGSD(TAG, "Archive days changed to %" PRIu16 "", val);
			g_rtc_config.advanced.archive_days = val;
			changed = index;
		}
		break;
	case cfg_advanced_stat_speed:
		if (g_rtc_config.advanced.stat_screen_speed != val) {
			FUNC_ENTRY_ARGSD(TAG, "Stat screen speed changed to %" PRIu16 "",
							 val);
			g_rtc_config.advanced.stat_screen_speed = val;
			changed = index;
		}
		break;
	case cfg_advanced_hostname:
		if (set_string_from_json(&g_rtc_config.advanced.hostname[0],
								 (const char *)_val)) {
			changed = cfg_advanced_hostname;
		}
		break;
	case cfg_advanced_experimental_features:
		if (g_rtc_config.advanced.experimental_features != val) {
			FUNC_ENTRY_ARGSD(
				TAG, "Experimental features changed to %" PRIu16 "", val);
			g_rtc_config.advanced.experimental_features = val;
			changed = index;
		}
		break;
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
	case cfg_advanced_stat_screens_time:
		if (g_rtc_config.advanced.stat_screens_time != val) {
			FUNC_ENTRY_ARGSD(TAG, "Stat screens time changed to %" PRIu16 "",
							 val);
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
		uint8_t handle = config_advanced_handle();
		unified_config_save_by_submodule(handle);
		// Notify all observers of change
		config_observer_notify(handle, index);
	}
	config_unlock();
	return true;
}

bool config_advanced_set_item(size_t index, const char *value) {
	if (!value) {
		return false;
	}
	if (index == cfg_advanced_hostname) {
		return config_advanced_set_item_impl(index, (void *)value);
	}
	uint16_t val = strtoul(value, 0, 10);
	return config_advanced_set_item_impl(index, (void *)&val);
}

// Unified functions for advanced group
bool config_get_advanced_item(size_t index, config_item_info_t *info) {
	return config_advanced_get_item(index, info);
}

static config_module_registry_t advanced_module = {
	.name = "advanced",
	// .group_id = SCFG_GROUP_ADVANCED,
	.flags = CONFIG_MODULE_FLAG_DEFAULT_HIDDEN,
	.parent_name = NULL,
	.nvs_key = NVS_KEY_ADVANCED,
	.data_ptr = &g_rtc_config.advanced,
	.data_size = sizeof(cfg_advanced_t),
	.migrate_func = NULL,
	.ops =
		{
			.get_item = config_advanced_get_item,
			.set_item = config_advanced_set_item,
			.get_values = get_advanced_item_values,
			.get_descriptions = get_advanced_item_descriptions,
			.value_str = config_advanced_value_str,
			.get_next_value = NULL,
			.set_next_value = NULL,
			.get_next_cycled_idx = NULL,
			.item_count = config_advanced_item_count,
			.item_names = config_advanced_items,
		},
	.handle = CONFIG_MODULE_HANDLE_INVALID};

void config_advanced_register(void) {
	advanced_module.handle = config_registry_register(&advanced_module);
	if (advanced_module.handle == CONFIG_MODULE_HANDLE_INVALID) {
		ELOG(TAG, "Failed to register advanced module");
	}
}

#if defined(CONFIG_LOGGER_COMMON_ENABLE_CFG_AUTOLOAD)
__attribute__((constructor)) static void config_advanced_auto_register(void) {
	config_advanced_register();
}
#endif

config_module_handle_t config_advanced_handle(void) {
	return advanced_module.handle;
}
