#ifndef B7E6AD5D_C654_4366_8929_8E54A61C71F4
#define B7E6AD5D_C654_4366_8929_8E54A61C71F4

#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"

#define SCFG_GRP_WIFI(l) l(WIFI)

#define CFG_WIFI_ITEM_LIST(l) \
l(ssid) \
l(password) \
l(ssid1) \
l(password1) \
l(ssid2) \
l(password2) \
l(ssid3) \
l(password3)

extern const char * const config_main_items[];
extern const size_t config_main_item_count;

typedef struct cfg_wifi_sta_s {
    char ssid[32];        // your SSID
    char password[32];    // your password
} cfg_wifi_sta_t;

#define CFG_SSID_MAX 4

typedef struct cfg_main_s {
    uint16_t version;
     struct cfg_wifi_sta_s wifi_sta[CFG_SSID_MAX];
} __attribute__((packed, aligned(4))) cfg_main_t;

#define CFG_MAIN_DEFAULTS() { \
    .version = 1, \
    .wifi_sta = { \
        { .ssid = "ssid1", .password = "password1" }, \
        { {0}, {0} }, \
        { {0}, {0} }, \
        { {0}, {0} }, \
    }, \
}

#define CFG_ENUM_WIFI(l) cfg_main_##l,
enum cfg_wifi_item_e {
    CFG_WIFI_ITEM_LIST(CFG_ENUM_WIFI)
};

struct strbf_s;
bool config_main_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
uint8_t get_main_item_values(size_t index, struct strbf_s *sb);
bool get_main_item_descriptions(size_t index, struct strbf_s *sb);

#ifdef __cplusplus
}
#endif

#endif /* B7E6AD5D_C654_4366_8929_8E54A61C71F4 */
