
#include "logger_events.h"

#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || CONFIG_LOGGER_GLOBAL_LOG_LEVEL < 2)
static const char *const _logger_event_strings[] = {
	LOGGER_EVENT_LIST(STRINGIFY)};
const char *logger_event_strings(int id) {
	return id < lengthof(_logger_event_strings) ? _logger_event_strings[id]
												: "LOGGER_EVENT_UNKNOWN";
}
#else
const char *logger_event_strings(int id) { return "LOGGER_EVENT"; }
#endif

static bool is_leap_year(uint32_t year) {
	return ((year % 4U) == 0U && (year % 100U) != 0U) ||
		   ((year % 400U) == 0U);
}

static uint8_t days_in_month(uint32_t year, uint8_t month) {
	static const uint8_t month_lengths[] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	};

	if (month == 2U && is_leap_year(year)) {
		return 29U;
	}
	return month_lengths[month - 1U];
}

static void increment_utc_minute(uint32_t *year, uint8_t *month, uint8_t *day,
						 uint8_t *hour, uint8_t *minute) {
	if (++(*minute) < 60U) {
		return;
	}

	*minute = 0U;
	if (++(*hour) < 24U) {
		return;
	}

	*hour = 0U;
	if (++(*day) <= days_in_month(*year, *month)) {
		return;
	}

	*day = 1U;
	if (++(*month) <= 12U) {
		return;
	}

	*month = 1U;
	++(*year);
}

static void decrement_utc_minute(uint32_t *year, uint8_t *month, uint8_t *day,
						 uint8_t *hour, uint8_t *minute) {
	if (*minute > 0U) {
		--(*minute);
		return;
	}

	*minute = 59U;
	if (*hour > 0U) {
		--(*hour);
		return;
	}

	*hour = 23U;
	if (*day > 1U) {
		--(*day);
		return;
	}

	if (*month > 1U) {
		--(*month);
	} else {
		*month = 12U;
		--(*year);
	}
	*day = days_in_month(*year, *month);
}

static int32_t round_signed_division(int32_t value, int32_t divisor) {
	if (value >= 0) {
		return (value + (divisor / 2)) / divisor;
	}
	return -(((-value) + (divisor / 2)) / divisor);
}

static int64_t days_from_civil(int32_t year, uint32_t month, uint32_t day) {
	year -= month <= 2U;
	const int32_t era = (year >= 0 ? year : year - 399) / 400;
	const uint32_t year_of_era = (uint32_t)(year - era * 400);
	const uint32_t month_prime = month + (month > 2U ? (uint32_t)-3 : 9U);
	const uint32_t day_of_year =
		(153U * month_prime + 2U) / 5U + day - 1U;
	const uint32_t day_of_era =
		year_of_era * 365U + year_of_era / 4U - year_of_era / 100U + day_of_year;

	return (int64_t)era * 146097LL + (int64_t)day_of_era - 719468LL;
}

int32_t c_nano_to_millis_round(int32_t nano) {
	return round_signed_division(nano, 1000000);
}

int32_t c_nano_to_us_round(int32_t nano) {
	return round_signed_division(nano, 1000);
}

void c_normalize_utc_fields(uint32_t *year, uint8_t *month, uint8_t *day,
						    uint8_t *hour, uint8_t *minute,
						    uint8_t *second, int32_t *subsecond,
						    uint32_t units_per_second) {
	int64_t total_units = 0;
	const int64_t units_per_minute = (int64_t)units_per_second * 60LL;

	if (!year || !month || !day || !hour || !minute || !second || !subsecond ||
		units_per_second == 0U || *month == 0U || *month > 12U) {
		return;
	}

	total_units = ((int64_t)(*second) * (int64_t)units_per_second) + *subsecond;

	while (total_units < 0) {
		total_units += units_per_minute;
		decrement_utc_minute(year, month, day, hour, minute);
	}

	while (total_units >= units_per_minute) {
		total_units -= units_per_minute;
		increment_utc_minute(year, month, day, hour, minute);
	}

	*second = (uint8_t)(total_units / (int64_t)units_per_second);
	*subsecond = (int32_t)(total_units % (int64_t)units_per_second);
}

uint64_t c_utc_ms_from_date_time(uint32_t year, uint8_t month, uint8_t day,
						 uint8_t hour, uint8_t minute, uint8_t second,
						 int32_t millis, int32_t *normalized_millis) {
	c_normalize_utc_fields(&year, &month, &day, &hour, &minute, &second,
					   &millis, 1000U);
	if (normalized_millis) {
		*normalized_millis = millis;
	}

	if (year < 1970U || month == 0U || day == 0U) {
		return 0;
	}

	const int64_t days = days_from_civil((int32_t)year, month, day);
	const int64_t seconds = days * 86400LL + (int64_t)hour * 3600LL +
						 (int64_t)minute * 60LL + (int64_t)second;

	if (seconds < 0) {
		return 0;
	}

	return (uint64_t)seconds * 1000ULL + (uint64_t)millis;
}

int c_set_time_ts(int64_t sec, uint32_t us, int8_t timezone) {
	if (sec > 1672531200) { /// 2023-01-01
		struct timeval tv = {.tv_sec = (sec + (int64_t)HOUR_TO_SEC(timezone)),
							 .tv_usec = us};
		return settimeofday(&tv, NULL);
	}
	return -2;
}

int c_set_time(struct tm *tm, uint32_t us, int8_t timezone) {
	if (tm && tm->tm_year > 123) { /// > 2023
		return c_set_time_ts(mktime(tm), us, timezone);
	}
	return -2;
}

int c_set_time_ms(int64_t ms, uint32_t us, int8_t timezone) {
	return c_set_time_ts((time_t)FROM_K_UL(ms), us, timezone);
}

struct tm *c_timeval_to_tm_utc(const struct timeval *tv, struct tm *result) {
	time_t sec = tv->tv_sec;
	return gmtime_r(&sec, result);
}

struct tm *get_local_time(struct tm *timeinfo) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	c_timeval_to_tm_utc(&tv, timeinfo);
	return timeinfo;
}

#if (C_LOG_LEVEL <= LOG_INFO_NUM)
esp_err_t task_memory_info(const char *task_name) {
	printf("*** Task %s (%s) stack High Water Mark: %u bytes ***\n",
		   pcTaskGetName(NULL), task_name, uxTaskGetStackHighWaterMark(NULL));
	return ESP_OK;
}
#endif

#if (C_LOG_LEVEL <= LOG_WARN_NUM)
esp_err_t mem_info(void) {
	printf("*** Heap: "
		   "free: %" PRIu32 " b, minfree: %" PRIu32" b"
		   ", def: %zu / %zu b"
		   ", int: %zu / %zu b"
		   ", 8bit: %zu / %zu b"
		   ", dma: %zu / %zu b"
#if CONFIG_IDF_TARGET_ESP32S3
		   ", spiram: %zu / %zu b"
#endif
		   " ***\n",
		   esp_get_free_heap_size(), esp_get_minimum_free_heap_size(),
		   /// may be internal is default, but not necessarily, so report both
		   heap_caps_get_total_size(MALLOC_CAP_DEFAULT), heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
		   heap_caps_get_total_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
		   /// may be 8bit is dma but not necessarily, so report both
		   heap_caps_get_total_size(MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_8BIT),
		   heap_caps_get_total_size(MALLOC_CAP_DMA), heap_caps_get_free_size(MALLOC_CAP_DMA)
#if CONFIG_IDF_TARGET_ESP32S3
		   /// report spiram for ESP32S3, which may be part of default/8bit/dma but not necessarily
		   , heap_caps_get_total_size(MALLOC_CAP_SPIRAM), heap_caps_get_free_size(MALLOC_CAP_SPIRAM)
#endif
	);
	return ESP_OK;
}
#endif

#if (C_LOG_LEVEL <= LOG_DEBUG_NUM)
esp_err_t tasks_memory_info() {
	char str[40 * uxTaskGetNumberOfTasks() + 1];
	vTaskList(&(str[0]));
	// fields:
	// Task Name: Name of the task
	// Task State: Current state: 'R' (Ready), 'B' (Blocked), 'S' (Suspended),
	// 'D' (Deleted)
	// Task Priority: Numeric priority level (0 = lowest) Stack
	// High Water Mark: Minimum amount of stack space that has remained for the
	// task since the task was created Task Number: Unique identifier for the
	// task
	printf("******\n%s", str);
	return ESP_OK;
}
esp_err_t task_top() {
	char str[40 * uxTaskGetNumberOfTasks() + 1];
	vTaskGetRunTimeStats(&(str[0]));
	// fields:
	// Name of the task,
	// Time (in ticks),
	// Percentage of total run time
	printf("******\n%s", str);
	return ESP_OK;
}
#endif

// unsigned long IRAM_ATTR get_micros() { return (unsigned long)
// (esp_timer_get_time()); }
unsigned long IRAM_ATTR get_millis() {
	return (unsigned long)(esp_timer_get_time() / 1000UL);
}

int32_t smooth(const int32_t *array, const int32_t index, const uint32_t size,
			   const uint8_t window_size) {
	if (index < 0)
		return 0;
	if (window_size == 0)
		return array[index % size];

	int32_t sum = 0, index_win = index - window_size,
			j; // move index_win to the left for window_size
	int8_t count = 0, i = -window_size;
	for (; i <= window_size; ++i) {
		j = index_win + i;
		sum += array[j % size];
		// printf("{i:%" PRId8 ", sum:%" PRId32 ", count:%" PRId8 ", j:%" PRId32
		// ", index_win:%" PRId32 ", size:%" PRIu32 ", array_j:%" PRId32 "}\n",
		// i, sum, count, j, index_win, size, array[j % size]);
		count++;
	}
	// printf("aaaaaaaaaaaaaaaaa sum:%" PRId32 " count:%" PRId8 " ret: %" PRId32
	// " aaaaaaaaaaaaaaaaa\n", sum, count, count ? sum / count : 0);
	return count ? sum / count : 0;
}

int smooth_int(const int *array, const int32_t index, const uint32_t size,
			   const uint8_t window_size) {
	if (index < 0)
		return 0;
	if (window_size == 0)
		return array[index % size];

	int sum = 0, index_win = index - window_size,
		j; // move index_win to the left for window_size
	int8_t count = 0, i = -window_size;
	for (; i <= window_size; ++i) {
		j = index_win + i;
		sum += array[j % size];
		count++;
	}
	return count ? sum / count : 0;
}

uint8_t len_helper_u(uint64_t x) {
	if (x >= 100000u) {
		if (x >= 10000000u) {
			if (x >= 1000000000u)
				return 10;
			if (x >= 100000000u)
				return 9;
			return 8;
		}
		if (x >= 1000000u)
			return 7;
		return 6;
	} else {
		if (x >= 1000u) {
			if (x >= 10000u)
				return 5;
			return 4;
		} else {
			if (x >= 100u)
				return 3;
			if (x >= 10u)
				return 2;
			return 1;
		}
	}
}
