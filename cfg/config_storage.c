/**
 * @file config_storage.c
 * @brief Configuration persistence layer implementation
 *
 * Provides NVS-based storage backend for configuration data. Extracted from
 * unified_config.c to separate RTC memory management from persistent storage
 * concerns.
 */

#include "config_storage.h"
#include "common_private.h"
#include "logger_buffer_pool.h"
#include <esp_rom_crc.h>
#include <nvs.h>
#include <nvs_flash.h>

static const char *TAG = "config_storage";

// NVS namespace for unified config
#define UNIFIED_CONFIG_NVS_NAMESPACE "unified_cfg"

// Active backend (defaults to NVS)
static const config_storage_backend_t *g_storage_backend = NULL;

// Forward declarations for NVS backend
static esp_err_t nvs_backend_init(void);
static esp_err_t nvs_backend_load(const char *key, void *data,
								  size_t data_size,
								  config_migrate_func_t migrate);
static esp_err_t nvs_backend_save(const char *key, const void *data,
								  size_t data_size);
static esp_err_t nvs_backend_erase_key(const char *key);
static esp_err_t nvs_backend_erase_all(void);

// NVS backend implementation
static const config_storage_backend_t nvs_backend = {
	.init = nvs_backend_init,
	.load = nvs_backend_load,
	.save = nvs_backend_save,
	.erase_key = nvs_backend_erase_key,
	.erase_all = nvs_backend_erase_all,
};

/**
 * @brief Get appropriate buffer size category for module data
 */
static logger_buffer_size_t get_buffer_size_category(size_t data_size) {
	if (data_size <= LOGGER_SMALL_BUFFER_SIZE) {
		return LOGGER_BUFFER_SMALL;
	} else if (data_size <= LOGGER_MEDIUM_BUFFER_SIZE) {
		return LOGGER_BUFFER_MEDIUM;
	}
	if (data_size > LOGGER_LARGE_BUFFER_SIZE) {
		WLOG(TAG, "Data size %zu exceeds max buffer size %d", data_size,
			 LOGGER_LARGE_BUFFER_SIZE);
	}
	return LOGGER_BUFFER_LARGE;
}

// ============================================================================
// Public API Implementation
// ============================================================================

uint32_t config_storage_calculate_crc32(const void *data, size_t size) {
	if (size <= 2) {
		return 0; // Not enough data to checksum
	}

	// Skip first 2 bytes (version field)
	const uint8_t *data_bytes = (const uint8_t *)data;
	return esp_rom_crc32_le(0, data_bytes + 2, size - 2);
}

esp_err_t config_storage_init(const config_storage_backend_t *backend) {
	// Use provided backend or default to NVS
	g_storage_backend = backend ? backend : &nvs_backend;

	if (g_storage_backend->init) {
		return g_storage_backend->init();
	}

	return ESP_OK;
}

esp_err_t config_storage_load(const char *key, void *data, size_t data_size,
							  config_migrate_func_t migrate) {
	if (!g_storage_backend || !g_storage_backend->load) {
		ELOG(TAG, "Storage backend not initialized");
		return ESP_ERR_INVALID_STATE;
	}

	return g_storage_backend->load(key, data, data_size, migrate);
}

esp_err_t config_storage_save(const char *key, const void *data,
							  size_t data_size) {
	if (!g_storage_backend || !g_storage_backend->save) {
		ELOG(TAG, "Storage backend not initialized");
		return ESP_ERR_INVALID_STATE;
	}

	return g_storage_backend->save(key, data, data_size);
}

esp_err_t config_storage_erase_key(const char *key) {
	if (!g_storage_backend || !g_storage_backend->erase_key) {
		ELOG(TAG, "Storage backend not initialized");
		return ESP_ERR_INVALID_STATE;
	}

	return g_storage_backend->erase_key(key);
}

esp_err_t config_storage_erase_all(void) {
	if (!g_storage_backend || !g_storage_backend->erase_all) {
		ELOG(TAG, "Storage backend not initialized");
		return ESP_ERR_INVALID_STATE;
	}

	return g_storage_backend->erase_all();
}

const config_storage_backend_t *config_storage_get_nvs_backend(void) {
	return &nvs_backend;
}

// ============================================================================
// NVS Backend Implementation
// ============================================================================

static esp_err_t nvs_backend_init(void) {
	FUNC_ENTRY(TAG);

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
		err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		WLOG(TAG, "NVS needs erase and reinit");
		err = nvs_flash_erase();
		if (err != ESP_OK) {
			ELOG(TAG, "Error erasing NVS flash: %s", esp_err_to_name(err));
			return err;
		}
		err = nvs_flash_init();
	}

	if (err != ESP_OK) {
		ELOG(TAG, "Error initializing NVS flash: %s", esp_err_to_name(err));
		return err;
	}

	ILOG(TAG, "NVS storage backend initialized");
	return ESP_OK;
}

static esp_err_t nvs_backend_load(const char *key, void *data,
								  size_t data_size,
								  config_migrate_func_t migrate) {
	if (!key) {
		DLOG(TAG, "Skipping load for module with no key (submodule)");
		return ESP_OK; // No storage data to load for this module
	}

	nvs_handle_t handle;
	esp_err_t err =
		nvs_open(UNIFIED_CONFIG_NVS_NAMESPACE, NVS_READONLY, &handle);
	if (err != ESP_OK) {
		DLOG(TAG, "Failed to open NVS for %s: %s", key, esp_err_to_name(err));
		return err;
	}

	// Get blob size
	size_t blob_size;
	err = nvs_get_blob(handle, key, NULL, &blob_size);
	if (err != ESP_OK) {
		DLOG(TAG, "Module %s not in NVS: %s", key, esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	// Allocate buffer for blob using centralized buffer pool
	logger_buffer_handle_t buffer_handle = {0};
	logger_buffer_size_t buffer_size = get_buffer_size_category(blob_size);

	err = logger_buffer_pool_alloc(buffer_size, LOGGER_BUFFER_USAGE_CONFIG,
								   &buffer_handle, 100); // 100ms timeout
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to allocate buffer for module %s: %s", key,
			 esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	// Load blob
	err = nvs_get_blob(handle, key, buffer_handle.buffer, &blob_size);
	nvs_close(handle);

	if (err != ESP_OK) {
		ELOG(TAG, "Failed to load blob for %s: %s", key, esp_err_to_name(err));
		logger_buffer_pool_free(&buffer_handle);
		return err;
	}

	// Validate blob structure
	if (blob_size < sizeof(nvs_module_blob_t)) {
		WLOG(TAG, "Blob too small for %s", key);
		logger_buffer_pool_free(&buffer_handle);
		return ESP_ERR_INVALID_SIZE;
	}

	nvs_module_blob_t *blob = (nvs_module_blob_t *)buffer_handle.buffer;
	// Calculate actual module data size: blob_size includes checksum field (4
	// bytes) blob_size = uint32_t(checksum) + data_bytes
	size_t module_data_size =
		blob_size - sizeof(uint32_t); // Subtract checksum size

	// Validate checksum (excluding version field which is first 2 bytes of
	// module data)
	uint32_t calculated_crc =
		config_storage_calculate_crc32(blob->data, module_data_size);

	if (blob->checksum != calculated_crc) {
		WLOG(TAG, "Module %s checksum mismatch (stored: 0x%08X, calc: 0x%08X)",
			 key, (unsigned)blob->checksum, (unsigned)calculated_crc);
		logger_buffer_pool_free(&buffer_handle);
		return ESP_ERR_INVALID_CRC;
	}

	// Get stored version (first 2 bytes of module data)
	uint16_t stored_version = *(uint16_t *)blob->data;

	// Get current version (first 2 bytes of destination buffer)
	uint16_t current_version = *(uint16_t *)data;

	if (stored_version == current_version && module_data_size == data_size) {
		// Same version and size, copy directly
		memcpy(data, blob->data, data_size);
		logger_buffer_pool_free(&buffer_handle);
		DLOG(TAG, "Loaded module %s v%d", key, stored_version);
		return ESP_OK;
	}

	// Version or size mismatch, try migration
	if (migrate) {
		bool migrated =
			migrate(stored_version, current_version, blob->data,
					module_data_size, data, data_size);

		if (migrated) {
			ILOG(TAG, "Migrated module %s from v%d to v%d", key,
				 stored_version, current_version);

			// Save migrated version (recursive call, but will use same version
			// path)
			esp_err_t save_err = nvs_backend_save(key, data, data_size);
			if (save_err != ESP_OK) {
				WLOG(TAG, "Failed to save migrated data for %s", key);
			}

			logger_buffer_pool_free(&buffer_handle);
			return ESP_OK;
		}
	}

	// Migration failed or not implemented
	WLOG(TAG, "Module %s v%d incompatible with v%d, using defaults", key,
		 stored_version, current_version);

	logger_buffer_pool_free(&buffer_handle);
	return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t nvs_backend_save(const char *key, const void *data,
								  size_t data_size) {
	if (!key) {
		DLOG(TAG, "Skipping save for module with no key (submodule)");
		return ESP_OK; // No storage data to save for this module
	}

	nvs_handle_t handle;
	esp_err_t err =
		nvs_open(UNIFIED_CONFIG_NVS_NAMESPACE, NVS_READWRITE, &handle);
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to open NVS for saving %s: %s", key,
			 esp_err_to_name(err));
		return err;
	}

	// Create blob with checksum
	size_t blob_size = sizeof(nvs_module_blob_t) + data_size;
	logger_buffer_handle_t buffer_handle = {0};
	logger_buffer_size_t buffer_size = get_buffer_size_category(blob_size);

	err = logger_buffer_pool_alloc(buffer_size, LOGGER_BUFFER_USAGE_CONFIG,
								   &buffer_handle, 100); // 100ms timeout
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to allocate buffer for saving %s: %s", key,
			 esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	nvs_module_blob_t *blob = (nvs_module_blob_t *)buffer_handle.buffer;

	// Copy data and calculate checksum (excluding version field)
	memcpy(blob->data, data, data_size);
	blob->checksum = config_storage_calculate_crc32(data, data_size);

	// Save to NVS
	err = nvs_set_blob(handle, key, blob, blob_size);
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to save %s: %s", key, esp_err_to_name(err));
	} else {
		DLOG(TAG, "Saved module %s v%d (crc: 0x%08X)", key, *(uint16_t *)data,
			 (unsigned)blob->checksum);
		err = nvs_commit(handle);
		if (err != ESP_OK) {
			ELOG(TAG, "Failed to commit %s: %s", key, esp_err_to_name(err));
		}
	}

	nvs_close(handle);
	logger_buffer_pool_free(&buffer_handle);
	return err;
}

static esp_err_t nvs_backend_erase_key(const char *key) {
	if (!key) {
		return ESP_OK; // Nothing to erase
	}

	nvs_handle_t handle;
	esp_err_t err =
		nvs_open(UNIFIED_CONFIG_NVS_NAMESPACE, NVS_READWRITE, &handle);
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to open NVS for erase: %s", esp_err_to_name(err));
		return err;
	}

	err = nvs_erase_key(handle, key);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
		ELOG(TAG, "Failed to erase key %s: %s", key, esp_err_to_name(err));
	} else {
		ILOG(TAG, "Erased key %s", key);
		nvs_commit(handle);
	}

	nvs_close(handle);
	return err;
}

static esp_err_t nvs_backend_erase_all(void) {
	FUNC_ENTRY(TAG);

	nvs_handle_t handle;
	esp_err_t err =
		nvs_open(UNIFIED_CONFIG_NVS_NAMESPACE, NVS_READWRITE, &handle);
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to open NVS for erase: %s", esp_err_to_name(err));
		return err;
	}

	err = nvs_erase_all(handle);
	if (err != ESP_OK) {
		ELOG(TAG, "Failed to erase NVS: %s", esp_err_to_name(err));
	} else {
		ILOG(TAG, "Erased all configuration from NVS");
		nvs_commit(handle);
	}

	nvs_close(handle);
	return err;
}
