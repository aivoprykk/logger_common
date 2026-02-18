#ifndef E4637772_1304_4CBE_8AE1_F6191216D7FD
#define E4637772_1304_4CBE_8AE1_F6191216D7FD

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "logger_common.h"

#define LOGGER_CONFIG_EVENT_BASE 0x60  // Component ID 6

ESP_EVENT_DECLARE_BASE(LOGGER_CONFIG_EVENT);        // declaration of the LOG_EVENT family
#define LOGGER_CONFIG_EVENT_ENUM(l) LOGGER_CONFIG_EVENT_##l,

#define LOGGER_CONFIG_EVENT_LIST(l) \
    l(INIT_DONE) \
    l(LOAD_DONE) \
    l(LOAD_FAIL) \
    l(SAVE_DONE) \
    l(SAVE_FAIL) \
    l(CFG_CHANGED) \
    l(CFG_SET) \
    l(CFG_GET)

// declaration of the specific events under the LOG_EVENT family
enum {                                       
    LOGGER_CONFIG_EVENT_LIST(LOGGER_CONFIG_EVENT_ENUM)
};

const char * logger_config_event_strings(int id);

#ifdef __cplusplus
}
#endif

#endif /* E4637772_1304_4CBE_8AE1_F6191216D7FD */
