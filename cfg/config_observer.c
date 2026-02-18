#include "common_private.h"

static const char *TAG = "config_observer";

// Observer list to store registered callbacks
static config_change_callback_t g_observers[CONFIG_MAX_OBSERVERS];
static size_t g_observer_count = 0;

void config_observer_init(void) {
    memset(g_observers, 0, sizeof(g_observers));
    g_observer_count = 0;
    ILOG(TAG, "Observer system initialized");
}

bool config_observer_add(config_change_callback_t callback) {
    if (!callback) {
        WLOG(TAG, "Cannot add NULL callback");
        return false;
    }

    // Check if already registered
    for (size_t i = 0; i < g_observer_count; i++) {
        if (g_observers[i] == callback) {
            WLOG(TAG, "Callback already registered");
            return true; // Already registered, consider it success
        }
    }

    // Add new observer
    if (g_observer_count < CONFIG_MAX_OBSERVERS) {
        g_observers[g_observer_count++] = callback;
        ILOG(TAG, "Observer added (total: %u)", g_observer_count);
        return true;
    }

    ELOG(TAG, "Observer list full (max: %d)", CONFIG_MAX_OBSERVERS);
    return false;
}

bool config_observer_remove(config_change_callback_t callback) {
    if (!callback) {
        return false;
    }

    // Find and remove the callback
    for (size_t i = 0; i < g_observer_count; i++) {
        if (g_observers[i] == callback) {
            // Shift remaining observers down
            for (size_t j = i; j < g_observer_count - 1; j++) {
                g_observers[j] = g_observers[j + 1];
            }
            g_observers[--g_observer_count] = NULL;
            ILOG(TAG, "Observer removed (remaining: %u)", g_observer_count);
            return true;
        }
    }

    WLOG(TAG, "Observer not found");
    return false;
}

void config_observer_notify(size_t group, size_t index) {
    DLOG(TAG, "Notifying %u observers: group=%u, index=%u", g_observer_count, group, index);

    for (size_t i = 0; i < g_observer_count; i++) {
        if (g_observers[i]) {
            g_observers[i](group, index);
        }
    }
}
