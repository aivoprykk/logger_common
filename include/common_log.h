#ifndef A19CB2A6_3C53_45BB_A057_D328AFD92736
#define A19CB2A6_3C53_45BB_A057_D328AFD92736

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(C_LOG_LEVEL)
#error "C_LOG_LEVEL must be defined before including this file"
#endif

// String suppression for binary size optimization
// When log level is higher than the message level, completely suppress strings from binary

#if (C_LOG_LEVEL <= 4) // 4 - error
#ifndef LOG_ERR
#define LOG_ERR(a, b, ...) ESP_LOGE(a, b, ##__VA_ARGS__)
#endif
#define ELOG LOG_ERR
#else
#define LOG_ERR(a, b, ...) ((void)0)
#define ELOG(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= 3) // 3 - warn
#include "esp_timer.h"
#ifndef MEAS_START
#define MEAS_START() uint64_t _start = (esp_timer_get_time())
#endif
#ifndef MEAS_END
#define MEAS_END(a, b, ...) \
    ESP_LOGI(a, b, ##__VA_ARGS__, (esp_timer_get_time() - _start))
#endif
#ifndef LOG_WARN
#define LOG_WARN(a, b, ...) ESP_LOGW(a, b, ##__VA_ARGS__)
#endif
#define WLOG LOG_WARN
#define WMEAS_START MEAS_START
#define WMEAS_END MEAS_END
#else
#define LOG_WARN(a, b, ...) ((void)0)
#define WLOG(a, b, ...) ((void)0)
#define MEAS_START() ((void)0)
#define MEAS_END(a, b, ...) ((void)0)
#define WMEAS_START() ((void)0)
#define WMEAS_END(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= 2) // 2 - info
#ifndef LOG_INFO
#define LOG_INFO(a, b, ...) ESP_LOGI(a, b, ##__VA_ARGS__)
#endif
#define ILOG LOG_INFO
#define IMEAS_START MEAS_START
#define IMEAS_END MEAS_END
#else
#define LOG_INFO(a, b, ...) ((void)0)
#define ILOG(a, b, ...) ((void)0)
#define IMEAS_START() ((void)0)
#define IMEAS_END(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= 1) // 1 - debug
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#ifndef LOG_DEBUG
#define LOG_DEBUG(a, b, ...) printf("%s: " b "\n", a, ##__VA_ARGS__)
#endif
#define DLOG LOG_DEBUG
#define DMEAS_START MEAS_START
#define DMEAS_END MEAS_END
#else
#define LOG_DEBUG(a, b, ...) ((void)0)
#define DLOG(a, b, ...) ((void)0)
#define DMEAS_START() ((void)0)
#define DMEAS_END(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL < 1) // 0 - trace
#ifndef LOG_TRACE
#define LOG_TRACE(a, b, ...) printf(b, ##__VA_ARGS__)
#endif
#define TLOG LOG_TRACE
#define TMEAS_START MEAS_START
#define TMEAS_END MEAS_END
#else
#define LOG_TRACE(a, b, ...) ((void)0)
#define TLOG(a, b, ...) ((void)0)
#define TMEAS_START() ((void)0)
#define TMEAS_END(a, b, ...) ((void)0)
#endif

#include "esp_log.h"

// Function entry logging macros
#if (C_LOG_LEVEL <= 1) // 1 - debug level for function entries
#define FUNC_ENTRYD(tag) DLOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGSD(tag, fmt, ...) DLOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRYD(tag) ((void)0)
#define FUNC_ENTRY_ARGSD(tag, fmt, ...) ((void)0)
#endif
#if (C_LOG_LEVEL <= 2) // 2 - info level for function entries
#define FUNC_ENTRY(tag) ILOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGS(tag, fmt, ...) ILOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRY(tag) ((void)0)
#define FUNC_ENTRY_ARGS(tag, fmt, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif