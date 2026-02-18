/**
 * @file config_registry.h
 * @brief Unified configuration module registry
 *
 * Centralizes module metadata and operations to eliminate hard-coded
 * module lists across unified_config.c and config_manager.c.
 */

#pragma once

#include "config_manager.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct strbf_s; // Forward declaration

/**
 * @brief Module handle (opaque identifier)
 *
 * Returned by registration, used for O(1) lookups.
 * Invalid handle is 0xFF.
 */
typedef uint8_t config_module_handle_t;
#define CONFIG_MODULE_HANDLE_INVALID 0xFF

/**
 * @brief Maximum number of configuration modules
 */
#define CONFIG_MAX_MODULES 16

/**
 * @brief Module flags for UI behavior and metadata
 */
typedef enum {
	CONFIG_MODULE_FLAG_NONE = 0,
	CONFIG_MODULE_FLAG_DEFAULT_HIDDEN = (1 << 0), // Hide by default in UI
	CONFIG_MODULE_FLAG_SUBMODULE = (1 << 1),	  // Submodule (child of parent)
	CONFIG_MODULE_FLAG_CYCLE_SCREENS = (1 << 2), // Appears in settings rotation
} config_module_flags_t;

/**
 * @brief Module migration function signature
 *
 * @param from_version Version in NVS
 * @param to_version Current code version
 * @param old_data Pointer to NVS data
 * @param old_size Size of NVS data
 * @param new_data Pointer to destination buffer
 * @param new_size Size of destination buffer
 * @return true if migration succeeded, false otherwise
 */
typedef bool (*config_migrate_func_t)(uint16_t from_version,
									  uint16_t to_version, const void *old_data,
									  size_t old_size, void *new_data,
									  size_t new_size);

/**
 * @brief Configuration item operations (from config_manager.c)
 */
typedef struct {
	bool (*get_item)(size_t index, config_item_info_t *info);
	bool (*set_item)(size_t index, const char *value);
	uint8_t (*get_values)(size_t index, struct strbf_s *sb);
	bool (*get_descriptions)(size_t index, struct strbf_s *sb);
	bool (*value_str)(size_t index, struct strbf_s *sb, uint8_t *type);

	// Extended operations (for cycling/UI)
	uint16_t (*get_next_value)(size_t index); // Get next value without setting
	bool (*set_next_value)(size_t index);	  // Cycle to next value
	int (*get_next_cycled_idx)(
		int current_idx); // Get next enabled index (for multi-item like
						  // stat_screens)

	size_t item_count;
	const char *const *item_names;
} config_group_ops_t;

/**
 * @brief Unified module registry entry
 *
 * Combines NVS storage metadata with config manager operations
 * to provide single source of truth for each module.
 *
 * Modules register themselves by providing this descriptor.
 * The registry assigns a handle for fast lookups.
 */
typedef struct {
	// Module identification
	const char *name; // Module name (e.g., "gps", "ubx") - unique identifier
	config_module_handle_t handle; // Assigned during registration (O(1) lookup)
	config_module_flags_t flags;   // Module flags (UI behavior, etc.)
	const char
		*parent_name; // Parent module name (if submodule), NULL otherwise

	// NVS storage metadata (for unified_config.c)
	const char *nvs_key;				// NVS storage key (e.g., "gps_cfg")
	void *data_ptr;						// Pointer to g_rtc_config.<field>
	size_t data_size;					// Size of config struct
	config_migrate_func_t migrate_func; // Optional migration function

	// Config manager operations (for config_manager.c)
	config_group_ops_t ops; // Function pointers for get/set/etc

	// Backward compatibility
	// int
	// 	group_id; // Legacy enum value (DEPRECATED - use handle instead)
} config_module_registry_t;

/**
 * @brief Initialize the configuration registry
 *
 * Calls all module registration functions.
 * Safe to call multiple times (idempotent).
 */
void config_registry_init(void);

/**
 * @brief Register a module with the configuration registry
 *
 * @param entry Pointer to module registration structure (must remain valid)
 * @return Module handle, or CONFIG_MODULE_HANDLE_INVALID on error
 */
config_module_handle_t
config_registry_register(config_module_registry_t *entry);

/**
 * @brief Get registry entry by module handle
 *
 * @param handle Module handle (from registration)
 * @return Pointer to registry entry, or NULL if invalid handle
 */
const config_module_registry_t *
config_registry_get_by_handle(config_module_handle_t handle);

/**
 * @brief Get registry entry by module name
 *
 * @param name Module name (e.g., "gps", "ubx")
 * @return Pointer to registry entry, or NULL if not found
 */
const config_module_registry_t *config_registry_get_by_name(const char *name);

/**
 * @brief Get registry entry by NVS key
 *
 * @param nvs_key NVS storage key (e.g., "gps_cfg")
 * @return Pointer to registry entry, or NULL if not found
 */
const config_module_registry_t *
config_registry_get_by_nvs_key(const char *nvs_key);

/**
 * @brief Get module handle by name
 *
 * @param name Module name
 * @return Module handle, or CONFIG_MODULE_HANDLE_INVALID if not found
 */
config_module_handle_t config_registry_get_handle_by_name(const char *name);

/**
 * @brief Get total number of registered modules
 *
 * @return Number of registered modules
 */
size_t config_registry_get_module_count(void);

/**
 * @brief Iterate over all registered modules
 *
 * @param index Module index (0 to count-1)
 * @return Pointer to registry entry, or NULL if index out of range
 */
const config_module_registry_t *config_registry_iterate(size_t index);

/**
 * @brief Iterate over cycle_screens flagged modules (for settings rotation)
 *
 * @param current_idx Current module handle/index (0 to count-1)
 * @return Next module handle with CONFIG_MODULE_FLAG_CYCLE_SCREENS, or 0xFF if
 * none
 */
uint8_t config_registry_get_next_cycled_module(uint8_t current_idx);

/**
 * @brief Get count of modules with CONFIG_MODULE_FLAG_CYCLE_SCREENS flag
 *
 * @return Number of modules available for cycling
 */
uint8_t config_registry_get_cycled_module_count(void);

// ============================================================================
// BACKWARD COMPATIBILITY API (DEPRECATED - prefer string-based API above)
// ============================================================================

/**
 * @brief Get registry entry for a specific group (DEPRECATED)
 *
 * @param group Group ID
 * @return Pointer to registry entry, or NULL if invalid group
 */
const config_module_registry_t *
config_registry_get(config_module_handle_t group);

/**
 * @brief Get operation functions for a group (DEPRECATED)
 *
 * @param group Group ID
 * @return Pointer to operations struct, or NULL if invalid group
 */
const config_group_ops_t *config_registry_get_ops(config_module_handle_t group);

/**
 * @brief Get NVS key for a group (DEPRECATED)
 *
 * @param group Group ID
 * @return NVS key string, or NULL if invalid group
 */
const char *config_registry_get_nvs_key(config_module_handle_t group);

/**
 * @brief Get data pointer for a group (DEPRECATED)
 *
 * @param group Group ID
 * @return Pointer to g_rtc_config.<field>, or NULL if invalid group
 */
void *config_registry_get_data_ptr(config_module_handle_t group);

/**
 * @brief Get data size for a group (DEPRECATED)
 *
 * @param group Group ID
 * @return Size of config struct, or 0 if invalid group
 */
size_t config_registry_get_data_size(config_module_handle_t group);

// ============================================================================
// NOTE: Module registration is now automatic via constructor attributes
// ============================================================================
//
// Each module (config_manager_*.c) uses __attribute__((constructor)) to
// auto-register itself when loaded. No central code needs to know about
// modules.
//
// Public config_*_register() functions kept for backward compatibility
// (no-ops).
//
// To add a new module:
// 1. Create config_manager_newmodule.c
// 2. Add __attribute__((constructor)) static void
// config_newmodule_auto_register()
// 3. Call config_registry_register(&your_module_descriptor)
// 4. Done! No changes to config_registry.h or config_registry.c needed!
//

/// @brief  manual initialize all configuration modules
/// this is called by unified_config_init()
/// #define CONFIG_LOGGER_COMMON_ENABLE_CFG_AUTOLOAD 1
void config_registry_init_modules(void);
