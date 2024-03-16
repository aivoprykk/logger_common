#ifndef C62D2DBF_C1B7_4992_8A7A_15901F9CE78B
#define C62D2DBF_C1B7_4992_8A7A_15901F9CE78B

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "sdkconfig.h"

uint32_t get_millis(void);

void delay_ms(uint32_t ms);

#if defined(CONFIG_VERBOSE_BUILD)
#include "esp_log.h"
#include "esp_system.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG 1
#endif

#define LOGR ESP_LOGI(TAG, "[%s]", __FUNCTION__);
#define TIMER_S                                                 \
    uint32_t millis = get_millis(), emillis = 0; \
    LOGR
#define TIMER_E                                                                        \
    emillis = get_millis();                                             \
    ESP_LOGI(TAG, "[%s] --- took %" PRIu32 " ms ---", __FUNCTION__, emillis - millis); \
    printf("[%s] minimum_free_heap_size:%" PRIu32 " bytes\n", __FUNCTION__, esp_get_minimum_free_heap_size());

#else

#ifdef DEBUG
#undef DEBUG
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#define TIMER_S

#define TIMER_E

#define LOGR

#endif

#define QUOTE(x) #x
#define QUOTE_CMD(x) QUOTE(x)

#ifdef __cplusplus
}
#endif

#endif /* C62D2DBF_C1B7_4992_8A7A_15901F9CE78B */
