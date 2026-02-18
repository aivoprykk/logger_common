/**
 * @file logger_vibration.c
 * @brief Vibration motor control implementation
 */

#include "logger_vibration.h"
#if defined(CONFIG_LOGGER_VIBRATION_ENABLED)
#include "common_private.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "vibration";

/* Module state */
static bool is_initialized = false;
static gpio_num_t vibration_pin = -1;
static esp_timer_handle_t timer_handle = NULL;

/* Pattern state for multi-pulse sequences */
typedef struct {
    uint8_t remaining_count;
    uint32_t duration_ms;
    uint32_t interval_ms;
    bool motor_on;  /* Track motor state for callback */
} vibration_pattern_t;

static vibration_pattern_t active_pattern = {0};

/* Forward declarations */
static void vibration_timer_callback(void *arg);
static void vibration_pattern_callback(void *arg);

/**
 * @brief Turn motor on
 */
static inline void motor_on(void)
{
    gpio_set_level(vibration_pin, 1);
}

/**
 * @brief Turn motor off
 */
static inline void motor_off(void)
{
    gpio_set_level(vibration_pin, 0);
}

/**
 * @brief Timer callback to turn off motor after pulse
 */
static void vibration_timer_callback(void *arg)
{
    motor_off();
}

/**
 * @brief Timer callback for pattern sequences
 * Handles both turning motor on/off during pattern playback
 */
static void vibration_pattern_callback(void *arg)
{
    if (active_pattern.remaining_count == 0) {
        /* Pattern complete */
        memset(&active_pattern, 0, sizeof(active_pattern));
        return;
    }

    if (active_pattern.motor_on) {
        /* Turn off motor (end of pulse) */
        motor_off();
        active_pattern.motor_on = false;

        /* Decrement pulse count */
        active_pattern.remaining_count--;

        /* If more pulses remain, schedule next pulse after interval */
        if (active_pattern.remaining_count > 0) {
            esp_timer_start_once(timer_handle,
                                 active_pattern.interval_ms * 1000);
        } else {
            /* Pattern complete */
            memset(&active_pattern, 0, sizeof(active_pattern));
        }
    } else {
        /* Turn on motor (start of pulse) */
        motor_on();
        active_pattern.motor_on = true;

        /* Schedule turn-off after duration */
        esp_timer_start_once(timer_handle,
                             active_pattern.duration_ms * 1000);
    }
}

esp_err_t logger_vibration_init(void)
{
    if (is_initialized) {
        WLOG(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }

#ifndef CONFIG_LOGGER_VIBRATION_GPIO
    ELOG(TAG, "GPIO not configured in Kconfig");
    return ESP_FAIL;
#endif

    vibration_pin = (gpio_num_t)CONFIG_LOGGER_VIBRATION_GPIO;

    /* Configure GPIO as output */
    esp_err_t ret = gpio_set_direction(vibration_pin, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK) {
        ELOG(TAG, "Failed to set GPIO %d direction: %d",
                 vibration_pin, ret);
        return ESP_FAIL;
    }

    /* Set initial state to OFF */
    ret = gpio_set_level(vibration_pin, 0);
    if (ret != ESP_OK) {
        ELOG(TAG, "Failed to set GPIO %d level: %d", vibration_pin, ret);
        return ESP_FAIL;
    }

    /* Create timer for pulse/pattern control */
    esp_timer_create_args_t timer_args = {
        .callback = vibration_pattern_callback,
        .arg = NULL,
        .name = "vib_timer",
        .dispatch_method = ESP_TIMER_TASK
    };

    ret = esp_timer_create(&timer_args, &timer_handle);
    if (ret != ESP_OK) {
        ELOG(TAG, "Failed to create timer: %d", ret);
        return ESP_FAIL;
    }

    is_initialized = true;
    ILOG(TAG, "Initialized on GPIO %d", vibration_pin);

    return ESP_OK;
}

esp_err_t logger_vibration_deinit(void)
{
    if (!is_initialized) {
        return ESP_OK;
    }

    /* Stop any active timers */
    if (timer_handle) {
        esp_timer_stop(timer_handle);
        esp_timer_delete(timer_handle);
        timer_handle = NULL;
    }

    /* Turn off motor */
    motor_off();

    /* Reset pattern state */
    memset(&active_pattern, 0, sizeof(active_pattern));

    /* Reset GPIO to input (safe state) */
    gpio_set_direction(vibration_pin, GPIO_MODE_INPUT);

    is_initialized = false;
    ILOG(TAG, "Deinitialized");

    return ESP_OK;
}

esp_err_t logger_vibration_pulse(uint32_t duration_ms)
{
    if (!is_initialized) {
        WLOG(TAG, "Not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    /* Use default duration if 0 specified */
    if (duration_ms == 0) {
#ifdef CONFIG_LOGGER_VIBRATION_DEFAULT_DURATION_MS
        duration_ms = CONFIG_LOGGER_VIBRATION_DEFAULT_DURATION_MS;
#else
        duration_ms = 50;  /* Fallback default */
#endif
    }

    /* Validate duration (max 1 second) */
    if (duration_ms > 1000) {
        WLOG(TAG, "Duration %lu ms too long, clamping to 1000ms",
                 duration_ms);
        duration_ms = 1000;
    }

    /* Stop any existing timer */
    if (timer_handle) {
        esp_timer_stop(timer_handle);
    }

    /* Setup single pulse pattern state */
    active_pattern.remaining_count = 1;
    active_pattern.duration_ms = duration_ms;
    active_pattern.interval_ms = 0;  /* Not used for single pulse */
    active_pattern.motor_on = false;  /* Will be set to true in callback */

    /* Start pulse */
    vibration_pattern_callback(NULL);

    return ESP_OK;
}

esp_err_t logger_vibration_pattern(uint8_t count, uint32_t duration_ms,
                                    uint32_t interval_ms)
{
    if (!is_initialized) {
        WLOG(TAG, "Not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    /* Validate parameters */
    if (count == 0 || count > 10) {
        ELOG(TAG, "Invalid count %d (must be 1-10)", count);
        return ESP_ERR_INVALID_ARG;
    }

    if (duration_ms == 0 || duration_ms > 500) {
        ELOG(TAG, "Invalid duration %lu ms (must be 1-500)", duration_ms);
        return ESP_ERR_INVALID_ARG;
    }

    if (interval_ms > 1000) {
        ELOG(TAG, "Invalid interval %lu ms (max 1000)", interval_ms);
        return ESP_ERR_INVALID_ARG;
    }

    /* Stop any existing pattern */
    if (timer_handle) {
        esp_timer_stop(timer_handle);
    }
    motor_off();

    /* Setup pattern state */
    active_pattern.remaining_count = count;
    active_pattern.duration_ms = duration_ms;
    active_pattern.interval_ms = interval_ms;
    active_pattern.motor_on = false;  /* Will be set to true in callback */

    /* Start first pulse */
    vibration_pattern_callback(NULL);

    return ESP_OK;
}
#endif /* CONFIG_LOGGER_VIBRATION_ENABLED */