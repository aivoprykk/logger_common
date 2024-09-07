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

#if (CONFIG_LOGGER_COMMON_LOG_LEVEL <= 2)

#ifndef LOG_INFO
#define LOG_INFO(a, b, ...) ESP_LOGI(a, b, __VA_ARGS__)
#endif
#ifndef MEAS_START
#define MEAS_START() uint64_t _start = (esp_timer_get_time())
#endif
#ifndef MEAS_END
#define MEAS_END(a, b, ...) \
    ESP_LOGI(a, b, __VA_ARGS__, (esp_timer_get_time() - _start))
#endif

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
