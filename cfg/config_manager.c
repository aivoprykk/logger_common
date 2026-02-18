// Config Manager Implementation
//
// API Design Notes:
// - Group-specific functions (config_{group}_get_item/set_item) take
// group-relative indices (0, 1, 2, ...)
// - NEW: Enum-based functions (config_{group}_get_item_by_enum) take
// sconfig_item_enum_t values
// - NEW: Global functions (config_get_item_by_enum) take any
// sconfig_item_enum_t value
// - Global enum values (sconfig_item_enum_t) can be used directly in the new
// enum-based functions
// - Use the unified functions config_get_item(group, index) for programmatic
// access
// - get_sconfig_item(group, index) handles the mapping from
// group+relative_index to global config array

#include "common_private.h"

static const char *TAG = "config_manager";

// Shared arrays for item descriptions
const char *const not_set = "not set";

const char *sconfig_group_names(config_module_handle_t id) {
	const config_module_registry_t *reg = config_registry_get(id);
	return reg ? reg->name : "unknown";
};

bool config_manager_is_group_default_hidden(config_module_handle_t group) {
	const config_module_registry_t *reg = config_registry_get(group);
	if (!reg)
		return false;
	return (reg->flags & CONFIG_MODULE_FLAG_DEFAULT_HIDDEN) != 0;
}

// ============================================================================
// Function pointer table replaced by config_registry
// Use config_registry_get_ops(group) to access operations
// ============================================================================

bool insert_json_string_value(struct strbf_s *sb, const char *str) {
	// printf("[%s] str: %s\n", __func__, str ? str : "-");
	if (!sb || !str)
		return false;
	strbf_putc(sb, '"');
	size_t len = strlen(str);
	for (size_t i = 0; i < len; ++i) {
		if (*(str + i) == '"' && (i == 0 || *(str + i - 1) != '\\'))
			strbf_putc(sb, '\\');
		strbf_putc(sb, *(str + i));
	}
	strbf_putc(sb, '"');
	return true;
}

bool set_string_from_json(char *str, const char *json) {
	// printf("[%s] json: %s, str: %s\n", __func__, json ? json : "-", str ? str
	// : "-");
	if (!str || !json)
		return false;
	while (*json == '"')
		++json;
	const char *end = json;
	while (*end != '\0') {
		if (*end == '"' && !(end > json && *(end - 1) == '\\')) {
			break;
		}
		++end;
	}

	size_t len = (size_t)(end - json);
	size_t slen = strlen(str);
	if (slen == len && (len == 0 || memcmp(str, json, len) == 0))
		return false;
	if (len)
		memcpy(str, json, len);
	str[len] = '\0';
	// printf("set string: %s from %s\n", str, json);
	return true;
}

// Helper function to add values array to JSON (matches original logger_config
// format)
void add_values_array(strbf_t *sb, const char *const *options,
					  const uint8_t *option_values, size_t count,
					  uint8_t init_value) {
	if (!options || count == 0)
		return;

	strbf_puts(sb, ",\"values\":[");
	for (size_t i = 0; i < count; i++) {
		if (i > 0)
			strbf_putc(sb, ',');
		strbf_puts(sb, "{\"value\":");
		if (option_values) {
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

// Helper function to add values array to JSON (matches original logger_config
// format)
void add_toggles_array(strbf_t *sb, const char *const *options,
					   const uint8_t *option_values, size_t count,
					   uint8_t init_value) {
	if (!options || count == 0)
		return;

	strbf_puts(sb, ",\"toggles\":[");
	uint16_t j = 1;
	for (size_t i = 0, k = count; i < k; i++, j <<= 1) {
		if (i > 0)
			strbf_putc(sb, ',');
		strbf_puts(sb, "{\"pos\":");
		if (option_values) {
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
static const char *get_type_string(sconfig_item_type_t type) {
	switch (type) {
	case SCONFIG_ITEM_TYPE_BOOL:
		return "bool";
	case SCONFIG_ITEM_TYPE_STRING:
		return "str";
	default:
		return "int";
	}
}

bool config_get_item_description(config_module_handle_t group, size_t index,
								 struct strbf_s *sb) {
	const config_group_ops_t *ops = config_registry_get_ops(group);
	if (!ops || !ops->get_descriptions)
		return false;
	return ops->get_descriptions(index, sb);
}

uint8_t config_get_item_values(config_module_handle_t group, size_t index,
							   strbf_t *sb) {
	const config_group_ops_t *ops = config_registry_get_ops(group);
	if (!ops || !ops->get_values)
		return 0;
	return ops->get_values(index, sb);
}

bool config_get_item_value_str(config_module_handle_t group_id, size_t idx,
							   struct strbf_s *sb, uint8_t *type) {
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group_id);
	if (!ops || !ops->value_str)
		return false;
	ops->value_str(idx, sb, type);
	return true;
}

// Unified functions

bool config_get_cycle_item(uint8_t group, size_t index,
						   struct m_config_item_s *item) {
	FUNC_ENTRY_ARGSD(TAG, "group: %u, index: %u", group, index);
	config_item_info_t info;
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group);
	if (!ops || !ops->get_item || !ops->get_item(index, &info)) {
		return false;
	}
	// Populate m_config_item_s structure
	item->name = info.name;
	item->pos = info.pos;
	item->value = info.value;
	item->desc = info.desc;
	return true;
}

bool config_set_item(config_module_handle_t group, size_t index,
					 const char *value) {
	if (!value)
		return false;
	const config_group_ops_t *ops = config_registry_get_ops(group);
	if (!ops || !ops->set_item)
		return false;
	return ops->set_item(index, value);
}

uint16_t config_get_next_value(uint8_t group, size_t index) {
	// Get ops from registry and call get_next_value callback
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group);
	if (!ops || !ops->get_next_value) {
		WLOG(TAG, "Get next not implemented for group %d", group);
		return 0;
	}
	return ops->get_next_value(index);
}

bool config_get_next_cycle_idx(uint8_t group, size_t index,
							   size_t *next_index) {
	// Get ops from registry and call get_next_cycled_idx callback
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group);
	if (!ops || !ops->get_next_cycled_idx) {
		WLOG(TAG, "Get next index not implemented for group %d", group);
		return false;
	}
	int next = ops->get_next_cycled_idx(index);
	if (next < 0) {
		return false;
	}
	*next_index = (size_t)next;
	return true;
}

bool config_set_cycle_item(uint8_t group, size_t index) {
	// Get ops from registry and call set_next_value callback
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group);
	if (!ops || !ops->set_next_value) {
		WLOG(TAG, "Cycle not implemented for group %d", group);
		return false;
	}
	return ops->set_next_value(index);
}

size_t config_get_group_size(config_module_handle_t group) {
	const config_group_ops_t *ops = config_registry_get_ops(group);
	return ops ? ops->item_count : 0;
}

bool config_get_group_data(config_module_handle_t group_id,
						   const char *const **names, uint8_t *count) {
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group_id);
	if (!ops) {
		*count = 0;
		return false;
	}
	*count = ops->item_count;
	*names = ops->item_names;
	return true;
}

bool config_get_by_name(const char *name, uint8_t *group_id, size_t *index) {
	if (!name)
		return false;

	// Iterate registry instead of ops table
	size_t module_count = config_registry_get_module_count();
	for (size_t i = 0; i < module_count; ++i) {
		const config_module_registry_t *reg = config_registry_iterate(i);
		if (!reg)
			continue;

		const config_group_ops_t *ops = config_registry_get_ops(reg->handle);
		const char *const *item_names = ops ? ops->item_names : NULL;

		if (!item_names)
			continue;

		for (size_t idx = 0; idx < ops->item_count; ++idx) {
			if (!strcmp(item_names[idx], name)) {
				*index = idx;
				*group_id = reg->handle;
				return true;
			}
		}
	}
	return false;
}

// Unified get item function - dispatches to group-specific get_item functions
bool config_get_item(config_module_handle_t group, size_t index,
					 config_item_info_t *info) {
	if (!info)
		return false;
	const config_group_ops_t *ops = config_registry_get_ops(group);
	if (!ops || !ops->get_item)
		return false;
	return ops->get_item(index, info);
}

bool config_set_item_by_name(const char *name, const char *value) {
	if (!name || !value)
		return false;
	uint8_t group_id;
	size_t index;
	if (!config_get_by_name(name, &group_id, &index))
		return false;
	return config_set_item(group_id, index, value);
}

// Get JSON for a specific configuration item by group and index
int config_manager_get_item_json(config_module_handle_t group, size_t index,
								 strbf_t *sb) {
	config_item_info_t info;
	if (!config_get_item(group, index, &info))
		return 0;
	size_t start = sb->cur - sb->start;
	strbf_puts(sb, "{\"name\":\"");
	strbf_puts(sb, info.name);
	strbf_puts(sb, "\",\"value\":");
	strbf_putul(sb, info.value);
	strbf_puts(sb, ",\"desc\":\"");
	if (info.desc)
		strbf_puts(sb, info.desc);
	strbf_puts(sb, "\"}");
	return sb->cur - sb->start - start;
}

// Optimized version using pre-fetched ops table (avoids 3 extra table lookups)
static int config_manager_get_item_json_with_ops(const config_group_ops_t *ops,
												 const char *name, size_t idx,
												 strbf_t *sb) {
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
	if (!ops->get_descriptions || !ops->get_descriptions(idx, sb)) {
		strbf_puts(sb, "configuration item");
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
	if (!values && (info.min || info.max)) {
		strbf_puts(sb, ",\"min\":");
		strbf_putul(sb, info.min);
		strbf_puts(sb, ",\"max\":");
		strbf_putul(sb, info.max);
	}
	strbf_puts(sb, "}");
	return (sb->cur - sb->start) - start_len; // bytes written
}

// Get configuration item by name (appends JSON to strbf)
int config_manager_get_item_json_l(const char *name, uint8_t group_id,
								   size_t idx, strbf_t *sb) {
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group_id);
	if (!ops)
		return 0;
	return config_manager_get_item_json_with_ops(ops, name, idx, sb);
}

int config_manager_get_item_by_name(const char *name, strbf_t *sb) {
	if (!name)
		return 0;

	uint8_t group_id;
	size_t index;
	if (!config_get_by_name(name, &group_id, &index))
		return 0;

	// Use registry instead of group_ops_table
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group_id);
	if (!ops)
		return 0;
	return config_manager_get_item_json_with_ops(ops, name, index, sb);
}

int config_manager_get_item_by_group_idx(config_module_handle_t group,
										 size_t index, strbf_t *sb) {
	const config_group_ops_t *ops =
		config_registry_get_ops((config_module_handle_t)group);
	if (!ops)
		return 0;

	// Single registry lookup, then direct function calls
	config_item_info_t info;
	if (!ops->get_item || !ops->get_item(index, &info))
		return 0;

	return config_manager_get_item_json_with_ops(ops, info.name, index, sb);
}

// Get all configuration items as JSON array (appends items with commas)
void config_manager_get_all_items(strbf_t *sb, bool *first) {
	// First pass: count total items
	size_t total_items = 0;
	size_t module_count = config_registry_get_module_count();
	for (size_t i = 0; i < module_count; ++i) {
		const config_module_registry_t *reg = config_registry_iterate(i);
		if (reg) {
			total_items += reg->ops.item_count;
		}
	}

	// Second pass: generate JSON with proper comma separation
	size_t current_item = 0;
	for (size_t module_idx = 0; module_idx < module_count; ++module_idx) {
		const config_module_registry_t *reg =
			config_registry_iterate(module_idx);
		if (!reg)
			continue;

		size_t item_count = reg->ops.item_count;
		for (size_t item_idx = 0; item_idx < item_count; ++item_idx) {
			config_manager_get_item_json(reg->handle, item_idx, sb);

			// Add comma if not the last item
			if (current_item < total_items - 1) {
				strbf_putc(sb, ',');
			}
			current_item++;
		}
	}
}

// Manager functions
void config_manager_init(void) {
	FUNC_ENTRY(TAG);
	// Initialize configuration registry (replaces init_group_ops_table)
	config_registry_init();

	// Initialize unified RTC config (loads from persistent storage)
	// Storage initialization (NVS by default) happens in unified_config_init()
	unified_config_init();
}

static bool
config_manager_save_submodule_handle(config_module_handle_t submodule) {
	// Validate module is registered before saving
	const config_group_ops_t *ops = config_registry_get_ops(submodule);
	if (!ops) {
		WLOG(TAG, "Cannot save unknown/unregistered submodule %u",
			 (unsigned)submodule);
		return false;
	}

	esp_err_t err = unified_config_save_by_submodule((int)submodule);
	if (err == ESP_OK) {
		ILOG(TAG, "CFG,UPDATED");
	}
	return err == ESP_OK;
}

bool config_manager_save_submodule(int submodule_int) {
	FUNC_ENTRY_ARGS(TAG, "handle: %d", submodule_int);
	if (submodule_int < 0 ||
		submodule_int >= (int)config_registry_get_module_count()) {
		WLOG(TAG, "Cannot save invalid submodule %d", submodule_int);
		return false;
	}
	return config_manager_save_submodule_handle(
		(config_module_handle_t)submodule_int);
}

bool config_manager_save_by_item_name(const char *name) {
	FUNC_ENTRY_ARGS(TAG, "name: %s", name ? name : "-");
	// Determine which submodule contains this item via registry
	uint8_t submodule_u8;
	size_t index;
	if (!config_get_by_name(name, &submodule_u8, &index)) {
		WLOG(TAG, "Could not find config item: %s", name);
		return false;
	}
	config_module_handle_t submodule = (config_module_handle_t)submodule_u8;

	DLOG(TAG, "Saving item '%s' to submodule %d", name, submodule);
	return config_manager_save_submodule_handle(submodule);
}

bool config_manager_save(void) {
	FUNC_ENTRY(TAG);
	// Save unified config to NVS (commits happen per-module)
	unified_config_save();
	ILOG(TAG, "CFG,UPDATED");
	return true;
}

bool config_manager_load(void) {
	FUNC_ENTRY(TAG);
	// unified_config already loaded in init
	// sconfig automatically loads defaults on init
	return true;
}

bool config_manager_reset(void) {
	FUNC_ENTRY(TAG);
	// Erase NVS first, then reset to defaults
	esp_err_t err = unified_config_erase_all();
	if (err == ESP_OK) {
		// Force defaults to RTC memory and save
		unified_config_reset_to_defaults();
		ILOG(TAG, "CFG,RESET");
	}
	return err == ESP_OK;
}

bool config_manager_repair(void) {
	FUNC_ENTRY(TAG);
	// Repair corrupted config (tries NVS restore first, defaults as
	// fallback)
	unified_config_repair();
	// sconfig doesn't need repair - it auto-loads on init
	return true;
}
// ============================================================================
// Module-level accessor functions (via registry)
// ============================================================================

bool config_module_get_item(config_module_handle_t handle, size_t index,
							config_item_info_t *info) {
	const config_module_registry_t *reg = config_registry_get_by_handle(handle);
	if (!reg || !reg->ops.get_item || index >= reg->ops.item_count) {
		return false;
	}
	return reg->ops.get_item(index, info);
}

bool config_module_set_item(config_module_handle_t handle, size_t index,
							const char *value) {
	const config_module_registry_t *reg = config_registry_get_by_handle(handle);
	if (!reg || !reg->ops.set_item || index >= reg->ops.item_count) {
		return false;
	}
	return reg->ops.set_item(index, value);
}

bool config_module_set_next_value(config_module_handle_t handle, size_t index) {
	const config_module_registry_t *reg = config_registry_get_by_handle(handle);
	if (!reg || !reg->ops.set_next_value || index >= reg->ops.item_count) {
		return false;
	}
	return reg->ops.set_next_value(index);
}

int config_module_get_next_cycled_idx(config_module_handle_t handle,
									  int current_idx) {
	const config_module_registry_t *reg = config_registry_get_by_handle(handle);
	if (!reg || !reg->ops.get_next_cycled_idx) {
		return -1;
	}
	return reg->ops.get_next_cycled_idx(current_idx);
}

size_t config_module_get_item_count(config_module_handle_t handle) {
	const config_module_registry_t *reg = config_registry_get_by_handle(handle);
	return reg ? reg->ops.item_count : 0;
}