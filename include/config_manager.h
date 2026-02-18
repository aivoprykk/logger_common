#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "logger_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// Note: config.h is included at end of file to avoid circular includes

// Forward declaration for module handle
typedef uint8_t config_module_handle_t;

// Shared arrays for item descriptions
extern const char *const not_set;

// Array sizes
#define BOARD_LOGOS_COUNT 4
#define SAIL_LOGOS_COUNT 7
#define SCREEN_ROTATIONS_COUNT 4
#define SECONDS_LIST_COUNT 5
#define BAT_VIEWS_COUNT 3

// Get sconfig item by group and index (made public for GPS config loading)
// const struct sconfig_item * get_sconfig_item(sconfig_group_t group, size_t
// index);

// Configuration item info structure (similar to m_config_item_s)
typedef struct {
	const char *name;
	uint8_t pos;
	uint32_t value;
	uint32_t min;
	uint32_t max;
	const char *desc;
} config_item_info_t;

struct strbf_s; // Forward declaration

// Group iteration helpers
// #define CONFIG_GROUP_FOREACH(group, item_count, item) \ 	for (size_t item =
// 0; item < item_count; item++)

// Unified get/set functions (for REST API compatibility)
bool config_get_item(uint8_t group, size_t index, config_item_info_t *info);
bool config_set_item(uint8_t group, size_t index, const char *value);
// Group size getters
size_t config_get_group_size(uint8_t group);

// Cycle configuration item to next value (for UI button presses)
bool config_set_cycle_item(uint8_t group, size_t index);
bool config_get_cycle_item(uint8_t group, size_t index,
						   struct m_config_item_s *item);
bool config_get_next_cycle_idx(uint8_t group, size_t index, size_t *next_index);
uint16_t config_get_next_value(uint8_t group, size_t index);

// Set configuration item by name (for REST API compatibility)
bool config_set_item_by_name(const char *name, const char *value);
bool insert_json_string_value(struct strbf_s *sb, const char *str);
bool set_string_from_json(char *str, const char *json);

// Initialize configuration manager
void config_manager_init(void);

// Save all configuration to NVS
bool config_manager_save(void);

// Save a specific submodule to NVS (targeted save, group is sconfig_group_t)
bool config_manager_save_submodule(int submodule);

// Save only the submodule containing the specified item
bool config_manager_save_by_item_name(const char *name);

// Load all configuration from NVS
bool config_manager_load(void);

// Reset configuration to defaults
bool config_manager_reset(void);

// Repair corrupted configuration (tries to restore from NVS first)
bool config_manager_repair(void);

// Get configuration item by name (for REST API compatibility)
int config_manager_get_item_by_name(const char *name, struct strbf_s *sb);

// Get all configuration items as JSON array (appends items with commas)
void config_manager_get_all_items(struct strbf_s *sb, bool *first);

// Get JSON for a specific configuration item by group and index
int config_manager_get_item_json(uint8_t group, size_t index,
								 struct strbf_s *sb);

bool config_get_by_name(const char *name, uint8_t *group_id, size_t *index);

int config_manager_get_item_by_group_idx(uint8_t group, size_t index,
										 struct strbf_s *sb);

void add_values_array(struct strbf_s *sb, const char *const *options,
					  const uint8_t *option_values, size_t count,
					  uint8_t init_value);
void add_toggles_array(struct strbf_s *sb, const char *const *options,
					   const uint8_t *option_values, size_t count,
					   uint8_t init_value);

// ============================================================================
// Module-level accessor functions (via registry)
// ============================================================================

/**
 * @brief Get item from a module by handle
 * @param handle Module handle (from config_*_handle())
 * @param index Item index within module
 * @param info Output structure
 * @return true if successful
 */
bool config_module_get_item(config_module_handle_t handle, size_t index,
							config_item_info_t *info);

/**
 * @brief Set item in a module by handle
 * @param handle Module handle (from config_*_handle())
 * @param index Item index within module
 * @param value String value to set
 * @return true if successful
 */
bool config_module_set_item(config_module_handle_t handle, size_t index,
							const char *value);

/**
 * @brief Cycle item to next value in a module
 * @param handle Module handle (from config_*_handle())
 * @param index Item index within module
 * @return true if successful
 */
bool config_module_set_next_value(config_module_handle_t handle, size_t index);

/**
 * @brief Get next cycled index for a module (e.g., for stat screens)
 * @param handle Module handle (from config_*_handle())
 * @param current_idx Current index
 * @return Next enabled index, or 0 if none enabled
 */
int config_module_get_next_cycled_idx(config_module_handle_t handle,
									  int current_idx);

/**
 * @brief Get item count for a module
 * @param handle Module handle (from config_*_handle())
 * @return Number of items in module
 */
size_t config_module_get_item_count(config_module_handle_t handle);

#endif /* CONFIG_MANAGER_H */