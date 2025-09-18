/**
 * @file logger_buffer_pool.h
 * @brief Centralized buffer pool for ESP GPS Logger components
 * 
 * Provides shared buffer management across logger_http, gps_log, and other components
 * to reduce stack usage and prevent memory fragmentation.
 */

#ifndef A75D3C76_0C6C_4D9E_A9A4_A6137FFBDE38
#define A75D3C76_0C6C_4D9E_A9A4_A6137FFBDE38

#ifndef LOGGER_BUFFER_POOL_H
#define LOGGER_BUFFER_POOL_H

#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Buffer pool configuration
 */
#define LOGGER_BUFFER_POOL_COUNT    8           // Total number of buffers
#define LOGGER_SMALL_BUFFER_SIZE    256         // Small buffer size (path names, small messages)
#define LOGGER_MEDIUM_BUFFER_SIZE   512         // Medium buffer size (log messages, GPS data)
#define LOGGER_LARGE_BUFFER_SIZE    1024        // Large buffer size (file transfers, large messages)

/**
 * @brief Buffer size categories
 */
typedef enum {
    LOGGER_BUFFER_SMALL = 0,                    ///< 256 bytes - for paths, short messages
    LOGGER_BUFFER_MEDIUM = 1,                   ///< 512 bytes - for log messages, GPS data
    LOGGER_BUFFER_LARGE = 2,                    ///< 1024 bytes - for file transfers, large data
    LOGGER_BUFFER_SIZE_MAX
} logger_buffer_size_t;

/**
 * @brief Buffer usage types for debugging and monitoring
 */
typedef enum {
    LOGGER_BUFFER_USAGE_UNKNOWN = 0,
    LOGGER_BUFFER_USAGE_HTTP_PATH,              ///< HTTP handler path buffer
    LOGGER_BUFFER_USAGE_HTTP_SCRATCH,           ///< HTTP handler scratch buffer
    LOGGER_BUFFER_USAGE_GPS_MESSAGE,            ///< GPS log message buffer
    LOGGER_BUFFER_USAGE_GPS_DATA,               ///< GPS data string buffer
    LOGGER_BUFFER_USAGE_GPS_TEXT,               ///< GPS text formatting buffer
    LOGGER_BUFFER_USAGE_CONFIG,                 ///< Configuration buffer
    LOGGER_BUFFER_USAGE_TEMP,                   ///< Temporary operations
    LOGGER_BUFFER_USAGE_MAX
} logger_buffer_usage_t;

/**
 * @brief Buffer pool statistics
 */
typedef struct {
    uint32_t total_allocations;                 ///< Total number of allocations
    uint32_t failed_allocations;               ///< Number of failed allocations
    uint32_t current_in_use;                   ///< Currently allocated buffers
    uint32_t peak_usage;                       ///< Peak concurrent usage
    uint32_t timeout_failures;                 ///< Allocation timeouts
    uint32_t usage_by_type[LOGGER_BUFFER_USAGE_MAX]; ///< Usage statistics by type
} logger_buffer_pool_stats_t;

/**
 * @brief Buffer handle for allocated buffers
 */
typedef struct {
    void *buffer;                              ///< Pointer to allocated buffer
    size_t size;                              ///< Size of allocated buffer
    logger_buffer_size_t size_type;           ///< Size category
    logger_buffer_usage_t usage_type;         ///< Usage type for statistics
    uint32_t allocation_id;                   ///< Unique allocation ID
} logger_buffer_handle_t;

/**
 * @brief Initialize the logger buffer pool
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t logger_buffer_pool_init(void);

/**
 * @brief Deinitialize the logger buffer pool
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t logger_buffer_pool_deinit(void);

/**
 * @brief Allocate a buffer from the pool
 * 
 * @param size_type Buffer size category
 * @param usage_type Buffer usage type for statistics
 * @param handle Output handle for allocated buffer
 * @param timeout_ms Timeout in milliseconds (0 for no timeout)
 * @return ESP_OK on success, ESP_ERR_TIMEOUT on timeout, ESP_FAIL on other errors
 */
esp_err_t logger_buffer_pool_alloc(logger_buffer_size_t size_type, 
                                  logger_buffer_usage_t usage_type,
                                  logger_buffer_handle_t *handle,
                                  uint32_t timeout_ms);

/**
 * @brief Release a buffer back to the pool
 * 
 * @param handle Pointer to buffer handle (will be cleared)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t logger_buffer_pool_free(logger_buffer_handle_t *handle);

/**
 * @brief Get buffer pool statistics
 * 
 * @param stats Output statistics structure
 * @return ESP_OK on success, error code on failure
 */
esp_err_t logger_buffer_pool_get_stats(logger_buffer_pool_stats_t *stats);

/**
 * @brief Reset buffer pool statistics
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t logger_buffer_pool_reset_stats(void);

/**
 * @brief Check if buffer pool is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool logger_buffer_pool_is_initialized(void);

/**
 * @brief Get available buffer count for a specific size
 * 
 * @param size_type Buffer size category
 * @return Number of available buffers, -1 on error
 */
int logger_buffer_pool_get_available(logger_buffer_size_t size_type);

/**
 * @brief Convenience macros for common allocations
 */

// Allocate with 100ms timeout
#define LOGGER_BUFFER_ALLOC(size_type, usage_type, handle) \
    logger_buffer_pool_alloc(size_type, usage_type, handle, 100)

// Allocate with no timeout
#define LOGGER_BUFFER_ALLOC_NOWAIT(size_type, usage_type, handle) \
    logger_buffer_pool_alloc(size_type, usage_type, handle, 0)

// Free buffer and clear handle
#define LOGGER_BUFFER_FREE(handle) \
    logger_buffer_pool_free(handle)

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_BUFFER_POOL_H */


#endif /* A75D3C76_0C6C_4D9E_A9A4_A6137FFBDE38 */
