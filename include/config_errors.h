/**
 * @file config_errors.h
 * @brief Configuration error codes and handling
 * 
 * Standardized error handling for configuration modules.
 */

#ifndef CONFIG_ERRORS_H
#define CONFIG_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Configuration error codes
 */
typedef enum {
    CONFIG_OK = 0,                    ///< Operation successful
    CONFIG_ERR_INVALID_PARAM = -1,    ///< Invalid parameter
    CONFIG_ERR_INVALID_GROUP = -2,    ///< Invalid group ID
    CONFIG_ERR_INVALID_INDEX = -3,    ///< Invalid item index
    CONFIG_ERR_INVALID_VALUE = -4,    ///< Invalid value
    CONFIG_ERR_VALUE_OUT_OF_RANGE = -5, ///< Value out of valid range
    CONFIG_ERR_STRING_TOO_LONG = -6,  ///< String value too long
    CONFIG_ERR_LOCK_FAILED = -7,      ///< Failed to acquire lock
    CONFIG_ERR_STORAGE_FAILED = -8,   ///< Storage operation failed
    CONFIG_ERR_NOT_IMPLEMENTED = -9,  ///< Function not implemented
    CONFIG_ERR_NO_MEMORY = -10,       ///< Memory allocation failed
    CONFIG_ERR_CORRUPTED = -11,       ///< Configuration corrupted
    CONFIG_ERR_VERSION_MISMATCH = -12, ///< Version mismatch
    CONFIG_ERR_NOT_FOUND = -13,       ///< Item not found
    CONFIG_ERR_READ_ONLY = -14,       ///< Item is read-only
    CONFIG_ERR_UNKNOWN = -255         ///< Unknown error
} config_error_t;

/**
 * @brief Get error message for error code
 * 
 * @param error Error code
 * @return Error message string
 */
const char *config_error_to_string(config_error_t error);

/**
 * @brief Check if operation was successful
 * 
 * @param error Error code
 * @return true if successful (CONFIG_OK), false otherwise
 */
static inline bool config_is_ok(config_error_t error) {
    return error == CONFIG_OK;
}

/**
 * @brief Check if operation failed
 * 
 * @param error Error code
 * @return true if failed (!= CONFIG_OK), false otherwise
 */
static inline bool config_is_error(config_error_t error) {
    return error != CONFIG_OK;
}

/**
 * @brief Macro for returning error with logging
 * 
 * Usage:
 * ```c
 * if (invalid_condition) {
 *     RETURN_CONFIG_ERROR(CONFIG_ERR_INVALID_PARAM, TAG, "Invalid parameter");
 * }
 * ```
 */
#define RETURN_CONFIG_ERROR(error, tag, format, ...) do { \
    ESP_LOGE(tag, format " (error: %d - %s)", ##__VA_ARGS__, error, config_error_to_string(error)); \
    return error; \
} while(0)

/**
 * @brief Macro for returning boolean false with error logging
 * 
 * Usage:
 * ```c
 * if (invalid_condition) {
 *     RETURN_CONFIG_FALSE(CONFIG_ERR_INVALID_PARAM, TAG, "Invalid parameter");
 * }
 * ```
 */
#define RETURN_CONFIG_FALSE(error, tag, format, ...) do { \
    ESP_LOGE(tag, format " (error: %d - %s)", ##__VA_ARGS__, error, config_error_to_string(error)); \
    return false; \
} while(0)

/**
 * @brief Macro for returning NULL with error logging
 * 
 * Usage:
 * ```c
 * if (invalid_condition) {
 *     RETURN_CONFIG_NULL(CONFIG_ERR_INVALID_PARAM, TAG, "Invalid parameter");
 * }
 * ```
 */
#define RETURN_CONFIG_NULL(error, tag, format, ...) do { \
    ESP_LOGE(tag, format " (error: %d - %s)", ##__VA_ARGS__, error, config_error_to_string(error)); \
    return NULL; \
} while(0)

/**
 * @brief Validate parameter is not NULL
 * 
 * Usage:
 * ```c
 * CHECK_PARAM_NOT_NULL(ptr, CONFIG_ERR_INVALID_PARAM);
 * ```
 */
#define CHECK_PARAM_NOT_NULL(param, error_code) do { \
    if ((param) == NULL) { \
        return error_code; \
    } \
} while(0)

/**
 * @brief Validate index is within bounds
 * 
 * Usage:
 * ```c
 * CHECK_INDEX_BOUNDS(index, count, CONFIG_ERR_INVALID_INDEX);
 * ```
 */
#define CHECK_INDEX_BOUNDS(index, count, error_code) do { \
    if ((index) >= (count)) { \
        return error_code; \
    } \
} while(0)

/**
 * @brief Validate group is within bounds
 * 
 * Usage:
 * ```c
 * CHECK_GROUP_BOUNDS(group, CONFIG_ERR_INVALID_GROUP);
 * ```
 */
#define CHECK_GROUP_BOUNDS(group, error_code) do { \
    if ((group) >= SCFG_GROUP_COUNT) { \
        return error_code; \
    } \
} while(0)

/**
 * @brief Acquire configuration lock with error handling
 * 
 * Usage:
 * ```c
 * ACQUIRE_CONFIG_LOCK(TAG);
 * ```
 */
#define ACQUIRE_CONFIG_LOCK(tag) do { \
    if (!config_lock(-1)) { \
        ESP_LOGE(tag, "Failed to acquire config lock"); \
        return CONFIG_ERR_LOCK_FAILED; \
    } \
} while(0)

/**
 * @brief Release configuration lock
 * 
 * Usage:
 * ```c
 * RELEASE_CONFIG_LOCK();
 * ```
 */
#define RELEASE_CONFIG_LOCK() do { \
    config_unlock(); \
} while(0)

/**
 * @brief Execute block with configuration lock
 * 
 * Usage:
 * ```c
 * WITH_CONFIG_LOCK(TAG) {
 *     // Critical section
 * }
 * ```
 */
#define WITH_CONFIG_LOCK(tag) \
    for (bool __lock_acquired = (config_lock(-1) ? true : (ESP_LOGE(tag, "Failed to acquire config lock"), false)); \
         __lock_acquired; \
         config_unlock(), __lock_acquired = false)

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ERRORS_H */