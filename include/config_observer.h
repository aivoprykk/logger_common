#ifndef CONFIG_OBSERVER_H
#define CONFIG_OBSERVER_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of observers that can be registered
#define CONFIG_MAX_OBSERVERS 8

// Callback function type for configuration changes
// Parameters: group (configuration group), index (item index within group)
typedef void (*config_change_callback_t)(size_t group, size_t index);

/**
 * @brief Add an observer callback for configuration changes
 * 
 * @param callback Function to call when configuration changes occur
 * @return true if observer was added successfully, false if list is full
 */
bool config_observer_add(config_change_callback_t callback);

/**
 * @brief Remove an observer callback
 * 
 * @param callback Function to remove from observer list
 * @return true if observer was removed, false if not found
 */
bool config_observer_remove(config_change_callback_t callback);

/**
 * @brief Notify all observers of a configuration change
 * 
 * @param group Configuration group that changed
 * @param index Item index within the group that changed
 */
void config_observer_notify(size_t group, size_t index);

/**
 * @brief Initialize the observer system
 */
void config_observer_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_OBSERVER_H */
