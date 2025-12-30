#ifndef BF19E034_3F73_4E8B_92A6_B8B07FFE87A2
#define BF19E034_3F73_4E8B_92A6_B8B07FFE87A2

#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"

// struct gps_user_cfg_evt_data_s {
    // int pos;
    // int value;
// };

// GPS configuration defines

#define GPS_CFG_ENUM_PRE(l) cfg_gps_##l
#define GPS_CFG_ENUM_PRE_N(l)  GPS_CFG_ENUM_PRE(l),
#define GPS_CFG_ENUM_PRE_L(l, m) .GPS_CFG_ENUM_PRE(l) = (uint8_t)(m),
#define GPS_STAT_S(a) uint8_t a;
#define GPS_STAT_D(l) GPS_CFG_ENUM_PRE_L(l, 1)
#define CFG_ENUM_GPS(l) GPS_CFG_ENUM_PRE(l),

#if defined (CONFIG_GPS_LOG_GPY)
#define CFG_GPS_USER_FILE_ITEMS_LL(l) l(log_gpy)
#else
#define CFG_GPS_USER_FILE_ITEMS_LL(l)
#endif

#define CFG_GPS_USER_FILE_ITEMS(l) \
 l(log_txt) \
 l(log_ubx) \
 l(log_sbp) \
 l(log_gpx) \
 CFG_GPS_USER_FILE_ITEMS_LL(l) \
 l(log_ubx_nav_sat)
 
#define CFG_GPS_ITEMS(l) l(timezone) l(speed_unit)

#define CFG_GPS_STAT_SCREENS_ITEMS(l) l(stat_screens)
 
#define CFG_GPS_USER_OTHER_ITEMS(l) l(file_date_time) l(ubx_file)

#define CFG_GPS_ITEM_LIST(l) \
 CFG_GPS_ITEMS(l) \
 CFG_GPS_STAT_SCREENS_ITEMS(l) \
 CFG_GPS_USER_FILE_ITEMS(l) \
 CFG_GPS_USER_OTHER_ITEMS(l)
extern const char * const config_gps_items[];
extern const size_t config_gps_item_count;

// GPS specific value lists

#define SPEED_UNIT_VAL_LIST(l) l(msec) l(kmh) l(knot) l(mph)
extern const char * const speed_units[];
extern const size_t speed_units_items_count;

#define DYNAMIC_MODEL_ITEM_LIST(l) l(Portable) l(Pedestrian) l(Automotive) l(Sea)
extern const char * const dynamic_models[];
extern const size_t dynamic_models_items_count;

#define FILE_DATE_TIME_ITEM_LIST(l) l(name_mac_index) l(name_date_time) l(date_time_name)
extern const char * const file_date_time_items[];
extern const size_t file_date_time_items_count;

#define TIMEZONE_ITEM_LIST(l) l(UTC) l(UTC+1) l(UTC+2) l(UTC+3)
extern const char * const timezone_items[];
extern const size_t timezone_items_count;

typedef enum {
    CFG_GPS_ITEM_LIST(GPS_CFG_ENUM_PRE_N)
} gps_cfg_item_t;


typedef enum {
    SPEED_UNIT_VAL_LIST(ENUM)
} speed_unit_item_t;

#define STAT_SCREEN_ITEM_LIST(l) \
    l(stat_10_sec) \
    l(stat_2_sec) \
    l(stat_250_m) \
    l(stat_500_m) \
    l(stat_1852_m) \
    l(stat_a500) \
    l(stat_avg_10sec) \
    l(stat_stat1) \
    l(stat_stat2) \
    l(stat_avg_a500)
extern const char * const gps_stat_screen_items[];
extern const size_t gps_stat_screen_item_count;

enum stat_screen_items_e {
    STAT_SCREEN_ITEM_LIST(ENUM)
};

typedef struct cfg_gps_stat_screens_s {
    STAT_SCREEN_ITEM_LIST(GPS_STAT_S)
} cfg_gps_stat_screens_t;

#define GPS_CFG_STAT_SCREENS_DEFAULTS() { \
    STAT_SCREEN_ITEM_LIST(GPS_STAT_D) \
}

#define GPS_UBX_FILE_MAX 24

typedef struct cfg_gps_s {
    speed_unit_item_t speed_unit;      // 0 = m/s, 1 = km/h, 2 = knots    
    float timezone;          // choice for timedifference in hours with UTC, for Belgium 1 or 2 (summertime)
    
    // GPS logging enables
    bool log_txt;
    bool log_ubx;
    bool log_sbp;
    bool log_gpx;
    bool log_gpy;
    bool log_ubx_nav_sat;
    
    // GPS file settings
    uint8_t file_date_time;  // 0=mac_index, 1=date_time, 2=date_time_name
    char ubx_file[GPS_UBX_FILE_MAX];    // UBX filename base
    
    // Statistics screen configuration
    uint16_t stat_screens;    // Bitfield for enabled stat screens
} cfg_gps_t;
// #define L_CONFIG_GPS_FIELDS sizeof(struct cfg_gps_s)

#define CFG_GPS_DEFAULTS() { \
    .speed_unit = kmh, \
    .timezone = 2, \
    .log_txt = true, \
    .log_ubx = false, \
    .log_sbp = true, \
    .log_gpx = true, \
    .log_gpy = true, \
    .log_ubx_nav_sat = false, \
    .file_date_time = 1, \
    .ubx_file = "GPSDATA", \
    .stat_screens = 0x01FF, \
}

extern cfg_gps_t c_gps_cfg;

// Conversion functions
inline float mm_s_to_km_h(uint32_t mm_s) { return (float)mm_s * 18.0f / 5000.0f; }
inline float mm_s_to_knots(uint32_t mm_s) { return (float)mm_s * 463.0f / 238200.0f; }
inline float mm_s_to_mph(uint32_t mm_s) { return (float)mm_s * 2237.0f / 100000.0f; }
inline float mms_to_ms(uint32_t mm_s) { return (float)mm_s / 1000.0f; }

inline float convert_speed(float speed, speed_unit_item_t unit) {
    switch(unit) {
        case kmh: return mm_s_to_km_h(speed);
        case knot: return mm_s_to_knots(speed);
        case mph: return mm_s_to_mph(speed);
        default: return mms_to_ms(speed);
    }
}

struct strbf_s;
bool config_gps_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
bool get_gps_item_values(size_t index, struct strbf_s *sb);
bool get_gps_item_descriptions(size_t index, struct strbf_s *sb);

uint16_t config_stat_screen_get_next_value(int num);
bool config_stat_screen_set_next_value(int num);
uint8_t config_gps_get_next_value(size_t index);
bool config_gps_set_next_item(size_t index); // Cycle to next value (for UI)
int config_stat_screen_get_next_cycled(int current);

#ifdef __cplusplus
}
#endif

#endif /* BF19E034_3F73_4E8B_92A6_B8B07FFE87A2 */
