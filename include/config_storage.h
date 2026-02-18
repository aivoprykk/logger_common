/**
 * @file config_storage.h
 * @brief Configuration persistence layer abstraction
 *
 * Provides backend-agnostic storage for configuration data.
 * Decouples unified_config (RTC memory management) from persistent storage
 * implementation (NVS, files, SD card, etc.).
 *
 * Architecture:
 * - config_manager: High-level API, JSON serialization
 * - unified_config: RTC memory (g_rtc_config), migration logic
 * - config_storage: Persistent storage backend (NVS, file, etc.)
 * - config_registry: Module metadata and dispatch
 */

#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NVS storage wrapper for module data
 *
 * Storage format:
 * - checksum: CRC32 of data[2:] (excludes version field)
 * - data[]: Module struct (version is first 2 bytes)
 */
typedef struct {
	uint32_t checksum; // CRC32 of data excluding version field
	uint8_t data[];	   // Module data, version at bytes 0-1
} __attribute__((packed)) nvs_module_blob_t;

/**
 * @brief Migration function signature (from config_registry.h)
 *
 * @param from_version Version in storage
 * @param to_version Current code version
 * @param old_data Pointer to storage data
 * @param old_size Size of storage data
 * @param new_data Pointer to destination buffer
 * @param new_size Size of destination buffer
 * @return true if migration succeeded, false otherwise
 */
typedef bool (*config_migrate_func_t)(uint16_t from_version,
									  uint16_t to_version, const void *old_data,
									  size_t old_size, void *new_data,
									  size_t new_size);

/**
 * @brief Storage backend interface
 *
 * Implementations provide persistent storage for configuration modules.
 * All operations are synchronous and return esp_err_t status codes.
 */
typedef struct {
	/**
	 * @brief Initialize storage backend
	 * @return ESP_OK on success, error code otherwise
	 */
	esp_err_t (*init)(void);

	/**
	 * @brief Load module data from storage
	 *
	 * @param key Storage key (e.g., "gps_cfg")
	 * @param data Destination buffer for module data
	 * @param data_size Size of destination buffer
	 * @param migrate Migration callback (NULL if not supported)
	 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key not found, error
	 * code otherwise
	 */
	esp_err_t (*load)(const char *key, void *data, size_t data_size,
					  config_migrate_func_t migrate);

	/**
	 * @brief Save module data to storage
	 *
	 * @param key Storage key (e.g., "gps_cfg")
	 * @param data Module data to save
	 * @param data_size Size of module data
	 * @return ESP_OK on success, error code otherwise
	 */
	esp_err_t (*save)(const char *key, const void *data, size_t data_size);

	/**
	 * @brief Erase a single key from storage
	 *
	 * @param key Storage key to erase
	 * @return ESP_OK on success, error code otherwise
	 */
	esp_err_t (*erase_key)(const char *key);

	/**
	 * @brief Erase all configuration data
	 * @return ESP_OK on success, error code otherwise
	 */
	esp_err_t (*erase_all)(void);
} config_storage_backend_t;

/**
 * @brief Initialize configuration storage with specified backend
 *
 * @param backend Backend implementation (NULL = use default NVS backend)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_storage_init(const config_storage_backend_t *backend);

/**
 * @brief Load module data from persistent storage
 *
 * Validates checksum, handles version migration if provided.
 *
 * @param key Storage key (e.g., "gps_cfg")
 * @param data Destination buffer for module data
 * @param data_size Size of destination buffer
 * @param migrate Migration callback (NULL if not supported)
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if not in storage, error code
 * otherwise
 */
esp_err_t config_storage_load(const char *key, void *data, size_t data_size,
							  config_migrate_func_t migrate);

/**
 * @brief Save module data to persistent storage
 *
 * Calculates checksum and wraps data before storage.
 *
 * @param key Storage key (e.g., "gps_cfg")
 * @param data Module data to save
 * @param data_size Size of module data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_storage_save(const char *key, const void *data,
							  size_t data_size);

/**
 * @brief Erase a single configuration key
 *
 * @param key Storage key to erase
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_storage_erase_key(const char *key);

/**
 * @brief Erase all configuration data from storage
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_storage_erase_all(void);

/**
 * @brief Calculate CRC32 checksum for module data (excluding version field)
 *
 * Version field is assumed to be the first 2 bytes of the data.
 * Checksum is calculated on data starting from byte 2.
 *
 * @param data Module data including version field
 * @param size Total size of module data
 * @return CRC32 checksum of data[2:]
 */
uint32_t config_storage_calculate_crc32(const void *data, size_t size);

/**
 * @brief Get default NVS backend implementation
 * @return Pointer to NVS backend struct
 */
const config_storage_backend_t *config_storage_get_nvs_backend(void);

#ifdef __cplusplus
}
#endif
