
#include "logger_events.h"

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

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

int c_set_time_ts(int64_t sec, uint32_t us, float timezone) {
    if(sec > 1672531200) { /// 2023-01-01
        struct timeval tv = { .tv_sec = (sec + (int64_t)HOUR_TO_SEC(timezone)), .tv_usec = us };
        // struct timeval tv = { .tv_sec = sec, .tv_usec = 0 };
        // printf("[%s]: sec:%lld, tv_sec:%lld, tv_usec:%ld, tzsec: %lld\n", __func__, sec, tv.tv_sec, tv.tv_usec, (int64_t)HOUR_TO_SEC(timezone));
        return settimeofday(&tv, NULL);
    }
    return -2;
}

int c_set_time(struct tm *tm, uint32_t us, float timezone) {
    if(tm && tm->tm_year > 123) { /// > 2023
        return c_set_time_ts(mktime(tm), us, timezone);
    }
    return -2;
}

int c_set_time_ms(int64_t ms, uint32_t us, float timezone) {
    return c_set_time_ts((time_t)FROM_K_UL(ms), us, timezone);
}

struct tm * c_timeval_to_tm_utc(const struct timeval *tv, struct tm *result) {
    time_t sec = tv->tv_sec;
    // printf("[%s] tv_sec:%lld, tv_usec:%ld\n", __func__, sec, tv->tv_usec);
    return gmtime_r(&sec, result);
}

struct tm * get_local_time(struct tm *timeinfo) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    c_timeval_to_tm_utc(&tv, timeinfo);
    return timeinfo;
}

esp_err_t task_memory_info(const char * task_name) {
    printf("*** Task %s (%s) stack High Water Mark: %u bytes ***\n", pcTaskGetName(NULL), task_name, uxTaskGetStackHighWaterMark(NULL));
    return ESP_OK;
}

esp_err_t mem_info(void) {
    printf("*** Heap: free: %lu b, minfree: %lu b, internal: %zu / %zu b, external: %zu / %zu b ***\n", 
        esp_get_free_heap_size(), 
        esp_get_minimum_free_heap_size(), 
        heap_caps_get_total_size(MALLOC_CAP_INTERNAL), 
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
        heap_caps_get_total_size(MALLOC_CAP_SPIRAM), 
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
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
