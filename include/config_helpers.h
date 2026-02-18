/**
 * @file config_helpers.h
 * @brief Configuration helper macros and utilities
 *
 * Common utilities for configuration module development.
 */

#ifndef CONFIG_HELPERS_H
#define CONFIG_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config_errors.h"
#include "config_registry.h"
#include "strbf.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// ============================================================================
// String Utilities
// ============================================================================

/**
 * @brief Safely copy string with bounds checking
 *
 * @param dest Destination buffer
 * @param src Source string
 * @param dest_size Size of destination buffer
 * @return true if successful, false if truncated
 */
static inline bool config_strcpy_safe(char *dest, const char *src,
									  size_t dest_size) {
	if (!dest || !src || dest_size == 0) {
		return false;
	}

	size_t src_len = strlen(src);
	if (src_len >= dest_size) {
		// Truncate
		strncpy(dest, src, dest_size - 1);
		dest[dest_size - 1] = '\0';
		return false;
	}

	strcpy(dest, src);
	return true;
}

/**
 * @brief Check if string is empty or NULL
 */
static inline bool config_str_is_empty(const char *str) {
	return !str || str[0] == '\0';
}

/**
 * @brief Trim whitespace from string
 */
static inline void config_str_trim(char *str) {
	if (!str)
		return;

	char *end;

	// Trim leading space
	while (*str == ' ')
		str++;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && *end == ' ')
		end--;

	// Write new null terminator
	*(end + 1) = '\0';
}

// ============================================================================
// Value Validation
// ============================================================================

/**
 * @brief Check if value is within range
 */
#define CONFIG_VALUE_IN_RANGE(value, min, max)                                 \
	((value) >= (min) && (value) <= (max))

/**
 * @brief Clamp value to range
 */
#define CONFIG_CLAMP(value, min, max)                                          \
	((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

/**
 * @brief Validate integer value range
 */
static inline config_error_t config_validate_int_range(int value, int min,
													   int max) {
	if (!CONFIG_VALUE_IN_RANGE(value, min, max)) {
		return CONFIG_ERR_VALUE_OUT_OF_RANGE;
	}
	return CONFIG_OK;
}

/**
 * @brief Validate unsigned integer value range
 */
static inline config_error_t config_validate_uint_range(unsigned int value,
														unsigned int min,
														unsigned int max) {
	if (!CONFIG_VALUE_IN_RANGE(value, min, max)) {
		return CONFIG_ERR_VALUE_OUT_OF_RANGE;
	}
	return CONFIG_OK;
}

/**
 * @brief Validate string length
 */
static inline config_error_t config_validate_string_length(const char *str,
														   size_t max_len) {
	if (!str) {
		return CONFIG_ERR_INVALID_PARAM;
	}

	size_t len = strlen(str);
	if (len >= max_len) {
		return CONFIG_ERR_STRING_TOO_LONG;
	}
	return CONFIG_OK;
}

// ============================================================================
// JSON Helpers
// ============================================================================

/**
 * @brief Escape string for JSON
 *
 * @param sb String buffer
 * @param str String to escape
 * @return true on success, false on error
 */
bool config_json_escape_string(strbf_t *sb, const char *str);

/**
 * @brief Parse string from JSON
 *
 * @param dest Destination buffer
 * @param json JSON string
 * @param dest_size Size of destination buffer
 * @return true if string was parsed and different, false otherwise
 */
bool config_json_parse_string(char *dest, const char *json, size_t dest_size);

/**
 * @brief Add JSON key-value pair for integer
 */
static inline void config_json_add_int(strbf_t *sb, const char *key,
									   int value) {
	strbf_puts(sb, "\"");
	strbf_puts(sb, key);
	strbf_puts(sb, "\":");
	strbf_putd(sb, value, 0, 0);
}

/**
 * @brief Add JSON key-value pair for unsigned integer
 */
static inline void config_json_add_uint(strbf_t *sb, const char *key,
										unsigned int value) {
	strbf_puts(sb, "\"");
	strbf_puts(sb, key);
	strbf_puts(sb, "\":");
	strbf_putul(sb, value);
}

/**
 * @brief Add JSON key-value pair for string
 */
static inline void config_json_add_string(strbf_t *sb, const char *key,
										  const char *value) {
	strbf_puts(sb, "\"");
	strbf_puts(sb, key);
	strbf_puts(sb, "\":\"");
	strbf_puts(sb, value);
	strbf_puts(sb, "\"");
}

/**
 * @brief Add JSON key-value pair for boolean
 */
static inline void config_json_add_bool(strbf_t *sb, const char *key,
										bool value) {
	strbf_puts(sb, "\"");
	strbf_puts(sb, key);
	strbf_puts(sb, "\":");
	strbf_puts(sb, value ? "true" : "false");
}

// ============================================================================
// Configuration Item Helpers
// ============================================================================

/**
 * @brief Get item name from group and index
 *
 * @param group Configuration group
 * @param index Item index
 * @return Item name or NULL if invalid
 */
const char *config_get_item_name(config_module_handle_t group, size_t index);

/**
 * @brief Get item description
 *
 * @param group Configuration group
 * @param index Item index
 * @param buffer Buffer to store description
 * @param buffer_size Size of buffer
 * @return true on success, false on error
 */
bool config_get_item_description_str(config_module_handle_t group, size_t index,
									 char *buffer, size_t buffer_size);

/**
 * @brief Get item type
 *
 * @param group Configuration group
 * @param index Item index
 * @return Item type or SCONFIG_ITEM_TYPE_MAX if invalid
 */
sconfig_item_type_t config_get_item_type(config_module_handle_t group,
										 size_t index);

// ============================================================================
// Debug and Logging
// ============================================================================

/**
 * @brief Dump configuration to log (debug level)
 *
 * @param group Configuration group
 */
void config_dump_group(config_module_handle_t group);

/**
 * @brief Dump all configuration to log (debug level)
 */
void config_dump_all(void);

/**
 * @brief Get configuration statistics
 *
 * @param total_items Output: total number of items
 * @param total_size Output: total size in bytes
 * @param rtc_size Output: RTC memory size in bytes
 */
void config_get_stats(size_t *total_items, size_t *total_size,
					  size_t *rtc_size);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_HELPERS_H */