#include "config_lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define TAG "config_lock"

static SemaphoreHandle_t config_mutex = NULL;

void config_lock_init(void) {
    // ILOG(TAG, "Initializing configuration lock");
    if (!config_mutex) {
        config_mutex = xSemaphoreCreateMutex();
        if (!config_mutex) {
            ESP_LOGE(TAG, "Failed to create config mutex");
        }
    }
}

bool config_lock(int timeout_ms) {
    // ILOG(TAG, "Acquiring configuration lock with timeout %d ms", timeout_ms);
    if (!config_mutex) {
        ESP_LOGW(TAG, "Config lock not initialized");
        return false;
    }

    TickType_t ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake(config_mutex, ticks);

    if (result != pdTRUE) {
        ESP_LOGW(TAG, "Config lock timeout after %d ms", timeout_ms);
    }

    return result == pdTRUE;
}

void config_unlock(void) {
    // ILOG(TAG, "Releasing configuration lock");
    if (config_mutex) {
        xSemaphoreGive(config_mutex);
    } else {
        ESP_LOGW(TAG, "Config lock not initialized");
    }
}

void config_lock_deinit(void) {
    // ILOG(TAG, "Deinitializing configuration lock");
    if (config_mutex) {
        /* Take the mutex with a short timeout before deleting. This drains any
         * task that is currently holding it, preventing FreeRTOS from walking
         * a corrupted blocked-task list after vSemaphoreDelete(). If a task is
         * blocked with portMAX_DELAY this will timeout, but that would indicate
         * a bug in shutdown ordering (a subsystem task still running). */
        if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
            ESP_LOGW(TAG, "config_lock_deinit: mutex still held at shutdown - "
                         "possible task still running");
        }
        vSemaphoreDelete(config_mutex);
        config_mutex = NULL;
    }
}