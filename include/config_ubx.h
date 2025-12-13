#ifndef DF3A3022_A815_42FF_BE6A_B9BDEC4FE8A3
#define DF3A3022_A815_42FF_BE6A_B9BDEC4FE8A3

#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"

#define UBX_GNSS_LIST(l) l(UBX_GNSS_GPS, 0x00) l(UBX_GNSS_SBAS, 0x01) l(UBX_GNSS_GALILEO, 0x02) l(UBX_GNSS_BEIDOU, 0x03) l(UBX_GNSS_QZSS, 0x05) l(UBX_GNSS_GLONASS, 0x06) l(UBX_GNSS_NAVIC, 0x07)
typedef enum ubx_gnss_e {
    UBX_GNSS_LIST(ENUM_V)
} ubx_gnss_t;

#define GNSS_DESC_VAL_LIST(l) l(111) l(107) l(103) l(47) l(99) l(43) l(39)
extern const uint8_t gnss_desc_val[];

#define GNSS_DESC_ITEM_LIST(l) \
    l(G + E + B + R) \
    l(G + B + R) \
    l(G + E + R) \
    l(G + E + B) \
    l(G + R) \
    l(G + B) \
    l(G + E)
extern const char * const gnss_desc[];
extern const size_t gnss_desc_items_count;

#define UBX_NAV_MODE_LIST(l) \
    l(UBX_MODE_PORTABLE, 0) \
    l(UBX_MODE_PEDESTRIAN, 3) \
    l(UBX_MODE_AUTOMOTIVE, 4) \
    l(UBX_MODE_SEA, 5) \
    l(UBX_MODE_AIR_1G_MAX, 6) \
    l(UBX_MODE_AIR_2G_MAX, 7) \
    l(UBX_MODE_AIR_4G_MAX, 8)
typedef enum ubx_nav_mode_e {
    UBX_NAV_MODE_LIST(ENUM_V)
} ubx_nav_mode_t;

#define UBX_BAUD_RATE_LIST(l) l(UBX_BAUD_9600, 9600) l(UBX_BAUD_38400, 38400) l(UBX_BAUD_57600, 57600) l(UBX_BAUD_115200, 115200) l(UBX_BAUD_230400, 230400)
typedef enum ubx_baud_rate_e {
    UBX_BAUD_RATE_LIST(ENUM_V)
} ubx_baud_rate_t;
#define BAUD_RATE_COUNT 5
#define UBX_BAUD_RATE_DEFAULT UBX_BAUD_38400

#define SAMPLE_RATE_VAL_LIST(l) l(1) l(2) l(5) l(10) l(16) l(20)
#define SAMPLE_RATE_ITEM_LIST(l) l(1_Hz) l(2_Hz) l(5_Hz) l(10_Hz) l(16_Hz) l(20_Hz)
extern const char * const sample_rates[];
extern const size_t sample_rates_items_count;
/**
 * @brief Message output rate enum.
 */
#define UBX_OUTPUT_RATES_LIST(l) l(UBX_OUTPUT_1HZ, 0x01) l(UBX_OUTPUT_2HZ, 0x02) l(UBX_OUTPUT_5HZ, 0x05) l(UBX_OUTPUT_10HZ, 0x0a) l(UBX_OUTPUT_16HZ, 0x10) l(UBX_OUTPUT_20HZ, 0x14)
typedef enum ubx_output_rate_e {
    UBX_OUTPUT_RATES_LIST(ENUM_V)
} ubx_output_rate_t;
#define OUTPUT_RATE_COUNT 6
#define UBX_OUTPUT_RATE_DEFAULT UBX_OUTPUT_10HZ

/**
 * @brief NMEA Messages enum.
 */
#define UBX_MESSAGE_LIST(l) l(UBX_MSG_DTM, 0x0A) l(UBX_MSG_GBS, 0x09) l(UBX_MSG_GGA, 0x00) l(UBX_MSG_GLL, 0x01) l(UBX_MSG_GRS, 0x06) l(UBX_MSG_GSA, 0x02) l(UBX_MSG_GST, 0x07) l(UBX_MSG_GSV, 0x03) l(UBX_MSG_RMC, 0x04) l(UBX_MSG_VTG, 0x05) l(UBX_MSG_ZDA, 0x08)
typedef enum ubx_message_e {
    UBX_MESSAGE_LIST(ENUM_V)
} ubx_message_t;

#define UBX_MSG_TYPE_LIST(l) l(MT_NONE, 0x00) l(MT_NAV_DUMMY, 0x01) l(MT_NAV_PVT, 0x02) l(MT_NAV_ACK, 0x03) l(MT_NAV_NACK, 0x04) l(MT_NAV_ID, 0x05) l(MT_MON_GNSS, 0x06) l(MT_NAV_DOP, 0x07) l(MT_MON_VER, 0x08) l(MT_NAV_SAT, 0x09)
typedef enum ubx_msg_type_e {
    UBX_MSG_TYPE_LIST(ENUM_V)
} ubx_msg_type_t;

#define UBX_EN_PIN_LEN 4

#define CFG_UBX_PRIVATE_ITEM_LIST(l) \
    l(ubx_baud) \
    l(ubx_msgout_sat)

#define CFG_UBX_PUBLIC_ITEM_LIST(l) \
    l(ubx_output_rate) \
    l(ubx_nav_mode) \
    l(ubx_gnss) \

#define CFG_UBX_ITEM_LIST(l) \
    CFG_UBX_PUBLIC_ITEM_LIST(l)
    //CFG_UBX_PRIVATE_ITEM_LIST(l)

extern const char * const config_ubx_items[];
extern const size_t config_ubx_item_count;

typedef struct cfg_ubx_s {
    int baud;
    ubx_output_rate_t output_rate;
    ubx_nav_mode_t nav_mode;
    // uint8_t nav_mode_auto;
    uint8_t gnss;
    bool msgout_sat;
} cfg_ubx_t;

#define CFG_UBX_DEFAULTS() {  \
    .baud = UBX_BAUD_115200,        \
    .output_rate = UBX_OUTPUT_5HZ, \
    .nav_mode = UBX_MODE_PEDESTRIAN,  \
    .gnss = 111, /* 01101111 */      \
    .msgout_sat = true,            \
}

#define CFG_ENUM_UBX(l) cfg_ubx_##l,
enum cfg_ubx_item_e {
    CFG_UBX_ITEM_LIST(CFG_ENUM_UBX)
};

struct strbf_s;
bool config_ubx_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
bool get_ubx_item_values(size_t index, struct strbf_s *sb);
bool get_ubx_item_descriptions(size_t index, struct strbf_s *sb);
uint8_t config_ubx_get_next_value(size_t index);
bool config_ubx_set_next_value(size_t index); // Cycle to next value (for UI)

#ifdef __cplusplus
}
#endif

#endif /* DF3A3022_A815_42FF_BE6A_B9BDEC4FE8A3 */
