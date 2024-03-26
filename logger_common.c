
#include "logger_common.h"

#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

void delay_ms(uint32_t ms) {
    vTaskDelay((ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS);
}

uint32_t get_millis() {
    return esp_timer_get_time() / 1000;
}

struct tm * getLocalTime(struct tm *info, uint32_t ms) {
    assert(info);
    time_t now = time(0);
    localtime_r(&now, info);
    return info;
}

esp_err_t task_memory_info(const char * task_name) {
    printf("*** Task [%s] stack High Water Mark: %u bytes ***\n", task_name, uxTaskGetStackHighWaterMark(NULL));
    return ESP_OK;
}

esp_err_t memory_info_large(const char * task_name) {
    multi_heap_info_t heap_info;
    heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
    printf("** Task: [%s] Free heap: %d bytes, minimum free heap size: %d bytes\n",task_name , heap_info.total_free_bytes, heap_info.minimum_free_bytes);
    const int min_free_8bit_cap = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    const int min_free_32bit_cap = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    printf("** Task: [%s] have had inimum free heap:  dram(8bit_cap): %d bytes, iram(32bit_cap): %d bytes\n", task_name, min_free_8bit_cap, (min_free_32bit_cap - min_free_8bit_cap));
    return ESP_OK;
}

static char msgbbb[280];
esp_err_t task_top() {
    vTaskGetRunTimeStats(&(msgbbb[0]));
    printf("** [%s]\n", msgbbb);
    return ESP_OK;
}

int32_t smooth(const int32_t * array, const int32_t index, const uint32_t size, const uint8_t window_size) {
    if(index < 0)
        return 0;
    if(window_size == 0) 
        return array[index%size];
    
    int32_t sum = 0, index_win = index - window_size, j; // move index_win to the left for window_size
    int8_t count = 0, i=-window_size;
    for (; i <= window_size; ++i) {
        j = index_win + i;
        sum += array[j % size];
        //printf("{i:%hhd, sum:%ld, count:%hhd, j:%ld, index_win:%ld, size:%lu, array_j:%ld}\n", i, sum, count, j, index_win, size, array[j % size]);
        count++;
    }
    //printf("aaaaaaaaaaaaaaaaa sum:%ld count:%hhd ret: %ld aaaaaaaaaaaaaaaaa\n", sum, count, count ? sum / count : 0);
    return count ? sum / count : 0;
}

int smooth_int(const int * array, const int32_t index, const uint32_t size, const uint8_t window_size) {
    if(index < 0)
        return 0;
    if(window_size == 0) 
        return array[index%size];
    
    int sum = 0, index_win = index - window_size, j; // move index_win to the left for window_size
    int8_t count = 0, i=-window_size;
    for (; i <= window_size; ++i) {
        j = index_win + i;
        sum += array[j % size];
        count++;
    }
    return count ? sum / count : 0;
}
