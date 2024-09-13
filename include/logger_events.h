#ifndef FF821ED1_B92A_42CD_9A5B_3D607C43E4E1
#define FF821ED1_B92A_42CD_9A5B_3D607C43E4E1

#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "logger_common.h"

// Declare an event base
ESP_EVENT_DECLARE_BASE(LOGGER_EVENT);        // declaration of the LOGGER_EVENT family

#define LOGGER_EVENT_LIST(l) \
    l(LOGGER_EVENT_DATETIME_SET) \
    l(LOGGER_EVENT_SDCARD_MOUNTED) \
    l(LOGGER_EVENT_SDCARD_MOUNT_FAILED) \
    l(LOGGER_EVENT_SDCARD_UNMOUNTED) \
    l(LOGGER_EVENT_SDCARD_INIT_DONE) \
    l(LOGGER_EVENT_SDCARD_DEINIT_DONE) \
    l(LOGGER_EVENT_FAT_PARTITION_MOUNTED) \
    l(LOGGER_EVENT_FAT_PARTITION_MOUNT_FAILED) \
    l(LOGGER_EVENT_FAT_PARTITION_UNMOUNTED) \
    l(LOGGER_EVENT_SCREEN_UPDATE_BEGIN) \
    l(LOGGER_EVENT_SCREEN_UPDATE_END) \
    l(LOGGER_EVENT_OTA_AUTO_UPDATE_START) \
    l(LOGGER_EVENT_OTA_AUTO_UPDATE_FINISH) \
    l(LOGGER_EVENT_OTA_AUTO_UPDATE_FAILED) \
    l(LOGGER_EVENT_OTA_AUTO_UPDATE_HAS_UPDATE)

// declaration of the specific events under the LOGGER_EVENT family
enum {                                       
    LOGGER_EVENT_LIST(ENUM)
};

extern const char * const logger_event_strings[];

#ifdef __cplusplus
}
#endif

#endif /* FF821ED1_B92A_42CD_9A5B_3D607C43E4E1 */
