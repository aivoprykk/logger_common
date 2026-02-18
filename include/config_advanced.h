#ifndef B2E3599F_F460_4975_A544_C72853DC3654
#define B2E3599F_F460_4975_A544_C72853DC3654

#ifdef __cplusplus
extern "C" {
#endif
#include "config_manager.h"
// NVS keys for each module (moved from unified_config.c)
#define NVS_KEY_ADVANCED "adv_cfg"

#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
#define CFG_STAT_SCREEN_TIME_ITEMS(l) l(stat_screens_time)
#else
#define CFG_STAT_SCREEN_TIME_ITEMS(l)
#endif
#if defined(CONFIG_LCD_IS_EPD)
#define CFG_SCREEN_MOVE_OFFSET_ITEMS(l) l(screen_move_offset)
#else
#define CFG_SCREEN_MOVE_OFFSET_ITEMS(l)
#endif

#define CFG_ADVANCED_PRIVATE_ITEM_LIST(l)                                      \
	l(speed_field_count) CFG_SCREEN_MOVE_OFFSET_ITEMS(l)

#define CFG_ADVANCED_PUBLIC_ITEM_LIST(l)                                       \
	l(stat_speed) l(archive_days) l(hostname) l(experimental_features)         \
		CFG_STAT_SCREEN_TIME_ITEMS(l)

#define CFG_ADVANCED_ITEM_LIST(l) CFG_ADVANCED_PUBLIC_ITEM_LIST(l)
// CFG_ADVANCED_PRIVATE_ITEM_LIST(l)

extern const char *const config_advanced_items[];
extern const size_t config_advanced_item_count;

typedef struct cfg_advanced_s {
	uint8_t speed_field_count;
	uint16_t
		archive_days;  // how many days files will be moved to the "Archive" dir
	char hostname[32]; // your hostname
	uint8_t stat_screen_speed;	   // max speed in m/s for showing Stat screens
	uint8_t experimental_features; // bool for enabling experimental features
#if defined(CONFIG_LOGGER_STAT_SCREEN_ROTATION)
	uint8_t stat_screens_time; // time between switching stat_screens
#define STAT_SCREEN_TIME_DEFAULT .stat_screens_time = 2,
#else
#define STAT_SCREEN_TIME_DEFAULT
#endif
#if defined(CONFIG_LCD_IS_EPD)
	uint8_t screen_move_offset; // EPD move offset
#define SCREEN_MOVE_OFFSET_DEFAULT .screen_move_offset = 1,
#else
#define SCREEN_MOVE_OFFSET_DEFAULT
#endif
} cfg_advanced_t;

#define CFG_ADVANCED_DEFAULTS()                                                \
	{.speed_field_count = 10,                                                  \
	 .archive_days = 30,                                                       \
	 .hostname = "esp",                                                        \
	 .stat_screen_speed = 1,                                                   \
	 .experimental_features = 0,                                               \
	 STAT_SCREEN_TIME_DEFAULT SCREEN_MOVE_OFFSET_DEFAULT}

#define CFG_ENUM_ADVANCED(l) cfg_advanced_##l,
enum cfg_advanced_item_e { CFG_ADVANCED_ITEM_LIST(CFG_ENUM_ADVANCED) };

struct strbf_s;
bool config_advanced_value_str(size_t index, struct strbf_s *sb, uint8_t *type);
uint8_t get_advanced_item_values(size_t index, struct strbf_s *sb);
bool get_advanced_item_descriptions(size_t index, struct strbf_s *sb);
// Advanced settings group functions (brightness/move offset)
bool config_advanced_get_item(size_t index, config_item_info_t *info);
bool config_advanced_set_item(size_t index, const char *value);
uint8_t config_advanced_handle(void);
void config_advanced_register(void);

#ifdef __cplusplus
}
#endif

#endif /* B2E3599F_F460_4975_A544_C72853DC3654 */
