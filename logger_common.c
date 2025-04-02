
#include "logger_events.h"

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "esp_timer.h"

#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || CONFIG_LOGGER_GLOBAL_LOG_LEVEL < 2)
static const char * const _logger_event_strings[] = {
    LOGGER_EVENT_LIST(STRINGIFY)
};
const char * logger_event_strings(int id) {
    return _logger_event_strings[id];
}
#else
const char * logger_event_strings(int id) {
    return "LOGGER_EVENT";
}
#endif

struct tm * getLocalTime(struct tm *info, uint32_t ms) {
    assert(info);
    time_t now = time(0);
    localtime_r(&now, info);
    return info;
}

esp_err_t task_memory_info(const char * task_name) {
    printf("*** Task %s (%s) stack High Water Mark: %u bytes ***\n", pcTaskGetName(NULL), task_name, uxTaskGetStackHighWaterMark(NULL));
    return ESP_OK;
}

#if (C_LOG_LEVEL < 3)
esp_err_t tasks_memory_info() {
    char str[40*uxTaskGetNumberOfTasks()+1];
    vTaskList(&(str[0]));
    printf("******\n%s", str);
    return ESP_OK;
}
esp_err_t task_top() {
    char str[40*uxTaskGetNumberOfTasks()+1];
    vTaskGetRunTimeStats(&(str[0]));
    printf("******\n%s", str);
    return ESP_OK;
}
#endif

// unsigned long IRAM_ATTR get_micros() { return (unsigned long) (esp_timer_get_time()); }
unsigned long IRAM_ATTR get_millis() { return (unsigned long) (esp_timer_get_time() / 1000); }

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
