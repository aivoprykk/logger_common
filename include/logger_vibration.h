/**
 * @file logger_vibration.h
 * @brief Vibration motor control for haptic feedback
 *
 * Provides non-blocking vibration motor control via GPIO output.
 * Uses esp_timer for timed pulses without blocking the main loop.
 *
 * Usage:
 *   logger_vibration_init();
 *   logger_vibration_pulse(50);  // 50ms pulse
 *   logger_vibration_pattern(3, 30, 100);  // 3 pulses: 30ms on, 100ms off
 */

#ifndef LOGGER_VIBRATION_H
#define LOGGER_VIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"
#if defined(CONFIG_LOGGER_VIBRATION_ENABLED)
#include "esp_err.h"
#include <stdint.h>
/**
 * @brief Initialize vibration motor GPIO
 *
 * Configures the GPIO pin specified in Kconfig as output and sets
 * initial state to LOW (motor off). Creates timer for non-blocking operation.
 *
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_STATE if already initialized
 *         ESP_FAIL on GPIO configuration error
 */
esp_err_t logger_vibration_init(void);

/**
 * @brief Deinitialize vibration motor
 *
 * Stops any active vibration, cancels timers, and resets GPIO.
 * Safe to call even if not initialized.
 *
 * @return ESP_OK on success
 */
esp_err_t logger_vibration_deinit(void);

/**
 * @brief Trigger a single vibration pulse
 *
 * Non-blocking function that activates the motor for specified duration.
 * Uses esp_timer to automatically turn off motor after delay.
 *
 * @param duration_ms Pulse duration in milliseconds
 *                    Use 0 to apply default from Kconfig
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_STATE if not initialized
 *         ESP_ERR_INVALID_ARG if duration > 1000ms
 */
esp_err_t logger_vibration_pulse(uint32_t duration_ms);

/**
 * @brief Trigger a vibration pattern (multiple pulses)
 *
 * Non-blocking function that activates motor in a repeated pattern.
 * Pattern: ON(duration) -> OFF(interval) -> repeat count times
 *
 * @param count Number of pulses (1-10)
 * @param duration_ms Duration per pulse in milliseconds
 * @param interval_ms Interval between pulses in milliseconds
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_STATE if not initialized
 *         ESP_ERR_INVALID_ARG if parameters out of range
 */
esp_err_t logger_vibration_pattern(uint8_t count, uint32_t duration_ms,
                                    uint32_t interval_ms);
#endif
#ifdef __cplusplus
}
#endif

#endif // LOGGER_VIBRATION_H
