#ifndef FF821ED1_B92A_42CD_9A5B_3D607C43E4E1
#define FF821ED1_B92A_42CD_9A5B_3D607C43E4E1

#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare an event base
ESP_EVENT_DECLARE_BASE(LOGGER_EVENT);        // declaration of the LOGGER_EVENT family

// declaration of the specific events under the LOGGER_EVENT family
enum {                                       
    LOGGER_EVENT_DATETIME_SET,                  // Date updated through SNTP 
    LOGGER_EVENT_SDCARD_MOUNTED,
    LOGGER_EVENT_SDCARD_MOUNT_FAILED, 
    LOGGER_EVENT_SDCARD_UNMOUNTED, 
    LOGGER_EVENT_FAT_PARTITION_MOUNTED,
    LOGGER_EVENT_FAT_PARTITION_MOUNT_FAILED, 
    LOGGER_EVENT_FAT_PARTITION_UNMOUNTED,
    LOGGER_EVENT_SCREEN_UPDATE_BEGIN,
    LOGGER_EVENT_SCREEN_UPDATE_END,
};

#ifdef __cplusplus
}
#endif

#endif /* FF821ED1_B92A_42CD_9A5B_3D607C43E4E1 */
