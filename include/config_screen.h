#ifndef A4A1AE36_A443_46AA_BCC0_3D334F6DD429
#define A4A1AE36_A443_46AA_BCC0_3D334F6DD429

#ifdef __cplusplus
extern "C" {
#endif
#include "logger_common.h"

// Configuration items for screen settings

#if defined(CONFIG_LOGGER_BUTTON_GPIO_1) || defined(CONFIG_UBUTTON_GPIO_1)
#define CFG_2ND_BUTTON_SCREEN_ITEMS(l) l(gpio12_screens)
#else
#define CFG_2ND_BUTTON_SCREEN_ITEMS(l)
#endif

#if !defined(CONFIG_LCD_IS_EPD)
#define CFG_SCREEN_BRIGHTNESS_ITEMS(l) l(screen_brightness)
#else
#define CFG_SCREEN_BRIGHTNESS_ITEMS(l) l(full_refresh_interval)
#endif

#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
#define CFG_SPEED_SCREEN_LARGE_FONT_ITEMS(l) l(speed_large_font)
#else
#define CFG_SPEED_SCREEN_LARGE_FONT_ITEMS(l)
#endif

#define CFG_SCREEN_ITEM_LIST(l) \
l(speed_field) \
l(board_logo) \
l(sail_logo) \
l(screen_rotation) \
l(bat_view) \
CFG_2ND_BUTTON_SCREEN_ITEMS(l) \
CFG_SPEED_SCREEN_LARGE_FONT_ITEMS(l) \
CFG_SCREEN_BRIGHTNESS_ITEMS(l) \
l(bar_length)  \
l(sleep_info)

#define CFG_SCREEN_ALL_ITEM_LIST(l) \
    CFG_SCREEN_ITEM_LIST(l)

extern const char * const config_screen_items[];
extern const size_t config_screen_item_count;

typedef struct cfg_screen_s {
    uint16_t version;
    uint8_t speed_field;        // choice for first field in speed screen
    uint8_t board_logo;        // choice for board logo
    uint8_t sail_logo;         // choice for sail logo
    int8_t screen_rotation;    // screen rotation
    uint8_t bat_view;          // battery view mode
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1) || defined(CONFIG_UBUTTON_GPIO_1)
    uint16_t gpio12_screens;     // GPIO12 screens
#define CFG_2ND_BUTTON_SCREENS .gpio12_screens = 255U,
#else
#define CFG_2ND_BUTTON_SCREENS
#endif
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
    uint8_t speed_large_font;    // Large speed font
#define CFG_SCREEN_SPEED_LARGE_FONT .speed_large_font = 0,
#else
#define CFG_SCREEN_SPEED_LARGE_FONT
#endif
#if !defined(CONFIG_LCD_IS_EPD)
    uint8_t screen_brightness; // Brightness %
#define SCREEN_BRIGHTNESS_DEFAULT .screen_brightness = 100,
#else
#define SCREEN_BRIGHTNESS_DEFAULT .full_refresh_interval = 200,
    uint8_t full_refresh_interval; // EPD full refresh interval in cycles
#endif
    uint16_t bar_length;         // Bar length in meters
    char sleep_info[32];         // Sleep info string
} __attribute__((packed, aligned(4))) cfg_screen_t;

#define CFG_SCREEN_DEFAULTS() { \
    .version = 1, \
    .speed_field = 1, \
    .board_logo = 1, \
    .sail_logo = 1, \
    .screen_rotation = SCR_DEFAULT_ROTATION, \
    .bat_view = 2, \
    CFG_2ND_BUTTON_SCREENS \
    CFG_SCREEN_SPEED_LARGE_FONT \
    SCREEN_BRIGHTNESS_DEFAULT \
    .bar_length = 1852, \
    .sleep_info = "ESP GPS", \
}

// Individual configuration item helpers

#define SPEED_SCREEN_UPPER_FIELD_ITEM_LIST(l) \
l(dynamic) \
l(spd_10_sec) \
l(spd_alpha) \
l(spd_1852_m) \
l(spd_500_m) \
l(spd_dist_time) \
l(spd_max_2s_10s) \
l(spd_half_hour) \
l(spd_1_hour) \
l(spd_1h_dynamic)
extern const char * const config_speed_field_items[];
extern const size_t config_speed_field_item_count;

#define BOARD_LOGO_ITEM_LIST(l) l(Starboard) l(Fanatic) l(JP) l(Patrik)
extern const char * const board_logos[];
extern const size_t board_logos_count;

#define SAIL_LOGO_ITEM_LIST(l) l(GASails) l(Duotone) l(NeilPryde) l(LoftSails) l(Gunsails) l(Point7) l(Patrik)
extern const char * const sail_logos[];
extern const size_t sail_logos_count;

#define SCREEN_ROTATION_ITEM_LIST(l) l(0_deg) l(90_deg) l(180_deg) l(270_deg)
extern const char * const screen_rotations[];
extern const size_t screen_rotations_count;

// Default screen rotation is 90 degrees.
#if !defined(SCR_DEFAULT_ROTATION)
#if !defined(CONFIG_LCD_IS_EPD)
#define SCR_DEFAULT_ROTATION 2
#else
#define SCR_DEFAULT_ROTATION 1
#endif
#endif

#define BAT_VIEW_LIST(l) \
l(bat_icon) \
l(bat_perc) \
l(bat_volt)

extern const char* const bat_views[];
extern const size_t bat_views_count;

#define BAT_VIEW_ENUM(l) scfg_bat_view_##l,
#define SPEED_SCREEN_UPPER_FIELD_ENUM(l) cfg_fld_##l,

typedef enum {
    SPEED_SCREEN_UPPER_FIELD_ITEM_LIST(SPEED_SCREEN_UPPER_FIELD_ENUM)
} speed_screen_upper_field_e_t;

typedef enum  {
    BAT_VIEW_LIST(BAT_VIEW_ENUM)
} bat_view_e_t;

#define CFG_ENUM_SCREEN(l) cfg_screen_##l,
enum cfg_screen_item_e {
    CFG_SCREEN_ALL_ITEM_LIST(CFG_ENUM_SCREEN)
};

struct strbf_s;
bool config_screen_value_str(size_t index, struct strbf_s *sb, uint8_t* type);
uint8_t get_screen_item_values(size_t index, struct strbf_s *sb);
bool get_screen_item_descriptions(size_t index, struct strbf_s *sb);
bool config_screen_set_next_item(size_t index);
uint8_t config_screen_get_next_value(size_t index);

#ifdef __cplusplus
}
#endif

#endif /* A4A1AE36_A443_46AA_BCC0_3D334F6DD429 */
