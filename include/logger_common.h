#ifndef C62D2DBF_C1B7_4992_8A7A_15901F9CE78B
#define C62D2DBF_C1B7_4992_8A7A_15901F9CE78B

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_err.h"

uint32_t get_millis(void);

void delay_ms(uint32_t ms);

#ifndef getLocalTime
struct tm;
struct tm *getLocalTime(struct tm *info, uint32_t ms);
#endif

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
#define TIMER_INIT static uint32_t millis = 0, emillis = 0;
#define TIMER_S millis = get_millis(); LOGR

#define TIMER_E                                                                        \
    emillis = get_millis();                                             \
    ESP_LOGI(TAG, "--- [%s] took %lu ms ---", __FUNCTION__, emillis - millis); /* \
    printf("[%s] minimum_free_heap_size:%" PRIu32 " bytes\n", __FUNCTION__, esp_get_minimum_free_heap_size()); */

#else

#ifdef DEBUG
#undef DEBUG
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#define TIMER_S

#define TIMER_E

#define TIMER_INIT

#define LOGR

#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif
#define QUOTE(x) #x
#define QUOTE_CMD(x) QUOTE(x)

// check if a character is a digit
#define IS_ALNUM(x) (((x)>='a' && (x) <= 'z')) ||((x)>='A' && (x) <= 'Z') || (((x)>='0' && (x) <= '9'))

// get a bit from a variable (bit 0-7)
#define GETBIT(var, bit)	(( var & (1 << (bit) ) ) ? 1 : 0 )

// set a bit to 1 (bit 0-7)
#define SETBIT(var, bit)	(var |= (1 << (bit)))

// set a bit to 0 (bit 0-7)
#define CLRBIT(var, bit)	(var &= (~(1 << (bit))))


esp_err_t task_memory_info(const char * task_name);
esp_err_t memory_info_large(const char * task_name);
esp_err_t task_top(void);
int32_t smooth(const int32_t * array, const int32_t index, const uint32_t size, const uint8_t window_size);
int smooth_int(const int * array, const int32_t index, const uint32_t size, const uint8_t window_size);

#ifdef __cplusplus
}
#endif

#endif /* C62D2DBF_C1B7_4992_8A7A_15901F9CE78B */
