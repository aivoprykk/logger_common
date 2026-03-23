#ifndef C62D2DBF_C1B7_4992_8A7A_15901F9CE78B
#define C62D2DBF_C1B7_4992_8A7A_15901F9CE78B

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"
#include <stddef.h>
#include <stdint.h>

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
#define IS_ALNUM(x)                                                            \
	(((x) >= 'a' && (x) <= 'z')) || ((x) >= 'A' && (x) <= 'Z') ||              \
		(((x) >= '0' && (x) <= '9'))

#ifndef BIT
#define BIT(x) (1UL << (uint8_t)(x))
#endif

// BIT_CHECK get a bit from a variable (bit 0-7)
#ifndef BIT_GET
#define BIT_GET(var, nr) ((var) & BIT(nr))
#endif
#define GETBIT(var, nr) (BIT_GET(var, nr) ? 1 : 0)
// BIT_SET set a bit to 1 (bit 0-7)
#ifndef BIT_SET
#define BIT_SET(var, nr) ((var) |= BIT(nr))
#endif
#define SETBIT(var, nr) BIT_SET(var, nr)
// BIT_CLEAR a bit to 0 (bit 0-7)
#ifndef BIT_CLR
#define BIT_CLR(var, nr) ((var) &= (~BIT(nr)))
#endif
#define CLRBIT(var, nr) BIT_CLR(var, nr)
// BIT_FLIP toggle a bit (bit 0-7)
#ifndef BIT_TGL
#define BIT_TGL(var, nr) ((var) ^= BIT(nr))
#endif
#define FLIPBIT(var, nr) BIT_TGL(var, nr)

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
#define ADD(x) x | A_
#define ADD_QUOTE(x) STRINGIFY_(x)
#define JOIN_AGAIN(x, y) x##y
#define JOIN(x, y) JOIN_AGAIN(x, y)

#define lengthof(x) (sizeof(x) / sizeof((x)[0]))

enum units_e {
	TWO_SECONDS = 2,
	MS_50 = 50,
	QUATER_SEC_IN_MS = 250,
	HALF_SEC_IN_MS = 500,
	ONE_K = 1000,
	HALF_H_IN_SECS = 1800,
	ONE_H_IN_SECS = 3600
};

#define NUM_GE_3_DIG(x) ((x) >= 100)
#define NUM_LT_3_DIG(x) ((x) < 100)
#define ONE_M_S_IN_MM_S ONE_K
#define FROM_K(x) ((x) / 1000.0f)
#define FROM_K_UL(x) ((x) / 1000ul)
#define HZ_TO_MS(x) (1000ul / (x))
#define FROM_100K(x) ((x) / 100000.0f)
#define FROM_M(x) ((x) / 1000000.0f)
#define FROM_10M(x) ((x) / 10000000.0f)
#define TO_K_F(x) ((x) * 1000.0f)
#define TO_K_UL(x) ((x) * 1000UL)
#define TO_M(x) ((x) * 1000000.0f)
#define TO_M_UL(x) ((x) * 1000000UL)
#define SEC_TO_HOUR(x) ((x) / 3600.0f)
#define HOUR_TO_SEC(x) ((x) * 3600LL)
#define MM_S_TO_M_S(x) FROM_K(x)
#define MM_TO_M(x) FROM_K(x)
#define M_TO_KM(x) FROM_K(x)
#define MM_TO_KM(x) FROM_M(x)
#define KM_TO_MM(x) TO_M(x)
#define MS_TO_SEC(x) FROM_K(x)
#define SEC_TO_MS(x) TO_K_UL(x)
#define SEC_TO_US(x) TO_M_UL(x)
#define MS_TO_US(x) TO_K_UL(x)
#define M_TO_MM(x) TO_K_UL(x)
#define MS_TO_MM_S(x) TO_K_UL(x)
#define NANO_TO_MILLIS_ROUND(x) c_nano_to_millis_round((int32_t)(x))
#define NANO_TO_US_ROUND(x) c_nano_to_us_round((int32_t)(x))
#define LEAP_UTC_OFFSET 18 // seconds, as of 2025
#define POW_2(x) ((x) * (x))
#define METERS_PER_LATITUDE_DEGREE 111195.0f
#define DEG_TO_METERS(x) ((x) * POW_2(METERS_PER_LATITUDE_DEGREE))
#define EARTH_RADIUS_M 6371000.0
#define ROUND_UP_TO_8(x) (((x) + 7) & ~7U) // Align to nearest 8 bits
// alternative: #define BYTE_PADDING(w) (((w + 7u) >> 3u) << 3u)

typedef struct m_config_item_s {
	const char *name;
	int pos;
	uint32_t value;
	uint32_t min;
	uint32_t max;
	const char *desc;
} m_config_item_t;

struct tm;
struct timeval;
int32_t c_nano_to_millis_round(int32_t nano);
int32_t c_nano_to_us_round(int32_t nano);
void c_normalize_utc_fields(uint32_t *year, uint8_t *month, uint8_t *day,
							uint8_t *hour, uint8_t *minute, uint8_t *second,
							int32_t *subsecond, uint32_t units_per_second);
uint64_t c_utc_ms_from_date_time(uint32_t year, uint8_t month, uint8_t day,
							 uint8_t hour, uint8_t minute, uint8_t second,
							 int32_t millis, int32_t *normalized_millis);
struct tm *get_local_time(struct tm *info);
int c_set_time(struct tm *tm, uint32_t us, int8_t timezone);
int c_set_time_ts(int64_t sec, uint32_t us, int8_t timezone);
int c_set_time_ms(int64_t ms, uint32_t us, int8_t timezone);
struct tm *c_timeval_to_tm_utc(const struct timeval *tv, struct tm *result);

#if (C_LOG_LEVEL <= LOG_INFO_NUM)
esp_err_t task_memory_info(const char *task_name);
#else
#define task_memory_info(x) (void)(0)
#endif

#if (C_LOG_LEVEL <= LOG_DEBUG_NUM)
esp_err_t tasks_memory_info(void);
esp_err_t task_top(void);
#else
#define tasks_memory_info() (void)(0)
#define task_top() (void)(0)
#endif
#if (C_LOG_LEVEL <= LOG_WARN_NUM)
esp_err_t mem_info(void);
#else
#define mem_info() (void)(0)
#endif
int32_t smooth(const int32_t *array, const int32_t index, const uint32_t size,
			   const uint8_t window_size);

// unsigned long get_micros();
unsigned long get_millis();
#define DELAY_MS(x)                                                            \
	vTaskDelay((x + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS)
inline void delay_ms(uint32_t ms) { DELAY_MS(ms); }

inline void uint8_to_hex_string(uint8_t value, char *hex_str) {
	const char hex_chars[] = "0123456789ABCDEF";
	hex_str[0] = hex_chars[(value >> 4u) & 0x0Fu]; // Extract high nibble
	hex_str[1] = hex_chars[value & 0x0Fu];		   // Extract low nibble
	hex_str[2] = '\0';							   // Null-terminate the string
}

inline void uint32_to_uint8_array(uint32_t value, uint8_t array[4]) {
	array[3] = (uint8_t)((value >> 24u) & 0xFFu);
	array[2] = (uint8_t)((value >> 16u) & 0xFFu);
	array[1] = (uint8_t)((value >> 8u) & 0xFFu);
	array[0] = (uint8_t)(value & 0xFF);
}

inline void mac_to_char(uint8_t *mac, char *mac_str, uint8_t start) {
	uint8_t i = start, j = 6;
	for (; i < j; i++) {
		uint8_to_hex_string(mac[i], &mac_str[(i - start) * 2]);
	}
	mac_str[(j - start) * 2] = 0;
}

#define TOLOWER_CHAR(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 32) : (c))

inline void str_tolower(char *s) {
	while (*s) {
		if (*s >= 'A' && *s <= 'Z')
			*s += 32;
		s++;
	}
}

inline float get_distance_m(int distance, int output_rate) {
	return MM_TO_M(distance) / output_rate;
}

#ifdef __cplusplus
}
#endif

#endif /* C62D2DBF_C1B7_4992_8A7A_15901F9CE78B */
