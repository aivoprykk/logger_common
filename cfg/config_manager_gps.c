#include "common_private.h"

static const char *TAG = "config_gps";

const int8_t timezone_values[] = {0, 1, 2, 3};
static const char *const str_on = "on";
static const char *const str_off = "off";

const char *const config_gps_items[] = {CFG_GPS_ITEM_LIST(STRINGIFY)};
const size_t config_gps_item_count =
	sizeof(config_gps_items) / sizeof(config_gps_items[0]);

// GPS specific description arrays
const char *const speed_units[] = {SPEED_UNIT_VAL_LIST(STRINGIFY)};
const size_t speed_units_items_count =
	sizeof(speed_units) / sizeof(speed_units[0]);

const char *const file_date_time_items[] = {
	FILE_DATE_TIME_ITEM_LIST(STRINGIFY)};
const size_t file_date_time_items_count =
	sizeof(file_date_time_items) / sizeof(file_date_time_items[0]);

const char *const timezone_items[] = {TIMEZONE_ITEM_LIST(STRINGIFY)};
const size_t timezone_items_count =
	sizeof(timezone_items) / sizeof(timezone_items[0]);

// Define the extern arrays that are used by gps_user_cfg.c
const char *const gps_stat_screen_items[] = {STAT_SCREEN_ITEM_LIST(STRINGIFY)};
const size_t gps_stat_screen_item_count =
	sizeof(gps_stat_screen_items) / sizeof(gps_stat_screen_items[0]);

const char *const log_format_items[] = {LOG_FORMAT_ITEM_LIST(STRINGIFY)};
const size_t log_format_items_count =
	sizeof(log_format_items) / sizeof(log_format_items[0]);

static uint8_t get_log_format_mask(uint8_t idx) {
	switch (idx) {
	case log_format_sbp:
		return (uint8_t)(1U << 1);
	case log_format_ubx:
		return (uint8_t)(1U << 2);
	case log_format_gpx:
		return (uint8_t)(1U << 3);
#if defined(GPS_LOG_HAS_OAO)
	case log_format_oao:
		return (uint8_t)(1U << 4);
#endif
#if defined(GPS_LOG_HAS_GPY)
	case log_format_gpy:
		return (uint8_t)(1U << 5);
#endif
	default:
		return (uint8_t)(1U << 1);
	}
}

static uint8_t get_all_log_format_masks(void) {
	uint8_t mask = (uint8_t)((1U << 1) | (1U << 2) | (1U << 3));
#if defined(GPS_LOG_HAS_OAO)
	mask |= (uint8_t)(1U << 4);
#endif
#if defined(GPS_LOG_HAS_GPY)
	mask |= (uint8_t)(1U << 5);
#endif
	return mask;
}

bool get_gps_item_descriptions(size_t index, struct strbf_s *sb) {
	switch (index) {
	case cfg_gps_speed_unit:
		// return "speed unit: 0=m/s, 1=km/h, 2=knots";
		strbf_puts(sb, "Speed unit for display and logging. 0 = m/s, 1 = km/h, "
					   "2 = knots.");
		break;
	case cfg_gps_timezone:
		// return "timezone difference with UTC in hours";
		strbf_puts(sb, "Timezone offset in hours from UTC. E.g., for Belgium "
					   "use 1 (CET) or 2 (CEST).");
		break;
	case cfg_gps_file_date_time:
		// return "file naming type: MAC address or datetime";
		strbf_puts(sb, "Method for naming GPS log files. mac_index, date_time "
					   "or date_time_name.");
		break;
	case cfg_gps_stat_screens:
		strbf_puts(sb, "Enable/disable GPS statistics screens on the display.");
		break;
	case cfg_gps_log_txt:
		strbf_puts(sb, "Enable/disable logging metainfo.");
		break;
	case cfg_gps_log_ubx_nav_sat:
		// return "enable/disable logging for this format";
		strbf_puts(sb, "Enable/disable logging for sat messages.");
		break;
	case cfg_gps_log_format:
		strbf_puts(sb, "GPS log format selection.");
		break;
	case cfg_gps_ubx_file:
		strbf_puts(sb, "Base filename for log files.");
		break;
	default:
		return false;
	}
	return true;
}

uint8_t get_gps_item_values(size_t index, struct strbf_s *sb) {
	switch (index) {
	case cfg_gps_speed_unit:
		add_values_array(sb, speed_units, 0, speed_units_items_count, 0);
		return 1;
	case cfg_gps_file_date_time:
		add_values_array(sb, file_date_time_items, 0,
						 file_date_time_items_count, 0);
		return 1;
	case cfg_gps_timezone:
		add_values_array(sb, timezone_items, 0, timezone_items_count, 0);
		return 1;
	case cfg_gps_log_format:
		add_values_array(sb, log_format_items, 0, log_format_items_count, 0);
		return 1;
	case cfg_gps_stat_screens:
		add_toggles_array(sb, gps_stat_screen_items, 0,
						  gps_stat_screen_item_count, 0);
		return 2;
	default:
		return 0;
	}
}

static uint8_t get_log_format_index(void) {
	uint8_t idx = g_rtc_config.gps.log_enables.bits.log_sbp	  ? log_format_sbp
				  : g_rtc_config.gps.log_enables.bits.log_ubx ? log_format_ubx
				  : g_rtc_config.gps.log_enables.bits.log_gpx ? log_format_gpx

#if defined(GPS_LOG_HAS_OAO)
				  : g_rtc_config.gps.log_enables.bits.log_oao ? log_format_oao
#endif
#if defined(GPS_LOG_HAS_GPY)
				  : g_rtc_config.gps.log_enables.bits.log_gpy ? log_format_gpy
#endif
				  : log_format_sbp; // Default to 0 if none set
	return idx;
}

bool config_gps_value_str(size_t index, struct strbf_s *sb, uint8_t *type) {
	if (!sb || !type)
		return false;
	switch (index) {
	case cfg_gps_timezone: // timezone
		strbf_putd(sb, (int)g_rtc_config.gps.timezone, 1, 0);
		*type = SCONFIG_ITEM_TYPE_INT8;
		break;
	case cfg_gps_speed_unit: // speed_unit
		strbf_putul(sb, g_rtc_config.gps.speed_unit);
		*type = SCONFIG_ITEM_TYPE_UINT8;
		break;
	case cfg_gps_stat_screens: // stat_screens
		strbf_putul(sb, g_rtc_config.gps.stat_screens.value);
		*type = SCONFIG_ITEM_TYPE_UINT16;
		break;
	case cfg_gps_log_txt: // log_txt
		strbf_putul(sb, g_rtc_config.gps.log_enables.bits.log_txt ? 1 : 0);
		*type = SCONFIG_ITEM_TYPE_BOOL;
		break;
	case cfg_gps_log_format: // log_format
		uint8_t idx = get_log_format_index();
		strbf_putul(sb, idx);
		*type = SCONFIG_ITEM_TYPE_UINT8;
		break;
	case cfg_gps_log_ubx_nav_sat: // log_ubx_nav_sat
		strbf_putul(sb,
					g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat ? 1 : 0);
		*type = SCONFIG_ITEM_TYPE_BOOL;
		break;
	case cfg_gps_file_date_time: // file_date_time
		strbf_putul(sb, g_rtc_config.gps.file_date_time);
		*type = SCONFIG_ITEM_TYPE_UINT8;
		break;
	case cfg_gps_ubx_file: // ubx_file
		insert_json_string_value(sb, g_rtc_config.gps.ubx_file);
		*type = SCONFIG_ITEM_TYPE_STRING;
		break;
	default:
		return false;
	}
	return true;
}

// GPS group implementation
// Note: index is group-relative (0, 1, 2, ...) not global sconfig_item_enum_t
// value
bool config_gps_get_item(size_t index, config_item_info_t *info) {
	// FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
	if (!info)
		return false;
	info->name = config_gps_items[index];
	info->pos = index;
	// printf("get cfg item: %s at %u.\n", info->name, index);
	// Handle different types and set descriptions
	switch (index) {
	case cfg_gps_timezone: // timezone
	{
		int8_t value = (int8_t)g_rtc_config.gps.timezone;
		info->value = value;
		for (uint8_t i = 0; i < timezone_items_count; i++) {
			if (value == timezone_values[i]) {
				info->desc = timezone_items[i];
				break;
			}
		}
		if (!info->desc) {
			info->desc = not_set;
		}
	} break;
	case cfg_gps_speed_unit: // speed_unit
		info->value = g_rtc_config.gps.speed_unit;
		if (info->value < speed_units_items_count) {
			info->desc = speed_units[info->value];
		} else {
			info->desc = not_set;
		}
		break;
	case cfg_gps_stat_screens: // stat_screens
		info->value = g_rtc_config.gps.stat_screens.value;
		info->desc = not_set; // No specific description
		break;
	case cfg_gps_log_txt: // log_txt
		info->value = g_rtc_config.gps.log_enables.bits.log_txt ? 1 : 0;
		info->desc = info->value ? str_on : str_off;
		break;
	case cfg_gps_log_format: // log_format
		uint8_t idx = get_log_format_index();
		info->value = idx;
		if (idx < log_format_items_count) {
			info->desc = log_format_items[idx];
		} else {
			info->desc = not_set;
		}
		break;
	case cfg_gps_log_ubx_nav_sat: // log_ubx_nav_sat
		info->value = g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat ? 1 : 0;
		info->desc = info->value ? str_on : str_off;
		break;
	case cfg_gps_file_date_time: // file_date_time
		info->value = g_rtc_config.gps.file_date_time;
		if (info->value < file_date_time_items_count) {
			info->desc = file_date_time_items[info->value];
		} else {
			info->desc = not_set;
		}
		break;
	case cfg_gps_ubx_file: // ubx_file
		info->value = (uintptr_t)&g_rtc_config.gps.ubx_file[0];
		info->desc = not_set;
		break;
	default:
		info->desc = not_set;
		break;
	}

	return true;
}

// Internal implementation that takes numeric value directly
static bool config_gps_set_item_impl(size_t index, uint16_t val,
									 const char *str_value) {
	FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
	if (!config_lock(-1)) {
		ELOG(TAG, "Failed to acquire config lock");
		return false;
	}

	uint8_t changed = 255;
	switch (index) {
	case cfg_gps_timezone: { // timezone
		int8_t val2 = val < timezone_items_count ? timezone_values[val] : 0;
		if (g_rtc_config.gps.timezone != val2) {
			FUNC_ENTRY_ARGSD(
				TAG, "Timezone config changed from %" PRId8 " to %" PRId8 ".",
				g_rtc_config.gps.timezone, val2);
			g_rtc_config.gps.timezone = val2;
			changed = cfg_gps_timezone;
		}
		break;
	}
	case cfg_gps_speed_unit: { // speed_unit
		int val2 = val < speed_units_items_count ? val : 0;
		if (g_rtc_config.gps.speed_unit != val2) {
			FUNC_ENTRY_ARGSD(TAG, "Speed unit config changed from %d to %d.",
							 g_rtc_config.gps.speed_unit, val2);
			g_rtc_config.gps.speed_unit = val2;
			changed = cfg_gps_speed_unit;
		}
		break;
	}
	case cfg_gps_stat_screens: { // stat_screens
		FUNC_ENTRY_ARGSD(TAG,
						 "Stat screens config change from %u to %" PRIu16 ".",
						 g_rtc_config.gps.stat_screens.value, val);
		if (g_rtc_config.gps.stat_screens.value != val) {
			g_rtc_config.gps.stat_screens.value =
				!val ? 1 : val; // Ensure at least one screen is enabled
			changed = cfg_gps_stat_screens;
		}
		break;
	}
	case cfg_gps_log_txt: { // log_txt
		if (g_rtc_config.gps.log_enables.bits.log_txt != val) {
			FUNC_ENTRY_ARGSD(TAG, "Log TXT config changed from %d to %u.",
							 g_rtc_config.gps.log_enables.bits.log_txt, val);
			g_rtc_config.gps.log_enables.bits.log_txt = val ? 1 : 0;
			changed = cfg_gps_log_txt;
		}
		break;
	}
	case cfg_gps_log_format: { // log_format
		uint8_t idx = val < log_format_items_count ? val : log_format_sbp;
		uint8_t cur_idx = get_log_format_index();
		if (idx != cur_idx) {
			FUNC_ENTRY_ARGSD(TAG, "Log format config changed from %d to %u.",
							 cur_idx, idx);
			g_rtc_config.gps.log_enables.value &= ~get_all_log_format_masks();
			// set selected format bit
			g_rtc_config.gps.log_enables.value =
				(g_rtc_config.gps.log_enables.value & 0x01)
				| get_log_format_mask(idx);
			changed = cfg_gps_log_format;
		}
		break;
	}
	case cfg_gps_log_ubx_nav_sat: { // log_ubx_nav_sat
		if (g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat != val) {
			FUNC_ENTRY_ARGSD(
				TAG, "Log UBX NAV SAT config changed from %d to %u.",
				g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat, val);
			g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat = val ? 1 : 0;
			changed = cfg_gps_log_ubx_nav_sat;
		}
		break;
	}
	case cfg_gps_file_date_time: { // file_date_time
		if (g_rtc_config.gps.file_date_time != val) {
			FUNC_ENTRY_ARGSD(TAG,
							 "File date time config changed from %u to %u.",
							 g_rtc_config.gps.file_date_time, val);
			g_rtc_config.gps.file_date_time = val;
			changed = cfg_gps_file_date_time;
		}
		break;
	}
	case cfg_gps_ubx_file: // ubx_file (requires string)
		if (str_value && set_string_from_json(&g_rtc_config.gps.ubx_file[0],
											  (char *)str_value)) {
			changed = cfg_gps_ubx_file;
		}
		break;
	default:
		config_unlock();
		return false;
	}
	if (changed != 255) {
		uint8_t handle = config_gps_handle();
		unified_config_save_by_submodule(handle);
		// Notify all observers of change
		config_observer_notify(handle, index);
	}
	config_unlock();
	return true;
}

bool config_gps_set_item(size_t index, const char *value) {
	if (!value) {
		return false;
	}
	// Parse string to number (except for ubx_file which needs the string)
	uint16_t val = (index == cfg_gps_ubx_file) ? 0 : strtoul(value, 0, 10);
	return config_gps_set_item_impl(index, val, value);
}

// Cycle to next value (for UI button presses)
uint16_t config_gps_get_next_value(size_t index) {
	// FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
	int16_t new_value = 0, current = 0;

	switch (index) {
	case cfg_gps_timezone: {
		// Cycle: 0 -> 1 -> 2 -> 3 -> 0
		current = g_rtc_config.gps.timezone;
		if (current == 0)
			new_value = 1;
		else if (current == 1)
			new_value = 2;
		else if (current == 2)
			new_value = 3;
		else if (current == 3)
			new_value = 4;
		else
			new_value = 0;
		break;
	}
	case cfg_gps_speed_unit: {
		// Cycle: 0 (m/s) -> 1 (km/h) -> 2 (knot) -> 3 (mph) -> 0
		current = g_rtc_config.gps.speed_unit;
		if (current == 0)
			new_value = 1;
		else if (current == 1)
			new_value = 2;
		else if (current == 2)
			new_value = 3;
		else
			new_value = 0;
		break;
	}
	case cfg_gps_log_txt:
		current = g_rtc_config.gps.log_enables.bits.log_txt ? 1 : 0;
		new_value = !current;
		break;
	case cfg_gps_log_format: {
		uint8_t cur_idx = get_log_format_index();
		uint8_t next_idx = cur_idx + 1;
		if (next_idx >= log_format_items_count) {
			next_idx = 0; // Wrap around
		}
		new_value = next_idx;
	} break;
	case cfg_gps_log_ubx_nav_sat: {
		// Toggle boolean: 0 -> 1 -> 0
		uint8_t current =
			g_rtc_config.gps.log_enables.bits.log_ubx_nav_sat ? 1 : 0;
		new_value = !current;
		break;
	}
	case cfg_gps_file_date_time: {
		// Cycle: 0 -> 1 -> 2 -> 0
		current = g_rtc_config.gps.file_date_time;
		if (current == 0)
			new_value = 1;
		else if (current == 1)
			new_value = 2;
		else
			new_value = 0;
		break;
	}
	default:
		break;
	}
	return new_value;
}

bool config_gps_set_next_value(size_t index) {
	FUNC_ENTRY_ARGSD(TAG, "index: %u", index);
	if (config_gps_set_item_impl(index, config_gps_get_next_value(index),
								 NULL)) {
		// Special logic: if switching to 0 (mac_index), ensure log_txt is
		// enabled
		if (index == cfg_gps_file_date_time &&
			g_rtc_config.gps.file_date_time == 0) {
			if (!g_rtc_config.gps.log_enables.bits.log_txt)
				config_gps_set_item_impl(cfg_gps_log_txt, 1, NULL);
		}
		return true;
	}
	return false;
}

bool config_stat_screen_get_item(int num, config_item_info_t *item) {
	// FUNC_ENTRY_ARGSD(TAG, "index:%d", num);
	if (!item)
		return false;
	if (num < 0 && num >= gps_stat_screen_item_count)
		num = 0;
	item->value = GETBIT(g_rtc_config.gps.stat_screens.value, num);
	item->name = gps_stat_screen_items[num];
	item->pos = num;
	item->desc = item->value ? str_on : str_off;
	// esp_event_post(GPS_LOG_EVENT, GPS_LOG_EVENT_CFG_GET, &num, sizeof(num),
	// timeout_max);
	return true;
}

struct m_config_item_s *get_gps_cfg_item(int num,
										 struct m_config_item_s *item) {
	if (!item)
		return 0;
	FUNC_ENTRY_ARGSD(TAG, "index:%d", num);
	if (num < 0 || num >= config_gps_item_count)
		num = 0;
	config_item_info_t info;
	if (config_gps_get_item(num, &info)) {
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
	// esp_event_post(GPS_LOG_EVENT, GPS_LOG_EVENT_CFG_GET, &item, sizeof(item),
	// timeout_max);
	return item;
}

uint16_t config_stat_screen_get_next_value(int num) {
	// printf("num:%d", num);
	if (num >= gps_stat_screen_item_count)
		return 0;
	uint16_t val = g_rtc_config.gps.stat_screens.value;
	if (num >= 0 && num < gps_stat_screen_item_count) {
		val ^= (BIT(num));
	}
	FUNC_ENTRY_ARGSD(TAG, "index:%d next value: %u", num, val);
	return val;
}

bool config_stat_screen_set_next_value(int num) {
	FUNC_ENTRY_ARGSD(TAG, "index:%d", num);
	// printf("num:%d", num);
	return config_gps_set_item_impl(
		cfg_gps_stat_screens, config_stat_screen_get_next_value(num), NULL);
}

// Cycled forward: if idx >= count -> return 0; else return next enabled index
// after idx, wrapping. Falls back to 0 if no enabled bits are present.
int config_stat_screen_get_next_cycled(int idx) {
	size_t count = gps_stat_screen_item_count;
	if (count == 0)
		return -1;
	if (idx >= (int)count)
		idx = -1;

	// Search from the next position, with wrap-around
	int start = idx;
	for (size_t step = 1; step <= count; step++) {
		int next = start + (int)step;
		if (next >= (int)count)
			next -= (int)count;
		if (GETBIT(g_rtc_config.gps.stat_screens.value, next)) {
			FUNC_ENTRY_ARGSD(TAG, "next enabled idx: %d, value: %" PRIu16 "",
							 next, g_rtc_config.gps.stat_screens.value);
			return next;
		}
	}
	// None enabled, fall back to 0
	FUNC_ENTRY_ARGSD(TAG, "no enabled stats screens; fallback to 0");
	return 0;
}

// Definition of convert_speed to ensure it's available for linking
float convert_speed(float speed, speed_unit_item_t unit) {
	switch (unit) {
	case kmh:
		return mm_s_to_km_h(speed);
	case knot:
		return mm_s_to_knots(speed);
	case mph:
		return mm_s_to_mph(speed);
	default:
		return mms_to_ms(speed);
	}
}

// ============================================================================
// STAT_SCREENS SUBMODULE - Module ops for LCD/HID access
// ============================================================================

/**
 * @brief Stat screens module get_item - wrapper for config_stat_screen_get_item
 */
static bool stat_screens_get_item(size_t index, config_item_info_t *info) {
	return config_stat_screen_get_item((int)index, info);
}

/**
 * @brief Stat screens module set_item - currently unsupported (use
 * set_next_value)
 */
static bool stat_screens_set_item(size_t index, const char *value) {
	UNUSED_PARAMETER(index);
	UNUSED_PARAMETER(value);
	config_gps_set_item_impl(cfg_gps_stat_screens,
							 config_stat_screen_get_next_value(index), NULL);
	return false;
}

/**
 * @brief Stat screens module get_values - provide on/off toggle options
 */
static uint8_t stat_screens_get_values(size_t index, struct strbf_s *sb) {
	UNUSED_PARAMETER(index);
	get_gps_item_values(cfg_gps_stat_screens,
						sb); // Reuse GPS stat_screens values (on/off)
	// add_toggles_array(sb, (const char *const[]){str_on, str_off}, 0, 2, 0);
	return 2; // SCONFIG_ITEM_TYPE_BOOL
}

/**
 * @brief Stat screens module get_descriptions
 */
static bool stat_screens_get_descriptions(size_t index, struct strbf_s *sb) {
	if (index >= gps_stat_screen_item_count) {
		return false;
	}
	get_gps_item_descriptions(cfg_gps_stat_screens,
							  sb); // Reuse GPS stat_screens description
	return true;
}

/**
 * @brief Stat screens module value_str
 */
static bool stat_screens_value_str(size_t index, struct strbf_s *sb,
								   uint8_t *type) {
	if (!sb || !type || index >= gps_stat_screen_item_count) {
		return false;
	}
	uint8_t val = GETBIT(g_rtc_config.gps.stat_screens.value, (int)index);
	strbf_putul(sb, val);
	*type = SCONFIG_ITEM_TYPE_BOOL;
	return true;
}

/**
 * @brief Stat screens module get_next_value
 */
static uint16_t stat_screens_get_next_value(size_t index) {
	return config_stat_screen_get_next_value((int)index);
}

/**
 * @brief Stat screens module set_next_value - cycle enable/disable
 */
static bool stat_screens_set_next_value(size_t index) {
	return config_stat_screen_set_next_value((int)index);
}

/**
 * @brief Stat screens module get_next_cycled_idx
 */
static int stat_screens_get_next_cycled_idx(int current_idx) {
	return config_stat_screen_get_next_cycled(current_idx);
}

static config_module_registry_t stat_screens_module = {
	.name = "stat_screens",
	.flags = CONFIG_MODULE_FLAG_SUBMODULE | CONFIG_MODULE_FLAG_CYCLE_SCREENS,
	.parent_name = "gps",
	.nvs_key = NULL, // Saved as part of GPS module
	.data_ptr = &g_rtc_config.gps.stat_screens,
	.data_size = sizeof(g_rtc_config.gps.stat_screens),
	.migrate_func = NULL,
	.ops =
		{
			.get_item = stat_screens_get_item,
			.set_item = stat_screens_set_item,
			.get_values = stat_screens_get_values,
			.get_descriptions = stat_screens_get_descriptions,
			.value_str = stat_screens_value_str,
			.get_next_value = stat_screens_get_next_value,
			.set_next_value = stat_screens_set_next_value,
			.get_next_cycled_idx = stat_screens_get_next_cycled_idx,
			.item_count = gps_stat_screen_item_count,
			.item_names = gps_stat_screen_items,
		},
	.handle = CONFIG_MODULE_HANDLE_INVALID};

/**
 * @brief get_next_cycled_idx - cycle through config items
 */
static int _get_next_cycled_idx(int current_idx) {
	if (current_idx < 0) {
		return 0;
	}
	int next_idx = current_idx + 1;
	if (next_idx >= (int)config_gps_item_count) {
		next_idx = 0;
	}
	return next_idx;
}

static config_module_registry_t gps_module = {
	.name = "gps",
	// .group_id = SCFG_GROUP_GPS,
	.flags = CONFIG_MODULE_FLAG_CYCLE_SCREENS,
	.parent_name = NULL,
	.nvs_key = NVS_KEY_GPS,
	.data_ptr = &g_rtc_config.gps,
	.data_size = sizeof(cfg_gps_t),
	.migrate_func = NULL, // TODO: Add when migration implemented
	.ops =
		{
			.get_item = config_gps_get_item,
			.set_item = config_gps_set_item,
			.get_values = get_gps_item_values,
			.get_descriptions = get_gps_item_descriptions,
			.value_str = config_gps_value_str,
			.get_next_value = config_gps_get_next_value,
			.set_next_value = config_gps_set_next_value,
			.get_next_cycled_idx = _get_next_cycled_idx,
			.item_count = config_gps_item_count,
			.item_names = config_gps_items,
		},
	.handle =
		CONFIG_MODULE_HANDLE_INVALID // Will be assigned during registration
};

void config_gps_register(void) {
	gps_module.handle = config_registry_register(&gps_module);
	if (gps_module.handle == CONFIG_MODULE_HANDLE_INVALID) {
		ELOG(TAG, "Failed to register GPS module");
		return;
	}

	// Register stat_screens as submodule of GPS
	stat_screens_module.handle = config_registry_register(&stat_screens_module);
	if (stat_screens_module.handle == CONFIG_MODULE_HANDLE_INVALID) {
		ELOG(TAG, "Failed to register stat_screens submodule");
	}
}

#if defined(CONFIG_LOGGER_COMMON_ENABLE_CFG_AUTOLOAD)
__attribute__((constructor)) static void config_gps_auto_register(void) {
	config_gps_register();
}
#endif

config_module_handle_t config_gps_handle(void) { return gps_module.handle; }

config_module_handle_t config_stat_screens_handle(void) {
	return stat_screens_module.handle;
}