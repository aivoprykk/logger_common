/**
 * @file config_registry.c
 * @brief Unified configuration module registry implementation
 */

#include "common_private.h"

static const char *TAG = "config_registry";

// Dynamic registry storage
static struct {
	const config_module_registry_t *modules[CONFIG_MAX_MODULES];
	uint8_t count;
	uint8_t initialized;
} s_registry = {0};

/**
 * @brief Initialize the configuration registry
 *
 * Just initializes the registry data structure.
 * Modules register themselves when their init functions are called.
 */
void config_registry_init(void) {
	if (s_registry.initialized) {
		return;
	}
	// Initialize all entries to zero first
	memset(&s_registry, 0, sizeof(s_registry));
	s_registry.initialized = 1;

	ILOG(TAG, "Registry initialized, ready to accept module registrations");
}

void config_registry_init_modules(void) {
	// Ensure registry is initialized
	if (!s_registry.initialized) {
		config_registry_init();
	}
#if !defined(CONFIG_LOGGER_COMMON_ENABLE_CFG_AUTOLOAD)
	// Call registration functions for all known modules
	config_ubx_register(); // first module should be cycle item as
						   // ctx->config_gr_state.group is 0 by default
	config_gps_register();
	config_screen_register();
	config_fw_update_register();
	config_advanced_register();
	config_wifi_register();
#if defined(CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
	config_admin_register();
#endif
#endif
}

/**
 * @brief Register a module with the configuration registry
 *
 * Auto-initializes registry if not already initialized (lazy init pattern).
 */
config_module_handle_t
config_registry_register(config_module_registry_t *entry) {
	// Auto-initialize registry if needed (lazy initialization)
	if (!s_registry.initialized) {
		config_registry_init();
	}
	if (!entry || !entry->name) {
		ELOG(TAG, "Invalid module descriptor");
		return CONFIG_MODULE_HANDLE_INVALID;
	}

	if (s_registry.count >= CONFIG_MAX_MODULES) {
		ELOG(TAG, "Registry full (max %d modules)", CONFIG_MAX_MODULES);
		return CONFIG_MODULE_HANDLE_INVALID;
	}

	// Check for duplicate names
	for (uint8_t i = 0; i < s_registry.count; i++) {
		if (strcmp(s_registry.modules[i]->name, entry->name) == 0) {
			ELOG(TAG, "Module '%s' already registered", entry->name);
			return CONFIG_MODULE_HANDLE_INVALID;
		}
	}

	// Assign handle and register
	config_module_handle_t handle = (config_module_handle_t)s_registry.count;
	entry->handle = handle;
	s_registry.modules[s_registry.count++] = entry;

	ILOG(TAG, "Registered module '%s' (handle=%" PRIu8 ", nvs_key='%s')",
		 entry->name, entry->handle, entry->nvs_key ? entry->nvs_key : "NULL");

	return handle;
}

const config_module_registry_t *
config_registry_get_by_handle(config_module_handle_t handle) {
	if (handle >= s_registry.count) {
		return NULL;
	}
	return s_registry.modules[handle];
}

const config_module_registry_t *config_registry_get_by_name(const char *name) {
	if (!name) {
		return NULL;
	}

	for (uint8_t i = 0; i < s_registry.count; i++) {
		if (strcmp(s_registry.modules[i]->name, name) == 0) {
			return s_registry.modules[i];
		}
	}
	return NULL;
}

const config_module_registry_t *
config_registry_get_by_nvs_key(const char *nvs_key) {
	if (!nvs_key) {
		return NULL;
	}

	for (size_t i = 0; i < s_registry.count; i++) {
		if (strcmp(s_registry.modules[i]->nvs_key, nvs_key) == 0) {
			return s_registry.modules[i];
		}
	}
	return NULL;
}

config_module_handle_t config_registry_get_handle_by_name(const char *name) {
	if (!name) {
		return CONFIG_MODULE_HANDLE_INVALID;
	}

	for (uint8_t i = 0; i < s_registry.count; i++) {
		if (strcmp(s_registry.modules[i]->name, name) == 0) {
			return s_registry.modules[i]->handle;
		}
	}
	return CONFIG_MODULE_HANDLE_INVALID;
}

size_t config_registry_get_module_count(void) { return s_registry.count; }

const config_module_registry_t *config_registry_iterate(size_t index) {
	if (index >= s_registry.count) {
		return NULL;
	}
	return s_registry.modules[index];
}

bool config_registry_is_registered(const char *name) {
	return config_registry_get_by_name(name) != NULL;
}

// ============================================================================
// BACKWARD COMPATIBILITY API (uses group_id field for legacy lookups)
// ============================================================================
const config_module_registry_t *
config_registry_get(config_module_handle_t group) {
	FUNC_ENTRY_ARGSD(TAG, "group: %u", group);
	// Find module with matching handle
	for (uint8_t i = 0; i < s_registry.count; i++) {
		if (s_registry.modules[i]->handle == group) {
			FUNC_ENTRY_ARGSD(TAG, "Found module '%s' for group %u",
							 s_registry.modules[i]->name, group);
			return s_registry.modules[i];
		}
	}
	FUNC_ENTRY_ARGSD(TAG, "No module found for group %u", group);
	return NULL;
}

const config_group_ops_t *
config_registry_get_ops(config_module_handle_t group) {
	const config_module_registry_t *reg = config_registry_get(group);
	return reg ? &reg->ops : NULL;
}

const char *config_registry_get_nvs_key(config_module_handle_t group) {
	const config_module_registry_t *reg = config_registry_get(group);
	return reg ? reg->nvs_key : NULL;
}

void *config_registry_get_data_ptr(config_module_handle_t group) {
	const config_module_registry_t *reg = config_registry_get(group);
	return reg ? reg->data_ptr : NULL;
}

size_t config_registry_get_data_size(config_module_handle_t group) {
	const config_module_registry_t *reg = config_registry_get(group);
	return reg ? reg->data_size : 0;
}

/**
 * @brief Get next module with CONFIG_MODULE_FLAG_CYCLE_SCREENS flag
 *
 * Wraps around to 0 if current_idx is the last cycled module.
 * Returns CONFIG_MODULE_HANDLE_INVALID if no cycled modules exist.
 */
uint8_t config_registry_get_next_cycled_module(uint8_t current_idx) {
	FUNC_ENTRY_ARGSD(TAG, "current_idx: %u", current_idx);
	// Find next module starting from current_idx + 1
	uint8_t start = (current_idx >= s_registry.count - 1) ? 0 : current_idx + 1;

	for (uint8_t i = 0; i < s_registry.count; i++) {
		uint8_t idx = (start + i) % s_registry.count;
		const config_module_registry_t *mod = s_registry.modules[idx];

		if (mod && (mod->flags & CONFIG_MODULE_FLAG_CYCLE_SCREENS)) {
			return (uint8_t)idx;
		}
	}

	// No cycled modules found
	return CONFIG_MODULE_HANDLE_INVALID;
}

/**
 * @brief Get count of modules with CONFIG_MODULE_FLAG_CYCLE_SCREENS flag
 */
uint8_t config_registry_get_cycled_module_count(void) {
	FUNC_ENTRY(TAG);
	uint8_t count = 0;
	for (uint8_t i = 0; i < s_registry.count; i++) {
		if (s_registry.modules[i] &&
			(s_registry.modules[i]->flags & CONFIG_MODULE_FLAG_CYCLE_SCREENS)) {
			count++;
		}
	}
	return count;
}
