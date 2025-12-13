#ifndef CONFIG_LOCK_H
#define CONFIG_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Initialize the centralized configuration lock
 *
 * Creates a mutex semaphore for synchronizing access to configuration data
 * across all components. Must be called during system initialization.
 */
void config_lock_init(void);

/**
 * @brief Acquire the configuration lock
 *
 * @param timeout_ms Timeout in milliseconds, or -1 for portMAX_DELAY
 * @return true if lock acquired successfully, false on timeout
 */
bool config_lock(int timeout_ms);

/**
 * @brief Release the configuration lock
 */
void config_unlock(void);

/**
 * @brief Deinitialize the configuration lock
 *
 * Deletes the semaphore. Should be called during system shutdown.
 */
void config_lock_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LOCK_H */