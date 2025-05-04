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
#define BIT_GET(var, bit)	( (var) & (1U << (uint8_t)(bit) ) )
#define GETBIT(var, bit)	(BIT_GET(var, bit) ? 1 : 0 )
// set a bit to 1 (bit 0-7)
#define BIT_SET(var, bit)	((var) |= (1U << (uint8_t)(bit)))
#define SETBIT(var, bit)	BIT_SET(var, bit)
// set a bit to 0 (bit 0-7)
#define BIT_CLR(var, bit)	((var) &= (~(1U << (uint8_t)(bit))))
#define CLRBIT(var, bit)	BIT_CLR(var, bit)
// toggle a bit (bit 0-7)
#define BIT_TGL(var, bit)	((var) ^= (1U << (uint8_t)(bit)))
#define FLIPBIT(var, bit)   BIT_TGL(var, bit)

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

#define NUM_GE_3_DIG(x) ((x) >= 100)
#define NUM_LT_3_DIG(x) ((x) < 100)
#define ONE_M_S_IN_MM_S ONE_K
#define FROM_K(x) ((x) / 1000.0f)
#define FROM_100K(x) ((x) / 100000.0f)
#define FROM_10M(x) ((x) / 10000000.0f)
#define TO_K_F(x) ((x) * 1000.0f)
#define TO_K_UL(x) ((x) * 1000UL)
#define TO_M(x) ((x) * 1000000.0f)
#define TO_M_UL(x) ((x) * 1000000UL)
#define SEC_TO_HOUR(x) ((x) / 3600.0f)
#define MM_S_TO_M_S(x) FROM_K(x)
#define MM_TO_M(x) FROM_K(x)
#define M_TO_KM(x) FROM_K(x)
#define MM_TO_KM(x) ((x) * 1000000.0f)
#define MS_TO_SEC(x) FROM_K(x)
#define SEC_TO_MS(x) TO_K_UL(x)
#define SEC_TO_US(x) TO_M_UL(x)
#define MS_TO_US(x) TO_K_UL(x)
#define M_TO_MM(x) TO_K_UL(x)

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
    hex_str[0] = hex_chars[(value >> 4u) & 0x0Fu]; // Extract high nibble
    hex_str[1] = hex_chars[value & 0x0Fu];        // Extract low nibble
    hex_str[2] = '\0';                           // Null-terminate the string
}

inline void uint32_to_uint8_array(uint32_t value, uint8_t array[4]) {
    array[3] = (uint8_t)((value >> 24u) & 0xFFu);
    array[2] = (uint8_t)((value >> 16u) & 0xFFu);
    array[1] = (uint8_t)((value >> 8u) & 0xFFu);
    array[0] = (uint8_t)(value & 0xFF);
}

inline void mac_to_char(uint8_t *mac, char *mac_str, uint8_t start) {
    uint8_t i = start, j = 6;
    for (;i < j; i++) {
        uint8_to_hex_string(mac[i], &mac_str[(i-start) * 2]);
    }
    mac_str[(j-start) * 2] = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* C62D2DBF_C1B7_4992_8A7A_15901F9CE78B */
