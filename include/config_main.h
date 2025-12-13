#ifndef B7E6AD5D_C654_4366_8929_8E54A61C71F4
#define B7E6AD5D_C654_4366_8929_8E54A61C71F4

#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"

#define CFG_MAIN_ITEM_LIST(l) \
l(bar_length) \
l(ssid) \
l(password) \
l(ssid1) \
l(password1) \
l(ssid2) \
l(password2) \
l(ssid3) \
l(password3) \
l(sleep_info)

extern const char * const config_main_items[];
extern const size_t config_main_item_count;

typedef struct cfg_wifi_sta_s {
    char ssid[32];        // your SSID
    char password[32];    // your password
} cfg_wifi_sta_t;

#define CFG_SSID_MAX 4

typedef struct cfg_main_s {
    uint16_t bar_length;         // Bar length in meters
    struct cfg_wifi_sta_s wifi_sta[CFG_SSID_MAX];
    char sleep_info[32];         // Sleep info string
} cfg_main_t;

#define CFG_MAIN_DEFAULTS() { \
    .bar_length = 1852, \
    .wifi_sta = { \
        { .ssid = "ssid1", .password = "password1" }, \
        { {0}, {0} }, \
        { {0}, {0} }, \
        { {0}, {0} }, \
    }, \
    .sleep_info = "ESP GPS", \
}

#define CFG_ENUM_MAIN(l) cfg_main_##l,
enum cfg_main_item_e {
    CFG_MAIN_ITEM_LIST(CFG_ENUM_MAIN)
};

struct strbf_s;
bool config_main_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
bool get_main_item_values(size_t index, struct strbf_s *sb);
bool get_main_item_descriptions(size_t index, struct strbf_s *sb);

#ifdef __cplusplus
}
#endif

#endif /* B7E6AD5D_C654_4366_8929_8E54A61C71F4 */
