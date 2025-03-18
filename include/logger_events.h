#ifndef FF821ED1_B92A_42CD_9A5B_3D607C43E4E1
#define FF821ED1_B92A_42CD_9A5B_3D607C43E4E1

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "logger_common.h"

// Declare an event base
ESP_EVENT_DECLARE_BASE(LOGGER_EVENT);        // declaration of the LOGGER_EVENT family

#define LOGGER_EVENT_LIST(l) \
    l(LOGGER_EVENT_DATETIME_SET) \
    l(LOGGER_EVENT_SCREEN_UPDATE_BEGIN) \
    l(LOGGER_EVENT_SCREEN_UPDATE_END) \

// declaration of the specific events under the LOGGER_EVENT family
enum {                                       
    LOGGER_EVENT_LIST(ENUM)
};

const char * logger_event_strings(int id);

#ifdef __cplusplus
}
#endif

#endif /* FF821ED1_B92A_42CD_9A5B_3D607C43E4E1 */
