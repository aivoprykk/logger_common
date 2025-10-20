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
#define LOG_ERR_NUM 4
#define LOG_WARN_NUM 3
#define LOG_INFO_NUM 2
#define LOG_DEBUG_NUM 1
#define LOG_TRACE_NUM 0

#ifdef USE_ESP_LOG
#define LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
#define LOGV(tag, fmt, ...) ESP_LOGV(tag, fmt, ##__VA_ARGS__)
#else
#define LOGE(tag, fmt, ...) printf("E: %s: " fmt "\n", tag, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) printf("W: %s: " fmt "\n", tag, ##__VA_ARGS__)
#define LOGI(tag, fmt, ...) printf("I: %s: " fmt "\n", tag, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) printf("D: %s: " fmt "\n", tag, ##__VA_ARGS__)
#define LOGV(tag, fmt, ...) printf("T: %s: " fmt "\n", tag, ##__VA_ARGS__)
#endif

#if (C_LOG_LEVEL <= LOG_ERR_NUM) // 4 - error
#ifndef LOG_ERR
#define LOG_ERR(a, fmt, ...) LOGE(a, fmt, ##__VA_ARGS__)
#endif
#define ELOG LOG_ERR
#else
#define LOG_ERR(a, b, ...) ((void)0)
#define ELOG(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= LOG_WARN_NUM) // 3 - warn
#ifndef LOG_WARN
#define LOG_WARN(tag, fmt, ...) LOGW(tag, fmt, ##__VA_ARGS__)
#endif
#define WLOG LOG_WARN
#else
#define LOG_WARN(a, b, ...) ((void)0)
#define WLOG(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
#ifndef LOG_INFO
#define LOG_INFO(a, fmt, ...) LOGI(a, fmt, ##__VA_ARGS__)
#endif
#define ILOG LOG_INFO

#include "esp_timer.h"
#ifndef IMEAS_START
#define IMEAS_START() uint64_t _start = (esp_timer_get_time())
#endif
#ifndef IMEAS_END
#define IMEAS_END(tag) \
    ILOG(tag, "[%s] took %llu us", __func__, (esp_timer_get_time() - _start))
#endif
#ifndef IMEAS_END_ARGS
#define IMEAS_END_ARGS(tag, fmt, ...) \
    ILOG(tag, "[%s] took %llu us " fmt, __func__, (esp_timer_get_time() - _start), ##__VA_ARGS__)
#endif

#else
#define LOG_INFO(a, b, ...) ((void)0)
#define ILOG(a, b, ...) ((void)0)
#define IMEAS_START() ((void)0)
#define IMEAS_END(a) ((void)0)
#define IMEAS_END_ARGS(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL <= LOG_DEBUG_NUM) // 1 - debug
#ifndef LOG_DEBUG
#define LOG_DEBUG(tag, fmt, ...) LOGD(tag, fmt, ##__VA_ARGS__)
#endif
#define DLOG LOG_DEBUG

#define DMEAS_START IMEAS_START
#ifndef DMEAS_END
#define DMEAS_END(tag) \
    DLOG(tag, "[%s] took %llu us", __func__, (esp_timer_get_time() - _start))
#endif
#ifndef DMEAS_END_ARGS
#define DMEAS_END_ARGS(tag, fmt, ...) \
    DLOG(tag, "[%s] took %llu us " fmt, __func__, (esp_timer_get_time() - _start), ##__VA_ARGS__)
#endif

#else
#define LOG_DEBUG(a, b, ...) ((void)0)
#define DLOG(a, b, ...) ((void)0)
#define DMEAS_START() ((void)0)
#define DMEAS_END(a) ((void)0)
#define DMEAS_END_ARGS(a, b, ...) ((void)0)
#endif

#if (C_LOG_LEVEL == LOG_TRACE_NUM) // 0 - trace
#ifndef LOG_TRACE
#define LOG_TRACE(tag, fmt, ...) LOGV(tag, fmt, ##__VA_ARGS__)
#endif
#define TLOG LOG_TRACE
#define TMEAS_START IMEAS_START
#define TMEAS_END DMEAS_END
#define TMEAS_END_ARGS DMEAS_END_ARGS
#else
#define LOG_TRACE(a, b, ...) ((void)0)
#define TLOG(a, b, ...) ((void)0)
#define TMEAS_START() ((void)0)
#define TMEAS_END(a) ((void)0)
#define TMEAS_END_ARGS(a, b, ...) ((void)0)
#endif

#ifdef USE_ESP_LOG
#if (C_LOG_LEVEL == LOG_TRACE_NUM) // 0 - trace
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#elif (C_LOG_LEVEL == LOG_DEBUG_NUM)
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#elif (C_LOG_LEVEL == LOG_INFO_NUM)
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#elif (C_LOG_LEVEL == LOG_WARN_NUM)
#define LOG_LOCAL_LEVEL ESP_LOG_WARN
#elif (C_LOG_LEVEL == LOG_ERR_NUM)
#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#else
#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#endif

#include "esp_log.h"
#endif

// Function entry logging macros
#if (C_LOG_LEVEL <= LOG_TRACE_NUM) // 1 - debug level for function entries
#define FUNC_ENTRYT(tag) TLOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGT(tag, fmt, ...) TLOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRYT(tag) ((void)0)
#define FUNC_ENTRY_ARGT(tag, fmt, ...) ((void)0)
#endif
#if (C_LOG_LEVEL <= LOG_DEBUG_NUM) // 1 - debug level for function entries
#define FUNC_ENTRYD(tag) DLOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGSD(tag, fmt, ...) DLOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRYD(tag) ((void)0)
#define FUNC_ENTRY_ARGSD(tag, fmt, ...) ((void)0)
#endif
#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info level for function entries
#define FUNC_ENTRY(tag) ILOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGS(tag, fmt, ...) ILOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRY(tag) ((void)0)
#define FUNC_ENTRY_ARGS(tag, fmt, ...) ((void)0)
#endif
#if (C_LOG_LEVEL <= LOG_WARN_NUM) // 3 - warn level for function entries
#define FUNC_ENTRYW(tag) WLOG(tag, "[%s]", __func__)
#define FUNC_ENTRY_ARGW(tag, fmt, ...) WLOG(tag, "[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define FUNC_ENTRYW(tag) ((void)0)
#define FUNC_ENTRY_ARGW(tag, fmt, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif