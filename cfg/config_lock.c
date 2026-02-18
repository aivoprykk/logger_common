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
        vSemaphoreDelete(config_mutex);
        config_mutex = NULL;
    }
}