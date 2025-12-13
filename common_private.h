#ifndef AC0B1DE6_7672_47CB_86F4_4D5EA5626359
#define AC0B1DE6_7672_47CB_86F4_4D5EA5626359

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#if (defined(CONFIG_LOGGER_USE_GLOBAL_LOG_LEVEL) && CONFIG_LOGGER_GLOBAL_LOG_LEVEL < CONFIG_LOGGER_COMMON_LOG_LEVEL)
#define C_LOG_LEVEL CONFIG_LOGGER_GLOBAL_LOG_LEVEL
#else
#define C_LOG_LEVEL CONFIG_LOGGER_COMMON_LOG_LEVEL
#endif
#include "common_log.h"
#include "unified_config.h"

#ifdef __cplusplus
}
#endif
#endif /* AC0B1DE6_7672_47CB_86F4_4D5EA5626359 */
