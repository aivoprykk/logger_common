/**
 * @file logger_fixed_pool.h
 * @brief Generic O(1) fixed-size block pool using a FreeRTOS queue free-list.
 *
 * Provides zero-heap-pressure alloc/free for hot paths (e.g. GPS write staging).
 * The caller owns the backing storage array; the pool manages a free-list of
 * pointers to individual blocks inside that array.
 *
 * Usage example:
 * @code
 *   typedef struct { uint8_t data[512]; size_t len; } my_block_t;
 *   static my_block_t   my_storage[32];
 *   static logger_fixed_pool_t my_pool;
 *
 *   // Init once:
 *   logger_fixed_pool_init(&my_pool, my_storage, sizeof(my_block_t), 32);
 *
 *   // Hot path (O(1), no malloc):
 *   my_block_t *blk = (my_block_t *)logger_fixed_pool_alloc(&my_pool);
 *   if (blk) { ... logger_fixed_pool_free(&my_pool, blk); }
 *
 *   // Teardown:
 *   logger_fixed_pool_deinit(&my_pool);
 * @endcode
 */

#ifndef LOGGER_FIXED_POOL_H
#define LOGGER_FIXED_POOL_H

#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pool handle.  Zero-initialise before calling logger_fixed_pool_init().
 */
typedef struct {
    QueueHandle_t free_queue;   ///< Free-list: stores void* pointers to available blocks
    size_t        block_size;   ///< Size of one block in bytes
    size_t        block_count;  ///< Total number of blocks
} logger_fixed_pool_t;

/**
 * @brief Initialise a fixed-size block pool.
 *
 * @param pool        Pool handle to initialise (must not be NULL).
 * @param storage     Caller-owned flat array of at least block_size * block_count bytes.
 * @param block_size  Size of each individual block in bytes.
 * @param block_count Number of blocks in the pool.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG or ESP_ERR_NO_MEM on failure.
 */
esp_err_t logger_fixed_pool_init(logger_fixed_pool_t *pool,
                                  void               *storage,
                                  size_t              block_size,
                                  size_t              block_count);

/**
 * @brief Deinitialise the pool and delete the internal FreeRTOS queue.
 *        Does NOT free the caller-owned storage array.
 *
 * @param pool Pool handle.
 */
void logger_fixed_pool_deinit(logger_fixed_pool_t *pool);

/**
 * @brief Borrow a block from the pool (non-blocking, O(1)).
 *
 * @param pool Pool handle.
 * @return Pointer to a free block, or NULL if the pool is exhausted.
 */
void *logger_fixed_pool_alloc(logger_fixed_pool_t *pool);

/**
 * @brief Return a block to the pool (non-blocking, O(1)).
 *
 * @param pool  Pool handle.
 * @param block Block previously obtained from logger_fixed_pool_alloc().
 */
void logger_fixed_pool_free(logger_fixed_pool_t *pool, void *block);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_FIXED_POOL_H */
