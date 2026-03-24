#ifndef BF19E034_3F73_4E8B_92A6_B8B07FFE87A2
#define BF19E034_3F73_4E8B_92A6_B8B07FFE87A2

#ifdef __cplusplus
extern "C" {
#endif
#include "config_manager.h"
// GPS configuration defines

// NVS keys for each module (moved from unified_config.c)
#define NVS_KEY_GPS "gps_cfg"

#define GPS_CFG_ENUM_PRE(l) cfg_gps_##l
#define GPS_CFG_ENUM_PRE_N(l) GPS_CFG_ENUM_PRE(l),
#define GPS_CFG_ENUM_PRE_L(l, m) .GPS_CFG_ENUM_PRE(l) = (uint8_t)(m),
#define GPS_STAT_S(a) uint8_t a;
#define GPS_STAT_BITFIELD(a) uint8_t a : 1;
#define GPS_STAT_BITFIELD_S(a) .##log_##a = 1,
#define GPS_LOG_BITFIELD(a) uint8_t log_##a : 1;
#define GPS_STAT_D(l) GPS_CFG_ENUM_PRE_L(l, 1)
#define CFG_ENUM_GPS(l) GPS_CFG_ENUM_PRE(l),

#define CFG_GPS_USER_FILE_ITEMS(l) l(log_txt) l(log_format) l(log_ubx_nav_sat)

#define CFG_GPS_ITEMS(l) l(timezone) l(speed_unit)

#define CFG_GPS_STAT_SCREENS_ITEMS(l) l(stat_screens)

#define CFG_GPS_USER_OTHER_ITEMS(l) l(file_date_time) l(ubx_file)

#define CFG_GPS_ITEM_LIST(l)                                                   \
	CFG_GPS_ITEMS(l)                                                           \
	CFG_GPS_STAT_SCREENS_ITEMS(l)                                              \
	CFG_GPS_USER_FILE_ITEMS(l)                                                 \
	CFG_GPS_USER_OTHER_ITEMS(l)
extern const char *const config_gps_items[];
extern const size_t config_gps_item_count;

#if defined(CONFIG_GPS_LOG_ENABLE_GPY)
#define GPS_LOG_HAS_GPY 1
#endif

#if defined(CONFIG_GPS_LOG_ENABLE_OAO)
#define GPS_LOG_HAS_OAO 1
#endif

// GPS specific value lists

#define SPEED_UNIT_VAL_LIST(l) l(msec) l(kmh) l(knot) l(mph)
extern const char *const speed_units[];
extern const size_t speed_units_items_count;

// Return the speed unit string for the current config, clamped to valid range
static inline const char *get_speed_unit_str(uint8_t unit_idx) {
	return speed_units[unit_idx < speed_units_items_count
		? unit_idx : speed_units_items_count - 1];
}

#define DYNAMIC_MODEL_ITEM_LIST(l)                                             \
	l(Portable) l(Pedestrian) l(Automotive) l(Sea)
extern const char *const dynamic_models[];
extern const size_t dynamic_models_items_count;

#define FILE_DATE_TIME_ITEM_LIST(l)                                            \
	l(name_mac_index) l(name_date_time) l(date_time_name)
extern const char *const file_date_time_items[];
extern const size_t file_date_time_items_count;

#define TIMEZONE_ITEM_LIST(l) l(UTC) l(UTC + 1) l(UTC + 2) l(UTC + 3)
extern const char *const timezone_items[];
extern const size_t timezone_items_count;

#if defined(GPS_LOG_HAS_OAO)
#define LOG_FORMAT_OAO_ITEM(l) l(oao)
#else
#define LOG_FORMAT_OAO_ITEM(l)
#endif

#if defined(GPS_LOG_HAS_GPY)
#define LOG_FORMAT_GPY_ITEM(l) l(gpy)
#else
#define LOG_FORMAT_GPY_ITEM(l)
#endif
#define LOG_FORMAT_STATIC_ITEM_LIST(l) l(sbp) l(ubx) l(gpx)
#define LOG_FORMAT_ITEM_LIST(l) LOG_FORMAT_STATIC_ITEM_LIST(l) LOG_FORMAT_OAO_ITEM(l) LOG_FORMAT_GPY_ITEM(l)
#define LOG_FORMAT_ENUM(l) log_format_##l,
enum log_format_items_e {
	LOG_FORMAT_ITEM_LIST(LOG_FORMAT_ENUM) log_format_max
};
extern const char *const log_format_items[];
extern const size_t log_format_items_count;

typedef enum { CFG_GPS_ITEM_LIST(GPS_CFG_ENUM_PRE_N) } gps_cfg_item_t;

typedef enum { SPEED_UNIT_VAL_LIST(ENUM) } speed_unit_item_t;

#define STAT_SCREEN_ITEM_LIST(l)                                               \
	l(stat_10_sec) l(stat_2_sec) l(stat_250_m) l(stat_500_m) l(stat_1852_m)    \
		l(stat_a500) l(stat_avg_10sec) l(stat_stat1) l(stat_stat2)             \
			l(stat_avg_a500)

extern const char *const gps_stat_screen_items[];
extern const size_t gps_stat_screen_item_count;

enum stat_screen_items_e { STAT_SCREEN_ITEM_LIST(ENUM) };

typedef union cfg_gps_stat_screens_u {
	struct {
		STAT_SCREEN_ITEM_LIST(GPS_STAT_BITFIELD)
	} bits;
	uint16_t value;
} cfg_gps_stat_screens_t;

#if defined(GPS_LOG_HAS_OAO)
#define GPS_LOG_OAO_FIELD uint8_t log_oao : 1;
#else
#define GPS_LOG_OAO_FIELD uint8_t reserved_oao : 1;
#endif

#if defined(GPS_LOG_HAS_GPY)
#define GPS_LOG_GPY_FIELD uint8_t log_gpy : 1;
#else
#define GPS_LOG_GPY_FIELD uint8_t reserved_gpy : 1;
#endif
typedef union cfg_gps_log_enables_u {
	struct {
		uint8_t log_txt : 1;
		LOG_FORMAT_STATIC_ITEM_LIST(GPS_LOG_BITFIELD)
		GPS_LOG_OAO_FIELD
		GPS_LOG_GPY_FIELD
		uint8_t log_ubx_nav_sat : 1;
	} bits;
	uint8_t value;
} cfg_gps_log_enables_t;

#define GPS_CFG_STAT_SCREENS_DEFAULTS()                                        \
	{STAT_SCREEN_ITEM_LIST(GPS_STAT_BITFIELD_S)}

#define GPS_UBX_FILE_MAX 24

typedef struct cfg_gps_s {
	// Version tracking for migration
	uint16_t version;
	// Byte-aligned fields (1 byte each)
	speed_unit_item_t speed_unit; // 0 = m/s, 1 = km/h, 2 = knots, 3 = mph
	uint8_t file_date_time;		  // 0=mac_index, 1=date_time, 2=date_time_name
	cfg_gps_log_enables_t
		log_enables; // Bitfield for GPS logging enables (packed)

	// 2-byte aligned field
	cfg_gps_stat_screens_t
		stat_screens; // Bitfield for enabled stat screens (uint16_t)

	// 4-byte aligned field (int8_t)
	int8_t timezone; // UTC offset in hours

	// String field (24 bytes)
	char ubx_file[GPS_UBX_FILE_MAX]; // UBX filename base
} __attribute__((packed, aligned(4))) cfg_gps_t;
// #define L_CONFIG_GPS_FIELDS sizeof(struct cfg_gps_s)

#define CFG_GPS_DEFAULTS()                                                     \
	{                                                                          \
		.version = 1,                                                          \
		.speed_unit = kmh,                                                     \
		.file_date_time = 2,                                                   \
		.log_enables.value = 0x03,                                             \
		.stat_screens.value = 0x01FF,                                          \
		.timezone = 2,                                                         \
		.ubx_file = "GPSDATA",                                                 \
	}

// extern cfg_gps_t c_gps_cfg;

// Conversion functions
inline float mm_s_to_km_h(uint32_t mm_s) {
	return (float)mm_s * 18.0f / 5000.0f;
}
inline float mm_s_to_knots(uint32_t mm_s) {
	return (float)mm_s * 463.0f / 238200.0f;
}
inline float mm_s_to_mph(uint32_t mm_s) {
	return (float)mm_s * 2237.0f / 100000.0f;
}
inline float mms_to_ms(uint32_t mm_s) { return (float)mm_s / 1000.0f; }

float convert_speed(float speed, speed_unit_item_t unit);

struct strbf_s;
bool config_gps_value_str(size_t index, struct strbf_s *sb, uint8_t *type);
uint8_t get_gps_item_values(size_t index, struct strbf_s *sb);
bool get_gps_item_descriptions(size_t index, struct strbf_s *sb);

bool config_gps_set_next_item(size_t index); // Cycle to next value (for UI)
// GPS group functions
bool config_gps_get_item(size_t index, config_item_info_t *info);
bool config_gps_set_item(size_t index, const char *value);
// GPS group helper functions
bool config_gps_set_next_value(size_t index);

bool config_stat_screen_set_next_value(int num);
bool config_stat_screen_get_item(int num, config_item_info_t *item);
int config_stat_screen_get_next_cycled(int idx);

uint8_t config_gps_handle(void);
uint8_t config_stat_screens_handle(void);
void config_gps_register(void);

#ifdef __cplusplus
}
#endif

#endif /* BF19E034_3F73_4E8B_92A6_B8B07FFE87A2 */
