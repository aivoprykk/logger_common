#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "strbf.h"
#include "logger_common.h"
// Note: config.h is included at end of file to avoid circular includes

// Shared arrays for item descriptions
extern const char * const not_set;

// Array sizes
#define BOARD_LOGOS_COUNT 4
#define SAIL_LOGOS_COUNT 7
#define SCREEN_ROTATIONS_COUNT 4
#define SECONDS_LIST_COUNT 5
#define BAT_VIEWS_COUNT 3

// Get sconfig item by group and index (made public for GPS config loading)
// const struct sconfig_item * get_sconfig_item(sconfig_group_t group, size_t index);

// Configuration item info structure (similar to m_config_item_s)
typedef struct {
    const char *name;
    uint8_t pos;
    uint32_t value;
    uint32_t min;
    uint32_t max;
    const char *desc;
} config_item_info_t;

// Group iteration helpers
#define CONFIG_GROUP_FOREACH(group, item_count, item) \
    for (size_t item = 0; item < item_count; item++)

// Screen settings group functions (index-based access)
bool config_screen_get_item(size_t index, config_item_info_t *info);
bool config_screen_set_item(size_t index, const char * value);

// Advanced settings group functions (brightness/move offset)
bool config_advanced_get_item(size_t index, config_item_info_t *info);
bool config_advanced_set_item(size_t index, const char * value);

// Firmware update group functions
bool config_fw_update_get_item(size_t index, config_item_info_t *info);
bool config_fw_update_set_item(size_t index, const char * value);

// Main config group functions (system + WiFi settings)
bool config_main_get_item(size_t index, config_item_info_t *info);
bool config_main_set_item(size_t index, const char *value);

// Admin group functions
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)
bool config_admin_get_item(size_t index, config_item_info_t *info);
bool config_admin_set_item(size_t index, const char *value);
#endif

// GPS group functions
bool config_stat_screen_get_item(int num, config_item_info_t *item);
bool config_gps_get_item(size_t index, config_item_info_t *info);
bool config_gps_set_item(size_t index, const char *value);

// UBX group functions
bool config_ubx_get_item(size_t index, config_item_info_t *info);
bool config_ubx_set_item(size_t index, const char *value);
uint8_t config_ubx_get_next_value(size_t index);

// GPS group helper functions
bool config_gps_set_next_value(size_t index);

// Screen group helper functions
bool config_screen_set_next_value(size_t index);

enum sconfig_group_e;
enum sconfig_cycle_group_e;

// Unified get/set functions (for REST API compatibility)
bool config_get_item(enum sconfig_group_e group, size_t index, config_item_info_t *info);
bool config_set_item(enum sconfig_group_e group, size_t index, const char *value);
// Group size getters
size_t config_get_group_size(enum sconfig_group_e group);

// Cycle configuration item to next value (for UI button presses)
bool config_set_cycle_item(enum sconfig_cycle_group_e group, size_t index);
bool config_get_cycle_item(enum sconfig_cycle_group_e group, size_t index, struct m_config_item_s *item);
bool config_get_next_cycle_idx(enum sconfig_cycle_group_e group, size_t index, size_t *next_index);
uint16_t config_get_next_value(enum sconfig_cycle_group_e group, size_t index);

// Set configuration item by name (for REST API compatibility)
bool config_set_item_by_name(const char *name, const char *value);
bool insert_json_string_value(struct strbf_s *sb, const char * str);
bool set_string_from_json(char * str, const char * json);

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
int config_manager_get_item_by_name(const char *name, strbf_t *sb);

// Get all configuration items as JSON array (appends items with commas)
void config_manager_get_all_items(strbf_t *sb, bool *first);

// Get JSON for a specific configuration item by group and index
int config_manager_get_item_json(uint8_t group, size_t index, strbf_t *sb);

bool config_get_by_name(const char *name, uint8_t * group_id, uint8_t * index);

int config_manager_get_item_by_group_idx(uint8_t group, size_t index, strbf_t *sb);

void add_values_array(strbf_t *sb, const char * const *options, const uint8_t * option_values, size_t count, uint8_t init_value);
void add_toggles_array(strbf_t *sb, const char * const *options, const uint8_t * option_values, size_t count, uint8_t init_value);

#endif /* CONFIG_MANAGER_H */