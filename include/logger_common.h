#ifndef C62D2DBF_C1B7_4992_8A7A_15901F9CE78B
#define C62D2DBF_C1B7_4992_8A7A_15901F9CE78B

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"

#include "esp_err.h"

#if (defined(CONFIG_LOGGER_BUILD_MODE_DEV) && !defined(DEBUG))
#define DEBUG
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

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x),
#define STRINGIFY_V(x, y) STRINGIFY_(x),
#define STRINGIFY_L(x, y) STRINGIFY_(y),
#define STRINGIFY_M(x, y, z) STRINGIFY_(y),
#define NUMERIFY_V(x, y) y,
#define NUMERIFY_VV(x, y, z) z,
#define ENUM(x) x,
#define ENUM_V(x, y) x = y,
#define ENUM_VV(x, y, z) x##_##y = z,
#define A_
#define ADD(x) x|A_
#define ADD_QUOTE(x) STRINGIFY_(x)
#define JOIN_AGAIN(x, y) x ## y
#define JOIN(x, y) JOIN_AGAIN(x, y)

#define lengthof(x) (sizeof(x) / sizeof((x)[0]))

typedef struct m_config_item_s {
    const char * name;
    int pos;
    uint32_t value;
    const char *desc;
} m_config_item_t;

#ifndef getLocalTime
struct tm;
struct tm *getLocalTime(struct tm *info, uint32_t ms);
#endif

esp_err_t task_memory_info(const char * task_name);
#if (C_LOG_LEVEL < 3  || defined(DEBUG))
esp_err_t tasks_memory_info();
#endif
esp_err_t task_top(void);
int32_t smooth(const int32_t * array, const int32_t index, const uint32_t size, const uint8_t window_size);

// unsigned long get_micros();
unsigned long get_millis();
#define DELAY_MS(x) vTaskDelay((ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS)
inline void delay_ms(uint32_t ms) { DELAY_MS(ms); }

inline void uint8_to_hex_string(uint8_t value, char *hex_str) {
    const char hex_chars[] = "0123456789ABCDEF";
    hex_str[0] = hex_chars[(value >> 4) & 0x0F]; // Extract high nibble
    hex_str[1] = hex_chars[value & 0x0F];        // Extract low nibble
    hex_str[2] = '\0';                           // Null-terminate the string
}

inline void uint32_to_uint8_array(uint32_t value, uint8_t array[4]) {
    array[3] = (uint8_t)((value >> 24) & 0xFF);
    array[2] = (uint8_t)((value >> 16) & 0xFF);
    array[1] = (uint8_t)((value >> 8) & 0xFF);
    array[0] = (uint8_t)(value & 0xFF);
}

inline void mac_to_char(uint8_t *mac, char *mac_str, uint8_t start) {
    uint8_t i = start, j = 6;
    for (uint8_t i = start; i < j; i++) {
        uint8_to_hex_string(mac[i], &mac_str[(i-start) * 2]);
    }
    mac_str[(j-start) * 2] = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* C62D2DBF_C1B7_4992_8A7A_15901F9CE78B */
