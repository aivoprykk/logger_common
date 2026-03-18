/**
 * @file logger_fixed_pool.c
 * @brief Generic O(1) fixed-size block pool — see logger_fixed_pool.h for docs.
 */

#include "logger_fixed_pool.h"
#include "common_private.h"

static const char *TAG = "fixed_pool";

esp_err_t logger_fixed_pool_init(logger_fixed_pool_t *pool,
                                  void               *storage,
                                  size_t              block_size,
                                  size_t              block_count)
{
    if (!pool || !storage || block_size == 0 || block_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    pool->free_queue = xQueueCreate((UBaseType_t)block_count, sizeof(void *));
    if (!pool->free_queue) {
        ELOG(TAG, "Failed to create free-list queue (%zu blocks x %zuB)",
             block_count, block_size);
        return ESP_ERR_NO_MEM;
    }

    pool->block_size  = block_size;
    pool->block_count = block_count;

    uint8_t *base = (uint8_t *)storage;
    for (size_t i = 0; i < block_count; i++) {
        void *blk = base + i * block_size;
        if (xQueueSend(pool->free_queue, &blk, 0) != pdTRUE) {
            ELOG(TAG, "Failed to populate free-list at index %zu", i);
            vQueueDelete(pool->free_queue);
            pool->free_queue = NULL;
            return ESP_FAIL;
        }
    }

    ILOG(TAG, "Fixed pool ready: %zu x %zuB = %zuB static DRAM (zero hot-path alloc)",
         block_count, block_size, block_count * block_size);
    return ESP_OK;
}

void logger_fixed_pool_deinit(logger_fixed_pool_t *pool)
{
    if (pool && pool->free_queue) {
        vQueueDelete(pool->free_queue);
        pool->free_queue = NULL;
    }
}

void *logger_fixed_pool_alloc(logger_fixed_pool_t *pool)
{
    if (!pool || !pool->free_queue) {
        return NULL;
    }
    void *blk = NULL;
    xQueueReceive(pool->free_queue, &blk, 0);   /* non-blocking */
    return blk;                                  /* NULL when exhausted */
}

void logger_fixed_pool_free(logger_fixed_pool_t *pool, void *block)
{
    if (pool && pool->free_queue && block) {
        xQueueSend(pool->free_queue, &block, 0);
    }
}
