#include "common_private.h"
#include "config_storage.h"

static const char *TAG = "unified_config";

// Global unified RTC configuration instance
RTC_DATA_ATTR unified_rtc_config_t g_rtc_config = UNIFIED_RTC_CONFIG_DEFAULTS();
static bool unified_config_initialized = false;

/**
 * @brief Load module with version migration support
 *
 * Delegates to config_storage layer for persistence operations.
 */
esp_err_t unified_config_load_module(
	const char *nvs_key, void *data, size_t data_size,
	bool (*migrate_func)(uint16_t from_version, uint16_t to_version,
						 const void *old_data, size_t old_size, void *new_data,
						 size_t new_size)) {
	// Delegate to storage layer
	esp_err_t err = config_storage_load(nvs_key, data, data_size, migrate_func);

	if (err == ESP_OK) {
		DLOG(TAG, "Loaded module %s from storage", nvs_key ? nvs_key : "(no key)");
	} else if (err == ESP_ERR_NOT_FOUND) {
		DLOG(TAG, "Module %s not in storage, using defaults", nvs_key ? nvs_key : "(no key)");
	} else {
		WLOG(TAG, "Failed to load module %s: %s", nvs_key ? nvs_key : "(no key)", esp_err_to_name(err));
	}

	return err;
}

/**
 * @brief Save module to persistent storage with version-aware checksum
 *
 * Delegates to config_storage layer for persistence operations.
 */
esp_err_t unified_config_save_module(const char *nvs_key, const void *data,
									 size_t data_size) {
	// Delegate to storage layer
	esp_err_t err = config_storage_save(nvs_key, data, data_size);

	if (err == ESP_OK) {
		DLOG(TAG, "Saved module %s to storage", nvs_key ? nvs_key : "(no key)");
	} else {
		ELOG(TAG, "Failed to save module %s: %s", nvs_key ? nvs_key : "(no key)", esp_err_to_name(err));
	}

	return err;
}

/**
 * @brief Legacy wrapper for backward compatibility
 * Uses the new unified_config_load_module internally
 */
static esp_err_t unified_config_load_submodule(const char *nvs_key, void *data,
											   size_t data_size) {
	// Try to load with migration support (migrate_func = NULL for legacy)
	return unified_config_load_module(nvs_key, data, data_size, NULL);
}

/**
 * @brief Legacy wrapper for backward compatibility
 * Uses the new unified_config_save_module internally
 */
static esp_err_t unified_config_save_submodule(const char *nvs_key,
											   const void *data,
											   size_t data_size) {
	return unified_config_save_module(nvs_key, data, data_size);
}

static esp_err_t unified_config_save_by_handle(config_module_handle_t module) {
	const config_module_registry_t *reg = config_registry_get(module);
	if (!reg || !reg->data_ptr) {
		WLOG(TAG, "Submodule %u not registered", (unsigned)module);
		return ESP_FAIL;
	}

	return unified_config_save_module(reg->nvs_key, reg->data_ptr,
									  reg->data_size);
}

esp_err_t unified_config_save_by_submodule(int module) {
	size_t module_count = config_registry_get_module_count();
	if (module < 0 || module >= (int)module_count) {
		WLOG(TAG, "Invalid submodule %d for saving", module);
		return ESP_FAIL;
	}

	return unified_config_save_by_handle((config_module_handle_t)module);
}

void unified_config_init(void) {
	if (unified_config_initialized) {
		return;
	}
	unified_config_initialized = true;
	FUNC_ENTRY_ARGS(TAG, "Initializing unified RTC configuration v%d",
					UNIFIED_CONFIG_VERSION);

	// Initialize storage backend (NVS by default)
	esp_err_t err = config_storage_init(NULL);
	if (err != ESP_OK) {
		ELOG(TAG, "Storage init failed: %s", esp_err_to_name(err));
		// Continue with defaults even if storage init fails
	}

	// Ensure registry is initialized first
	config_registry_init_modules();

	// Start with defaults
	g_rtc_config = (unified_rtc_config_t)UNIFIED_RTC_CONFIG_DEFAULTS();

	// Load each registered module from NVS with migration support
	// (skip submodules - they're loaded as part of parent)
	size_t module_count = config_registry_get_module_count();
	for (size_t i = 0; i < module_count; i++) {
		const config_module_registry_t *reg = config_registry_iterate(i);
		if (!reg || !reg->data_ptr) {
			continue; // Skip invalid entries
		}
		if (reg->flags & CONFIG_MODULE_FLAG_SUBMODULE) {
			continue; // Skip submodules - loaded as part of parent
		}
		if (!reg->nvs_key) {
			DLOG(TAG, "Module %s has no storage key, skipping load", reg->name);
			continue; // Skip modules without persistent storage
		}

		err = unified_config_load_module(
			reg->nvs_key, reg->data_ptr, reg->data_size, reg->migrate_func);
		if (err == ESP_OK) {
			DLOG(TAG, "%s module loaded from storage", reg->name);
		} else {
			DLOG(TAG, "%s module not in storage or load failed, using defaults",
				 reg->name);
		}
	}

	ILOG(TAG,
		 "Unified RTC configuration initialized (registry-driven with %zu "
		 "modules)",
		 module_count);
}

void unified_config_save(void) {
	size_t module_count = config_registry_get_module_count();
	FUNC_ENTRY_ARGS(TAG, "Saving unified configuration to storage (%zu modules)",
					module_count);

	// Save each registered module independently
	// (skip submodules - they're saved as part of parent)
	for (size_t i = 0; i < module_count; i++) {
		const config_module_registry_t *reg = config_registry_iterate(i);
		if (!reg || !reg->data_ptr) {
			continue; // Skip invalid entries
		}
		if (reg->flags & CONFIG_MODULE_FLAG_SUBMODULE) {
			continue; // Skip submodules - saved as part of parent
		}
		if (!reg->nvs_key) {
			DLOG(TAG, "Module %s has no storage key, skipping save", reg->name);
			continue; // Skip modules without persistent storage
		}

		unified_config_save_module(reg->nvs_key, reg->data_ptr, reg->data_size);
	}

	ILOG(TAG, "Configuration saved successfully");
}

void unified_config_flush_to_nvs(void) {
	// Since each submodule is saved independently, just call save
	unified_config_save();
}

bool unified_config_validate(void) {
	// With version-aware module storage, validation is done during load
	// This is a no-op; modules are validated during load with checksums
	return true;
}

void unified_config_repair(void) {
	FUNC_ENTRY(TAG);
	// Reload all modules from storage (will use defaults if corrupted)
	// This will trigger migration if versions don't match
	unified_config_init();

	ILOG(TAG, "Configuration repaired");
}

void unified_config_reset_to_defaults(void) {
	FUNC_ENTRY(TAG);
	// Force all submodules to defaults
	g_rtc_config = (unified_rtc_config_t)UNIFIED_RTC_CONFIG_DEFAULTS();

	// Save all to storage (overwrite existing)
	unified_config_save();

	ILOG(TAG, "Configuration reset to defaults");
}

esp_err_t unified_config_erase_all(void) {
	FUNC_ENTRY(TAG);

	// Delegate to storage layer
	esp_err_t err = config_storage_erase_all();
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to erase all config: %s", esp_err_to_name(err));
	} else {
		ILOG(TAG, "Erased all configuration from storage");
	}

	return err;
}
