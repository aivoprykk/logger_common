#ifndef CFB98B9D_63A6_4EF8_A2B4_D928EABBDBF5
#define CFB98B9D_63A6_4EF8_A2B4_D928EABBDBF5


#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"
#if defined (CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG)

#define SCFG_GRP_ADMIN(l) l(ADMIN)

// Admin configuration list
#define CFG_ADMIN_ITEM_LIST(l) \
    l(admin_auth) \
    l(admin_username) \
    l(admin_password)

extern const char * const config_admin_items[];
extern const size_t config_admin_item_count;

typedef struct cfg_admin_s {
    uint16_t version;
    // Add main configuration items here
    uint8_t admin_auth;
    char admin_username[32];
    char admin_password[64];
} __attribute__((packed, aligned(4))) cfg_admin_t;

#define CFG_ADMIN_DEFAULTS() { \
    .version = 1, \
    .admin_auth = 0, \
    .admin_username = "", \
    .admin_password = "" \
}

#define CFG_ENUM_ADMIN(l) cfg_admin_##l,
enum cfg_admin_item_e {
    CFG_ADMIN_ITEM_LIST(CFG_ENUM_ADMIN)
};

struct strbf_s;
bool config_admin_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
uint8_t get_admin_item_values(size_t index, struct strbf_s *sb);
bool get_admin_item_descriptions(size_t index, struct strbf_s *sb);

#else
#define SCFG_GRP_ADMIN(l)
#endif // CONFIG_LOGGER_COMMON_ENABLE_ADMIN_CONFIG

#ifdef __cplusplus
}
#endif
#endif /* CFB98B9D_63A6_4EF8_A2B4_D928EABBDBF5 */