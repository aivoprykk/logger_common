/**
 * @file logger_buffer_pool.c
 * @brief Implementation of centralized buffer pool for ESP GPS Logger
 */

#include "logger_buffer_pool.h"
#include "common_private.h"
#include <string.h>

static const char *TAG = "buffer_pool";

/**
 * @brief Buffer pool structure
 */
typedef struct {
    uint8_t buffers[LOGGER_BUFFER_POOL_COUNT][LOGGER_LARGE_BUFFER_SIZE];
    bool in_use[LOGGER_BUFFER_POOL_COUNT];
    logger_buffer_size_t buffer_sizes[LOGGER_BUFFER_POOL_COUNT];
    logger_buffer_usage_t buffer_usage[LOGGER_BUFFER_POOL_COUNT];
    uint32_t allocation_ids[LOGGER_BUFFER_POOL_COUNT];
    SemaphoreHandle_t mutex;
    bool initialized;
    uint32_t next_allocation_id;
    logger_buffer_pool_stats_t stats;
} logger_buffer_pool_t;

// Buffer pool distribution:
// - 4 small buffers (256B each = 1KB total)
// - 3 medium buffers (512B each = 1.5KB total)  
// - 1 large buffer (1024B = 1KB total)
// Total: 3.5KB of buffer space
static const logger_buffer_size_t BUFFER_SIZE_DISTRIBUTION[LOGGER_BUFFER_POOL_COUNT] = {
    LOGGER_BUFFER_SMALL,  // Buffer 0
    LOGGER_BUFFER_SMALL,  // Buffer 1
    LOGGER_BUFFER_SMALL,  // Buffer 2
    LOGGER_BUFFER_SMALL,  // Buffer 3
    LOGGER_BUFFER_MEDIUM, // Buffer 4
    LOGGER_BUFFER_MEDIUM, // Buffer 5
    LOGGER_BUFFER_MEDIUM, // Buffer 6
    LOGGER_BUFFER_LARGE   // Buffer 7
};

static logger_buffer_pool_t g_buffer_pool = {0};

/**
 * @brief Get actual buffer size for size type
 */
static size_t get_buffer_size(logger_buffer_size_t size_type) {
    switch (size_type) {
        case LOGGER_BUFFER_SMALL:
            return LOGGER_SMALL_BUFFER_SIZE;
        case LOGGER_BUFFER_MEDIUM:
            return LOGGER_MEDIUM_BUFFER_SIZE;
        case LOGGER_BUFFER_LARGE:
            return LOGGER_LARGE_BUFFER_SIZE;
        default:
            return 0;
    }
}

/**
 * @brief Find available buffer of requested size or larger
 */
static int find_available_buffer(logger_buffer_size_t min_size_type) {
    for (int i = 0; i < LOGGER_BUFFER_POOL_COUNT; i++) {
        if (!g_buffer_pool.in_use[i] && 
            g_buffer_pool.buffer_sizes[i] >= min_size_type) {
            return i;
        }
    }
    return -1;
}

esp_err_t logger_buffer_pool_init(void) {
    // ILOG(TAG, "[%s] initialized: %d", __FUNCTION__, g_buffer_pool.initialized);
    if (g_buffer_pool.initialized) {
        return ESP_OK;
    }

    // Initialize buffer sizes according to distribution
    for (int i = 0; i < LOGGER_BUFFER_POOL_COUNT; i++) {
        g_buffer_pool.buffer_sizes[i] = BUFFER_SIZE_DISTRIBUTION[i];
        g_buffer_pool.in_use[i] = false;
        g_buffer_pool.buffer_usage[i] = LOGGER_BUFFER_USAGE_UNKNOWN;
        g_buffer_pool.allocation_ids[i] = 0;
    }

    // Create mutex
    g_buffer_pool.mutex = xSemaphoreCreateMutex();
    if (g_buffer_pool.mutex == NULL) {
        ELOG(TAG, "Failed to create buffer pool mutex");
        return ESP_FAIL;
    }

    // Initialize statistics
    memset(&g_buffer_pool.stats, 0, sizeof(logger_buffer_pool_stats_t));
    g_buffer_pool.next_allocation_id = 1;
    g_buffer_pool.initialized = true;

    ILOG(TAG, "Buffer pool initialized with %d buffers (%.1fKB total)", 
             LOGGER_BUFFER_POOL_COUNT,
             (4 * LOGGER_SMALL_BUFFER_SIZE + 3 * LOGGER_MEDIUM_BUFFER_SIZE + 1 * LOGGER_LARGE_BUFFER_SIZE) / 1024.0f);

    return ESP_OK;
}

esp_err_t logger_buffer_pool_deinit(void) {
    ILOG(TAG, "[%s] initialized: %d", __FUNCTION__, g_buffer_pool.initialized);
    if (!g_buffer_pool.initialized)  return ESP_OK;
    if (xSemaphoreTake(g_buffer_pool.mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // Check for leaked buffers
        int leaked_count = 0;
        for (int i = 0; i < LOGGER_BUFFER_POOL_COUNT; i++) {
            if (g_buffer_pool.in_use[i]) {
                leaked_count++;
                WLOG(TAG, "Buffer %d still in use during deinit (usage: %d, id: %" PRIu32 ")", 
                        i, g_buffer_pool.buffer_usage[i], g_buffer_pool.allocation_ids[i]);
            }
        }
        if (leaked_count > 0) {
            WLOG(TAG, "Found %d leaked buffers during deinit", leaked_count);
        }

        xSemaphoreGive(g_buffer_pool.mutex);
    }
    // Delete mutex
    vSemaphoreDelete(g_buffer_pool.mutex);
    g_buffer_pool.mutex = NULL;
    ILOG(TAG, "Buffer pool deinitialized");
    g_buffer_pool.initialized = false;

    return ESP_OK;
}

esp_err_t logger_buffer_pool_alloc(logger_buffer_size_t size_type, 
                                  logger_buffer_usage_t usage_type,
                                  logger_buffer_handle_t *handle,
                                  uint32_t timeout_ms) {
    if (!g_buffer_pool.initialized) {
        ELOG(TAG, "Buffer pool not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!handle || size_type >= LOGGER_BUFFER_SIZE_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    // Clear handle
    memset(handle, 0, sizeof(logger_buffer_handle_t));

    TickType_t timeout_ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    if (xSemaphoreTake(g_buffer_pool.mutex, timeout_ticks) != pdTRUE) {
        g_buffer_pool.stats.timeout_failures++;
        return ESP_ERR_TIMEOUT;
    }

    // Find available buffer
    int buffer_index = find_available_buffer(size_type);
    if (buffer_index < 0) {
        xSemaphoreGive(g_buffer_pool.mutex);
        g_buffer_pool.stats.failed_allocations++;
        return ESP_FAIL;
    }

    // Allocate buffer
    g_buffer_pool.in_use[buffer_index] = true;
    g_buffer_pool.buffer_usage[buffer_index] = usage_type;
    g_buffer_pool.allocation_ids[buffer_index] = g_buffer_pool.next_allocation_id++;

    // Fill handle
    handle->buffer = g_buffer_pool.buffers[buffer_index];
    handle->size_type = g_buffer_pool.buffer_sizes[buffer_index];
    handle->size = get_buffer_size(handle->size_type);
    handle->usage_type = usage_type;
    handle->allocation_id = g_buffer_pool.allocation_ids[buffer_index];

    // Update statistics
    g_buffer_pool.stats.total_allocations++;
    g_buffer_pool.stats.current_in_use++;
    if (g_buffer_pool.stats.current_in_use > g_buffer_pool.stats.peak_usage) {
        g_buffer_pool.stats.peak_usage = g_buffer_pool.stats.current_in_use;
    }
    if (usage_type < LOGGER_BUFFER_USAGE_MAX) {
        g_buffer_pool.stats.usage_by_type[usage_type]++;
    }

    xSemaphoreGive(g_buffer_pool.mutex);

    // Clear buffer contents
    memset(handle->buffer, 0, handle->size);

    return ESP_OK;
}

esp_err_t logger_buffer_pool_free(logger_buffer_handle_t *handle) {
    if (!g_buffer_pool.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!handle || !handle->buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(g_buffer_pool.mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ELOG(TAG, "Failed to acquire mutex for free");
        return ESP_ERR_TIMEOUT;
    }

    // Find buffer index
    int buffer_index = -1;
    for (int i = 0; i < LOGGER_BUFFER_POOL_COUNT; i++) {
        if (g_buffer_pool.buffers[i] == handle->buffer && 
            g_buffer_pool.allocation_ids[i] == handle->allocation_id) {
            buffer_index = i;
            break;
        }
    }

    if (buffer_index < 0) {
        xSemaphoreGive(g_buffer_pool.mutex);
        ELOG(TAG, "Invalid buffer handle (ptr=%p, id=%" PRIu32 ")", handle->buffer, handle->allocation_id);
        return ESP_ERR_INVALID_ARG;
    }

    if (!g_buffer_pool.in_use[buffer_index]) {
        xSemaphoreGive(g_buffer_pool.mutex);
        ELOG(TAG, "Double free detected for buffer %d", buffer_index);
        return ESP_ERR_INVALID_STATE;
    }

    // Free buffer
    g_buffer_pool.in_use[buffer_index] = false;
    g_buffer_pool.buffer_usage[buffer_index] = LOGGER_BUFFER_USAGE_UNKNOWN;
    g_buffer_pool.allocation_ids[buffer_index] = 0;

    // Update statistics
    g_buffer_pool.stats.current_in_use--;

    xSemaphoreGive(g_buffer_pool.mutex);

    // Clear handle
    memset(handle, 0, sizeof(logger_buffer_handle_t));

    return ESP_OK;
}

esp_err_t logger_buffer_pool_get_stats(logger_buffer_pool_stats_t *stats) {
    if (!g_buffer_pool.initialized || !stats) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(g_buffer_pool.mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    memcpy(stats, &g_buffer_pool.stats, sizeof(logger_buffer_pool_stats_t));

    xSemaphoreGive(g_buffer_pool.mutex);
    return ESP_OK;
}

esp_err_t logger_buffer_pool_reset_stats(void) {
    if (!g_buffer_pool.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(g_buffer_pool.mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    // Reset statistics but preserve current_in_use
    uint32_t current_in_use = g_buffer_pool.stats.current_in_use;
    memset(&g_buffer_pool.stats, 0, sizeof(logger_buffer_pool_stats_t));
    g_buffer_pool.stats.current_in_use = current_in_use;
    g_buffer_pool.stats.peak_usage = current_in_use;

    xSemaphoreGive(g_buffer_pool.mutex);
    return ESP_OK;
}

bool logger_buffer_pool_is_initialized(void) {
    return g_buffer_pool.initialized;
}

int logger_buffer_pool_get_available(logger_buffer_size_t size_type) {
    if (!g_buffer_pool.initialized || size_type >= LOGGER_BUFFER_SIZE_MAX) {
        return -1;
    }

    if (xSemaphoreTake(g_buffer_pool.mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return -1;
    }

    int available = 0;
    for (int i = 0; i < LOGGER_BUFFER_POOL_COUNT; i++) {
        if (!g_buffer_pool.in_use[i] && g_buffer_pool.buffer_sizes[i] >= size_type) {
            available++;
        }
    }

    xSemaphoreGive(g_buffer_pool.mutex);
    return available;
}