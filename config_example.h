/**
 * @file config_example.h
 * @brief Example configuration module
 * 
 * This is a template for creating new configuration modules.
 * Copy and modify for your specific module.
 * 
 * Module: Example
 * Group: SCFG_GROUP_EXAMPLE (must be added to config.h)
 */

#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config_manager.h"

// ============================================================================
// Example Configuration Structure
// ============================================================================

typedef struct cfg_example_s {
    uint16_t version;          ///< Structure version (start at 1)
    
    // Example fields - modify as needed
    uint8_t enabled;           ///< Enable/disable feature (0/1)
    uint16_t interval;         ///< Interval in milliseconds (100-5000)
    uint8_t mode;              ///< Operation mode (0=low, 1=medium, 2=high)
    char name[16];             ///< Name string
    
    // Add checksum for integrity verification (optional)
    uint32_t checksum;
} __attribute__((packed, aligned(4))) cfg_example_t;

// ============================================================================
// Configuration Items Enumeration
// ============================================================================

// Define items in display/access order
#define CFG_EXAMPLE_ITEM_LIST(l) \
    l(enabled)   \
    l(interval)  \
    l(mode)      \
    l(name)

#define CFG_ENUM_EXAMPLE(l) cfg_example_##l,

typedef enum {
    CFG_EXAMPLE_ITEM_LIST(CFG_ENUM_EXAMPLE)
} example_cfg_item_t;

// ============================================================================
// Default Values
// ============================================================================

#define CFG_EXAMPLE_DEFAULTS() { \
    .version = 1, \
    .enabled = 1, \
    .interval = 1000, \
    .mode = 1, \
    .name = "Example", \
    .checksum = 0 \
}

// ============================================================================
// Option Arrays (for enum/select fields)
// ============================================================================

// Mode options
#define EXAMPLE_MODE_OPTIONS(l) \
    l(Low) \
    l(Medium) \
    l(High)

extern const char * const example_mode_options[];
extern const size_t example_mode_options_count;

// ============================================================================
// Required API Functions
// ============================================================================

bool config_example_get_item(size_t index, config_item_info_t *info);
bool config_example_set_item(size_t index, const char *value);
bool config_example_value_str(size_t index, strbf_t *sb, uint8_t *type);
bool get_example_item_descriptions(size_t index, strbf_t *sb);
uint8_t get_example_item_values(size_t index, strbf_t *sb);

// ============================================================================
// Optional API Functions
// ============================================================================

uint8_t config_example_get_next_value(size_t index);
bool config_example_set_next_value(size_t index);
bool config_example_validate(void);
void config_example_repair(void);
bool config_example_migrate(uint16_t from_version, uint16_t to_version);

// ============================================================================
// Module-specific Helper Functions
// ============================================================================

/**
 * @brief Get current configuration pointer
 */
static inline cfg_example_t *config_example_get(void) {
    // Note: This assumes g_rtc_config has an 'example' field
    // You need to add it to unified_rtc_config_t in unified_config.h
    return &g_rtc_config.example;
}

/**
 * @brief Check if example feature is enabled
 */
static inline bool config_example_is_enabled(void) {
    return config_example_get()->enabled != 0;
}

/**
 * @brief Get interval in seconds
 */
static inline float config_example_get_interval_seconds(void) {
    return config_example_get()->interval / 1000.0f;
}

// ============================================================================
// External Declarations
// ============================================================================

extern const char * const config_example_items[];
extern const size_t config_example_item_count;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_EXAMPLE_H */