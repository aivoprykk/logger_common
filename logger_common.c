
#include "logger_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

void delay_ms(uint32_t ms) {
    vTaskDelay((ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS);
}

uint32_t get_millis() {
    return esp_timer_get_time() / 1000;
}
