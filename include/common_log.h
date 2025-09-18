#ifndef A19CB2A6_3C53_45BB_A057_D328AFD92736
#define A19CB2A6_3C53_45BB_A057_D328AFD92736

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(C_LOG_LEVEL)
#error "C_LOG_LEVEL must be defined before including this file"
#endif

#if (C_LOG_LEVEL <= 4) // 4 - error
#ifndef LOG_ERR
#define LOG_ERR(a, b, ...) ESP_LOGE(a, b, ##__VA_ARGS__)
#endif
#define ELOG LOG_ERR
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
#endif

#if (C_LOG_LEVEL <= 2) // 2 - info
#ifndef LOG_INFO
#define LOG_INFO(a, b, ...) ESP_LOGI(a, b, ##__VA_ARGS__)
#endif
#define ILOG LOG_INFO
#define IMEAS_START MEAS_START
#define IMEAS_END MEAS_END
#endif

#if (C_LOG_LEVEL <= 1) // 1 - debug
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#ifndef LOG_DEBUG
#define LOG_DEBUG(a, b, ...) printf(b "\n", ##__VA_ARGS__)
#endif
#define DLOG LOG_DEBUG
#define DMEAS_START MEAS_START
#define DMEAS_END MEAS_END
#endif

#if (C_LOG_LEVEL < 1) // 0 - trace
#ifndef LOG_TRACE
#define LOG_TRACE(a, b, ...) printf(b, ##__VA_ARGS__)
#endif
#define TLOG LOG_TRACE
#define TMEAS_START MEAS_START
#define TMEAS_END MEAS_END
#endif

#include "esp_log.h"

#ifndef LOG_TRACE
#define LOG_TRACE(a, b, ...)
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(a, b, ...)
#endif
#ifndef LOG_INFO
#define LOG_INFO(a, b, ...)
#endif
#ifndef LOG_WARN
#define LOG_WARN(a, b, ...)
#endif
#ifndef LOG_ERR
#define LOG_ERR(a, b, ...)
#endif
#ifndef MEAS_START
#define MEAS_START()
#endif
#ifndef MEAS_END
#define MEAS_END(a, b, ...)
#endif

#ifndef TLOG
#define TLOG(a, b, ...) ((void)0)
#endif
#ifndef DLOG
#define DLOG(a, b, ...) ((void)0)
#endif
#ifndef DMEAS_START
#define DMEAS_START() ((void)0)
#endif
#ifndef DMEAS_END
#define DMEAS_END(a, b, ...) ((void)0)
#endif
#ifndef ILOG
#define ILOG(a, b, ...) ((void)0)
#endif
#ifndef IMEAS_START
#define IMEAS_START() ((void)0)
#endif
#ifndef IMEAS_END
#define IMEAS_END(a, b, ...) ((void)0)
#endif
#ifndef WLOG
#define WLOG(a, b, ...) ((void)0)
#endif
#ifndef WMEAS_START
#define WMEAS_START() ((void)0)
#endif
#ifndef WMEAS_END
#define WMEAS_END(a, b, ...) ((void)0)
#endif
#ifndef ELOG
#define ELOG(a, b, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif