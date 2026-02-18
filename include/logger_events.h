#ifndef FF821ED1_B92A_42CD_9A5B_3D607C43E4E1
#define FF821ED1_B92A_42CD_9A5B_3D607C43E4E1

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "logger_common.h"

#define LOGGER_EVENT_BASE 0x50  // Component ID 5

// Declare an event base
ESP_EVENT_DECLARE_BASE(LOGGER_EVENT);        // declaration of the LOGGER_EVENT family
// declaration of the specific events under the BUTTON_EVENT family
#define LOGGER_EVENT_ENUM(l) LOGGER_EVENT_##l,

#define LOGGER_EVENT_LIST(l) \
    l(DATETIME_SET) \
    l(SCREEN_UPDATE_BEGIN) \
    l(SCREEN_UPDATE_END) \

// declaration of the specific events under the LOGGER_EVENT family
enum {                                       
    LOGGER_EVENT_LIST(LOGGER_EVENT_ENUM)
};

const char * logger_event_strings(int id);

#ifdef __cplusplus
}
#endif

#endif /* FF821ED1_B92A_42CD_9A5B_3D607C43E4E1 */
