/**
 * @file config_template.h
 * @brief Template for configuration module header files
 * 
 * This template provides a standardized structure for configuration modules.
 * Each module should:
 * 1. Define its configuration structure
 * 2. Provide default values
 * 3. Implement the standard API functions
 * 4. Register with the central config manager
 * 
 * @note Replace all <MODULE> placeholders with actual module name
 */

#ifndef CONFIG_TEMPLATE_H
#define CONFIG_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "strbf.h"
#include "config_manager.h"

// ============================================================================
// Module Configuration Structure
// ============================================================================

/**
 * @brief <MODULE> configuration structure
 * 
 * @note Use __attribute__((packed, aligned(4))) for RTC memory compatibility
 * @note Add version field for migration support
 */
typedef struct cfg_<module>_s {
    uint16_t version;          ///< Structure version for migration
    // Add module-specific fields here
    // Example:
    // uint8_t field1;
    // uint16_t field2;
    // char string_field[32];
} __attribute__((packed, aligned(4))) cfg_<module>_t;

// ============================================================================
// Configuration Items Enumeration
// ============================================================================

/**
 * @brief <MODULE> configuration item enumeration
 * 
 * @note Use CFG_<MODULE>_ITEM_LIST macro to define items
 * @note Items are in display/access order
 */
#define CFG_<MODULE>_ITEM_LIST(l) \
    /* l(item1) */ \
    /* l(item2) */ \
    /* Add more items here */

#define CFG_ENUM_<MODULE>(l) cfg_<module>_##l,

typedef enum {
    CFG_<MODULE>_ITEM_LIST(CFG_ENUM_<MODULE>)
} <module>_cfg_item_t;

// ============================================================================
// Default Values
// ============================================================================

/**
 * @brief Default configuration values
 */
#define CFG_<MODULE>_DEFAULTS() { \
    .version = 1, \
    /* Initialize fields with default values */ \
    /* .field1 = 0, */ \
    /* .field2 = 100, */ \
    /* .string_field = "default", */ \
}

// ============================================================================
// Public API Functions (Required by config_manager)
// ============================================================================

/**
 * @brief Get configuration item information
 * 
 * @param index Item index (0-based within this module)
 * @param info Pointer to config_item_info_t to populate
 * @return true on success, false on error
 */
bool config_<module>_get_item(size_t index, config_item_info_t *info);

/**
 * @brief Set configuration item value
 * 
 * @param index Item index (0-based within this module)
 * @param value String representation of new value
 * @return true on success, false on error
 */
bool config_<module>_set_item(size_t index, const char *value);

/**
 * @brief Get string representation of item value
 * 
 * @param index Item index
 * @param sb String buffer to write to
 * @param type Output parameter for item type
 * @return true on success, false on error
 */
bool config_<module>_value_str(size_t index, strbf_t *sb, uint8_t *type);

/**
 * @brief Get item description
 * 
 * @param index Item index
 * @param sb String buffer to write to
 * @return true on success, false on error
 */
bool get_<module>_item_descriptions(size_t index, strbf_t *sb);

/**
 * @brief Get item possible values (for enums/options)
 * 
 * @param index Item index
 * @param sb String buffer to write to
 * @return true on success, false on error
 */
uint8_t get_<module>_item_values(size_t index, strbf_t *sb);

// ============================================================================
// Optional API Functions
// ============================================================================

/**
 * @brief Get next value for cycling (UI support)
 * 
 * @param index Item index
 * @return Next value in cycle
 */
uint8_t config_<module>_get_next_value(size_t index);

/**
 * @brief Set to next value in cycle (UI support)
 * 
 * @param index Item index
 * @return true on success, false on error
 */
bool config_<module>_set_next_value(size_t index);

/**
 * @brief Validate module configuration
 * 
 * @return true if configuration is valid, false otherwise
 */
bool config_<module>_validate(void);

/**
 * @brief Repair corrupted configuration
 * 
 * Restores defaults for invalid values
 */
void config_<module>_repair(void);

/**
 * @brief Migrate configuration from older version
 * 
 * @param from_version Source version
 * @param to_version Target version
 * @return true if migration successful, false otherwise
 */
bool config_<module>_migrate(uint16_t from_version, uint16_t to_version,
                            const void *old_data, size_t old_size,
                            void *new_data, size_t new_size);

// ============================================================================
// Module-specific Helper Functions
// ============================================================================

/**
 * @brief Get current configuration pointer
 * 
 * @return Pointer to module configuration in RTC memory
 */
static inline cfg_<module>_t *config_<module>_get(void) {
    return &g_rtc_config.<module>;
}

/**
 * @brief Check if configuration has changed
 * 
 * @return true if configuration modified since last save
 */
bool config_<module>_is_dirty(void);

// ============================================================================
// External Declarations
// ============================================================================

extern const char * const config_<module>_items[];
extern const size_t config_<module>_item_count;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_TEMPLATE_H */