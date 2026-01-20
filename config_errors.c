/**
 * @file config_errors.c
 * @brief Configuration error handling implementation
 */

#include "config_errors.h"
#include "esp_log.h"

static const char *TAG = "config_errors";

const char *config_error_to_string(config_error_t error) {
    switch (error) {
        case CONFIG_OK:
            return "Success";
        case CONFIG_ERR_INVALID_PARAM:
            return "Invalid parameter";
        case CONFIG_ERR_INVALID_GROUP:
            return "Invalid group ID";
        case CONFIG_ERR_INVALID_INDEX:
            return "Invalid item index";
        case CONFIG_ERR_INVALID_VALUE:
            return "Invalid value";
        case CONFIG_ERR_VALUE_OUT_OF_RANGE:
            return "Value out of valid range";
        case CONFIG_ERR_STRING_TOO_LONG:
            return "String value too long";
        case CONFIG_ERR_LOCK_FAILED:
            return "Failed to acquire configuration lock";
        case CONFIG_ERR_STORAGE_FAILED:
            return "Storage operation failed";
        case CONFIG_ERR_NOT_IMPLEMENTED:
            return "Function not implemented";
        case CONFIG_ERR_NO_MEMORY:
            return "Memory allocation failed";
        case CONFIG_ERR_CORRUPTED:
            return "Configuration corrupted";
        case CONFIG_ERR_VERSION_MISMATCH:
            return "Version mismatch";
        case CONFIG_ERR_NOT_FOUND:
            return "Item not found";
        case CONFIG_ERR_READ_ONLY:
            return "Item is read-only";
        case CONFIG_ERR_UNKNOWN:
            return "Unknown error";
        default:
            return "Unrecognized error code";
    }
}

/**
 * @brief Log configuration error with context
 * 
 * @param error Error code
 * @param tag Log tag
 * @param function Function name
 * @param message Error message
 */
void config_log_error(config_error_t error, const char *tag, const char *function, const char *message) {
    if (config_is_error(error)) {
        ESP_LOGE(tag, "%s: %s (error: %d - %s)", 
                function, message, error, config_error_to_string(error));
    }
}

/**
 * @brief Log configuration warning
 * 
 * @param tag Log tag
 * @param function Function name
 * @param message Warning message
 */
void config_log_warning(const char *tag, const char *function, const char *message) {
    ESP_LOGW(tag, "%s: %s", function, message);
}

/**
 * @brief Log configuration debug info
 * 
 * @param tag Log tag
 * @param function Function name
 * @param message Debug message
 */
void config_log_debug(const char *tag, const char *function, const char *message) {
    ESP_LOGD(tag, "%s: %s", function, message);
}