# Logger Common Component

A comprehensive utility library providing common functionality for ESP32-based GPS logger applications, including advanced logging, memory management, time utilities, configuration helpers, and shared data structures.

## Features

### Advanced Logging System
- **Compile-time Log Level Control**: Efficient log suppression at compile time
- **Multiple Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR with aliases
- **Performance Measurement**: Built-in timing macros for function profiling
- **Function Entry Logging**: Automatic function entry/exit logging
- **ESP-IDF Integration**: Compatible with ESP_LOG system

### Memory Management
- **Buffer Pool**: Shared buffer management to reduce heap fragmentation
- **Memory Monitoring**: Real-time heap and task memory statistics
- **Debug Tools**: Memory usage analysis and leak detection
- **Task Monitoring**: Stack usage and CPU utilization tracking

### Time & Date Utilities
- **Timezone Support**: Configurable timezone offsets with DST handling
- **Time Synchronization**: Integration with SNTP and GPS time sources
- **Time Conversion**: Multiple time format conversions (ms, us, ns)
- **Local Time**: POSIX timezone support for local time display

### Utility Macros & Functions
- **Bit Operations**: Set, clear, toggle, and test bit operations
- **Unit Conversions**: Time, distance, and measurement unit conversions
- **String Utilities**: Case conversion, hex formatting, MAC address formatting
- **Math Helpers**: Smoothing algorithms, rounding functions

### Configuration Management
- **Type-safe Setters**: Safe configuration value assignment with validation
- **Default Values**: Automatic fallback to default values
- **Bit Field Support**: Configuration bit manipulation helpers

### Event System Foundation
- **Base Event Declarations**: Foundation for component-specific events
- **Event String Conversion**: Debug-friendly event name resolution
- **ESP-IDF Integration**: Native ESP event loop compatibility

## Installation

### ESP-IDF Integration
Add this component to your ESP-IDF project:

```cmake
# In your project's CMakeLists.txt
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components logger_common)
```

### PlatformIO Integration
Add to your `platformio.ini`:

```ini
[env]
lib_deps =
    ; Add other dependencies
    file://./components/logger_common
```

## Configuration

### Build Configuration

#### Log Level Configuration
Set the compile-time log level in your component's CMakeLists.txt:

```cmake
# Set log level (0=TRACE, 1=DEBUG, 2=INFO, 3=WARN, 4=ERROR, 5=USER, 6=NONE)
set(C_LOG_LEVEL 2)  # INFO level
target_compile_definitions(${COMPONENT_TARGET} PRIVATE C_LOG_LEVEL=${C_LOG_LEVEL})
```

#### Build Mode Configuration
```cmake
# Development mode (enables DEBUG macros)
set(CONFIG_LOGGER_BUILD_MODE_DEV y)

# Production mode (disables DEBUG macros)
# set(CONFIG_LOGGER_BUILD_MODE_PROD y)
```

## Usage

### Logging System

#### Basic Logging
```c
#include "common_log.h"

// Define log level before including the header
#define C_LOG_LEVEL 2  // INFO level
#include "common_log.h"

static const char *TAG = "my_component";

// Log messages at different levels
ELOG(TAG, "Error: %s", error_message);        // Error level
WLOG(TAG, "Warning: %d", warning_value);      // Warning level
ILOG(TAG, "Info: %s", info_message);          // Info level
DLOG(TAG, "Debug: %d", debug_value);          // Debug level (only in DEBUG builds)
TLOG(TAG, "Trace: %s", trace_message);        // Trace level
```

#### Function Entry Logging
```c
#include "common_log.h"

void my_function(int param) {
    FUNC_ENTRY(TAG);  // Logs: "[my_function]"

    FUNC_ENTRY_ARGS(TAG, "param=%d", param);  // Logs: "[my_function] param=5"

    // Function implementation

    FUNC_ENTRYD(TAG);  // Debug level function entry
    FUNC_ENTRYW(TAG);  // Warning level function entry
}
```

#### Performance Measurement
```c
#include "common_log.h"

void process_data(void) {
    IMEAS_START();  // Start info-level timing

    // Code to measure
    do_something();

    IMEAS_END(TAG);  // Logs: "[process_data] took 1500 us"

    DMEAS_START();   // Start debug-level timing
    do_something_else();
    DMEAS_END(TAG);  // Logs: "[do_something_else] took 500 us"
}
```

### Memory Management

#### Buffer Pool Usage
```c
#include "logger_buffer_pool.h"

// Initialize buffer pool
esp_err_t ret = logger_buffer_pool_init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize buffer pool");
    return ret;
}

// Allocate a buffer
logger_buffer_handle_t handle;
ret = LOGGER_BUFFER_ALLOC(LOGGER_BUFFER_MEDIUM, LOGGER_BUFFER_USAGE_GPS_DATA, &handle);
if (ret == ESP_OK) {
    // Use the buffer
    snprintf((char*)handle.buffer, handle.size, "GPS data: %f", gps_value);

    // Free the buffer
    LOGGER_BUFFER_FREE(&handle);
}

// Get statistics
logger_buffer_pool_stats_t stats;
logger_buffer_pool_get_stats(&stats);
ESP_LOGI(TAG, "Buffers in use: %d, Peak usage: %d",
         stats.current_in_use, stats.peak_usage);
```

#### Memory Monitoring
```c
#include "logger_common.h"

// Log current heap status
mem_info();  // Logs heap usage statistics

// Log task memory information
task_memory_info("my_task");  // Logs specific task memory

// Log all tasks memory (debug builds only)
tasks_memory_info();  // Logs all tasks memory usage

// Log task CPU usage (debug builds only)
task_top();  // Logs task CPU utilization
```

### Time & Date Utilities

#### Timezone and Time Setting
```c
#include "logger_common.h"

// Set timezone (hours offset from UTC)
float timezone_offset = 2.0;  // Central European Time
set_time_zone(timezone_offset);

// Set system time from struct tm
struct tm time_info = {
    .tm_year = 2024 - 1900,
    .tm_mon = 0,    // January
    .tm_mday = 15,
    .tm_hour = 10,
    .tm_min = 30,
    .tm_sec = 0
};

c_set_time(&time_info, 0, timezone_offset);

// Set time from timestamp
int64_t timestamp_sec = 1705312200;  // 2024-01-15 10:30:00 UTC
c_set_time_ts(timestamp_sec, 0, timezone_offset);

// Get local time
struct tm local_time;
get_local_time(&local_time);
ESP_LOGI(TAG, "Local time: %04d-%02d-%02d %02d:%02d:%02d",
         local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday,
         local_time.tm_hour, local_time.tm_min, local_time.tm_sec);
```

#### Time Conversions
```c
#include "logger_common.h"

// Time unit conversions
uint32_t ms_value = 1500;
uint32_t seconds = MS_TO_SEC(ms_value);        // 1.5 seconds
uint32_t microseconds = MS_TO_US(ms_value);    // 1,500,000 us
uint32_t back_to_ms = SEC_TO_MS(seconds);      // 1,500 ms

// Distance conversions
float distance_mm = 1500.0f;
float distance_m = MM_TO_M(distance_mm);       // 1.5 meters
float distance_km = MM_TO_KM(distance_mm);     // 0.0015 km

// Speed conversions
float speed_mm_s = 1500.0f;  // 1500 mm/s
float speed_m_s = MM_S_TO_M_S(speed_mm_s);     // 1.5 m/s
```

### Utility Functions

#### Bit Operations
```c
#include "logger_common.h"

uint8_t flags = 0;

// Set, clear, toggle, and test bits
BIT_SET(flags, 0);        // Set bit 0
BIT_SET(flags, 2);        // Set bit 2
BIT_CLR(flags, 1);        // Clear bit 1
BIT_TGL(flags, 3);        // Toggle bit 3

if (BIT_GET(flags, 0)) {  // Test bit 0
    ESP_LOGI(TAG, "Bit 0 is set");
}

// Check result: flags = 0b00001101 (bits 0, 2, 3 set)
```

#### String and Data Conversions
```c
#include "logger_common.h"

// Convert uint8 to hex string
uint8_t value = 255;
char hex_str[3];
uint8_to_hex_string(value, hex_str);  // Result: "FF"

// Convert uint32 to byte array
uint32_t big_value = 0x12345678;
uint8_t bytes[4];
uint32_to_uint8_array(big_value, bytes);
// Result: bytes[0] = 0x78, bytes[1] = 0x56, bytes[2] = 0x34, bytes[3] = 0x12

// Convert MAC address to string
uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
char mac_str[13];
mac_to_char(mac, mac_str, 0);  // Result: "AABBCCDDEEFF"

// String case conversion
char text[] = "Hello World";
str_tolower(text);  // Result: "hello world"
```

#### Data Smoothing
```c
#include "logger_common.h"

#define WINDOW_SIZE 5
int32_t readings[100];
int32_t smoothed_readings[100];

// Fill readings array with sensor data
// ... sensor reading code ...

// Apply smoothing to reduce noise
for (int i = 0; i < 100; i++) {
    smoothed_readings[i] = smooth(readings, i, 100, WINDOW_SIZE);
}
```

### Configuration Management

#### Type-Safe Configuration Setting
```c
#include "common_cfg.h"

// Example configuration structure
typedef struct {
    float gain;
    uint8_t enabled;
    uint16_t threshold;
    char name[32];
} my_config_t;

my_config_t config;

// Set values with validation and defaults
uint8_t changed = 0;
changed |= set_f(&json_value, &config.gain, 1.0f);           // Float with default 1.0
changed |= set_hhu(&json_value, &config.enabled, 1);         // uint8_t with default 1
changed |= set_u(&json_value, &config.threshold, 1000);      // uint16_t with default 1000
changed |= set_c(&json_value, config.name, "default");       // String with default

// Set specific bit in a bitfield
uint8_t flags = 0;
changed |= set_bit(&json_value, &flags, 2, 1);  // Set bit 2 with default 1

if (changed) {
    ESP_LOGI(TAG, "Configuration updated");
    // Save configuration...
}
```

### Event System

#### Event Declaration and Handling
```c
#include "logger_events.h"

// Events are declared in logger_events.h
// Components can post events using:

esp_event_post(LOGGER_EVENT, LOGGER_EVENT_DATETIME_SET, NULL, 0, portMAX_DELAY);

// Handle events in other components
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == LOGGER_EVENT) {
        switch (event_id) {
            case LOGGER_EVENT_DATETIME_SET:
                ESP_LOGI(TAG, "Date/time has been set");
                break;
            case LOGGER_EVENT_SCREEN_UPDATE_BEGIN:
                ESP_LOGI(TAG, "Screen update starting");
                break;
            case LOGGER_EVENT_SCREEN_UPDATE_END:
                ESP_LOGI(TAG, "Screen update completed");
                break;
        }
    }
}

// Register event handler
esp_event_handler_register(LOGGER_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
```

## API Reference

### Logging Macros

#### Log Level Macros
- `ELOG(tag, fmt, ...)` - Error level logging
- `WLOG(tag, fmt, ...)` - Warning level logging
- `ILOG(tag, fmt, ...)` - Info level logging
- `DLOG(tag, fmt, ...)` - Debug level logging (DEBUG builds only)
- `TLOG(tag, fmt, ...)` - Trace level logging

#### Function Entry Macros
- `FUNC_ENTRY(tag)` - Log function entry (info level)
- `FUNC_ENTRYD(tag)` - Log function entry (debug level)
- `FUNC_ENTRYT(tag)` - Log function entry (trace level)
- `FUNC_ENTRY_ARGS(tag, fmt, ...)` - Log function entry with arguments

#### Performance Measurement
- `IMEAS_START()` - Start info-level timing
- `IMEAS_END(tag)` - End timing and log duration
- `DMEAS_START()` - Start debug-level timing
- `DMEAS_END(tag)` - End debug timing and log duration

### Memory Management Functions

#### Buffer Pool
- `logger_buffer_pool_init()` - Initialize buffer pool
- `logger_buffer_pool_alloc(size_type, usage_type, handle, timeout)` - Allocate buffer
- `logger_buffer_pool_free(handle)` - Free buffer
- `logger_buffer_pool_get_stats(stats)` - Get usage statistics

#### Memory Monitoring
- `mem_info()` - Log heap memory information
- `task_memory_info(task_name)` - Log task memory usage
- `tasks_memory_info()` - Log all tasks memory (debug builds)
- `task_top()` - Log task CPU usage (debug builds)

### Time & Date Functions

#### Time Setting
- `set_time_zone(offset)` - Set timezone offset
- `c_set_time(tm, us, timezone)` - Set time from struct tm
- `c_set_time_ts(sec, us, timezone)` - Set time from timestamp
- `c_set_time_ms(ms, us, timezone)` - Set time from milliseconds

#### Time Retrieval
- `get_local_time(info)` - Get current local time
- `c_timeval_to_tm_utc(tv, result)` - Convert timeval to UTC tm

### Utility Functions

#### Bit Operations
- `BIT_SET(var, nr)` - Set bit
- `BIT_CLR(var, nr)` - Clear bit
- `BIT_TGL(var, nr)` - Toggle bit
- `BIT_GET(var, nr)` - Get bit value

#### Conversions
- `uint8_to_hex_string(value, hex_str)` - Convert byte to hex string
- `uint32_to_uint8_array(value, array)` - Convert uint32 to byte array
- `mac_to_char(mac, mac_str, start)` - Convert MAC to string
- `str_tolower(s)` - Convert string to lowercase

#### Math Functions
- `smooth(array, index, size, window_size)` - Apply smoothing filter
- `get_distance_m(distance, output_rate)` - Calculate distance from sensor data

### Configuration Functions

#### Type-Safe Setters
- `set_f(value, val, def)` - Set float with default
- `set_hhu(value, val, def)` - Set uint8_t with default
- `set_u(value, val, def)` - Set uint16_t with default
- `set_bit(value, val, bit, def)` - Set bit with default
- `set_c(value, val, def)` - Set char array with default

### Data Structures

#### Buffer Pool Types
```c
typedef enum {
    LOGGER_BUFFER_SMALL = 0,     // 256 bytes
    LOGGER_BUFFER_MEDIUM = 1,    // 512 bytes
    LOGGER_BUFFER_LARGE = 2,     // 1024 bytes
} logger_buffer_size_t;

typedef enum {
    LOGGER_BUFFER_USAGE_HTTP_PATH,
    LOGGER_BUFFER_USAGE_GPS_DATA,
    LOGGER_BUFFER_USAGE_CONFIG,
    // ... more usage types
} logger_buffer_usage_t;

typedef struct {
    void *buffer;                // Allocated buffer pointer
    size_t size;                 // Buffer size
    logger_buffer_size_t size_type;
    logger_buffer_usage_t usage_type;
    uint32_t allocation_id;      // Unique allocation ID
} logger_buffer_handle_t;
```

#### Configuration Item
```c
typedef struct m_config_item_s {
    const char * name;           // Configuration name
    int pos;                     // Position/index
    uint32_t value;              // Current value
    const char *desc;            // Description
} m_config_item_t;
```

### Events

#### Logger Events
- `LOGGER_EVENT_DATETIME_SET` - Date/time has been set
- `LOGGER_EVENT_SCREEN_UPDATE_BEGIN` - Screen update starting
- `LOGGER_EVENT_SCREEN_UPDATE_END` - Screen update completed

## Examples

### Complete Component Setup

```c
#include "logger_common.h"
#include "common_log.h"
#include "logger_buffer_pool.h"

#define C_LOG_LEVEL 2  // INFO level
#include "common_log.h"

static const char *TAG = "my_component";

esp_err_t my_component_init(void) {
    FUNC_ENTRY(TAG);

    // Initialize buffer pool
    esp_err_t ret = logger_buffer_pool_init();
    if (ret != ESP_OK) {
        ELOG(TAG, "Failed to initialize buffer pool: %s", esp_err_to_name(ret));
        return ret;
    }

    // Set timezone
    set_time_zone(2.0);  // CET

    ILOG(TAG, "Component initialized successfully");
    return ESP_OK;
}

void my_component_process_data(int32_t *raw_data, size_t count) {
    FUNC_ENTRY_ARGS(TAG, "count=%d", count);

    IMEAS_START();

    // Allocate buffer for processing
    logger_buffer_handle_t handle;
    esp_err_t ret = LOGGER_BUFFER_ALLOC(LOGGER_BUFFER_MEDIUM,
                                       LOGGER_BUFFER_USAGE_TEMP, &handle);
    if (ret != ESP_OK) {
        ELOG(TAG, "Failed to allocate buffer");
        return;
    }

    // Process data with smoothing
    int32_t *smoothed = (int32_t *)handle.buffer;
    for (size_t i = 0; i < count; i++) {
        smoothed[i] = smooth(raw_data, i, count, 5);
    }

    // Log results
    ILOG(TAG, "Processed %d data points", count);

    // Free buffer
    LOGGER_BUFFER_FREE(&handle);

    IMEAS_END(TAG);
}

void my_component_monitor_memory(void) {
    // Log memory status periodically
    mem_info();

    // Get buffer pool statistics
    logger_buffer_pool_stats_t stats;
    if (logger_buffer_pool_get_stats(&stats) == ESP_OK) {
        DLOG(TAG, "Buffer pool: in_use=%d, peak=%d, failed=%d",
             stats.current_in_use, stats.peak_usage, stats.failed_allocations);
    }
}
```

### Advanced Logging Example

```c
#include "common_log.h"

#define C_LOG_LEVEL 1  // DEBUG level
#include "common_log.h"

void complex_algorithm(void) {
    FUNC_ENTRYD(TAG);

    DMEAS_START();

    // Phase 1
    TLOG(TAG, "Starting phase 1");
    phase1_processing();
    TMEAS_END_ARGS(TAG, "Phase 1 completed");

    // Phase 2
    TLOG(TAG, "Starting phase 2");
    phase2_processing();
    TMEAS_END_ARGS(TAG, "Phase 2 completed");

    // Phase 3
    TLOG(TAG, "Starting phase 3");
    phase3_processing();
    TMEAS_END_ARGS(TAG, "Phase 3 completed");

    DMEAS_END(TAG);  // Total time for all phases

    FUNC_ENTRYD(TAG);  // Function exit
}
```

## Troubleshooting

### Common Issues

#### Logging Not Working
- Ensure `C_LOG_LEVEL` is defined before including `common_log.h`
- Check that the log level is appropriate for your build (DEBUG vs RELEASE)
- Verify ESP-IDF logging is properly configured

#### Buffer Pool Allocation Failures
- Check buffer pool initialization
- Monitor buffer pool statistics for leaks
- Increase timeout values for allocations
- Verify buffer sizes meet your requirements

#### Time Setting Issues
- Check timezone offset values
- Verify SNTP server connectivity
- Ensure proper time source (GPS/SNTP) integration
- Check for daylight saving time handling

#### Memory Issues
- Use `mem_info()` to monitor heap usage
- Check for buffer pool leaks with statistics
- Monitor task stack usage
- Enable debug builds for detailed memory analysis

### Debug Configuration

Enable detailed debugging:

```cmake
# Enable debug logging
set(C_LOG_LEVEL 1)

# Enable debug build mode
set(CONFIG_LOGGER_BUILD_MODE_DEV y)

# Add debug definitions
target_compile_definitions(${COMPONENT_TARGET} PRIVATE
    C_LOG_LEVEL=${C_LOG_LEVEL}
)
```

### Performance Monitoring

```c
// Monitor buffer pool usage
logger_buffer_pool_stats_t stats;
logger_buffer_pool_get_stats(&stats);
ESP_LOGI(TAG, "Buffer usage: %d/%d (peak: %d)",
         stats.current_in_use, LOGGER_BUFFER_POOL_COUNT, stats.peak_usage);

// Monitor memory periodically
static uint32_t last_mem_check = 0;
uint32_t now = get_millis();
if (now - last_mem_check > 30000) {  // Every 30 seconds
    mem_info();
    last_mem_check = now;
}
```

## Dependencies

- ESP-IDF v4.4 or later
- FreeRTOS
- ESP event system
- ESP timer (for performance measurement)

## Contributing

1. Follow ESP-IDF coding conventions
2. Add comprehensive logging to new functions
3. Include performance measurements for complex operations
4. Update documentation for new utilities
5. Test with multiple log levels and build configurations

## License

See LICENSE file in component directory.