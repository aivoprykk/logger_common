#ifndef CBB4ADE6_CD39_491C_9028_E38A4EF649FD
#define CBB4ADE6_CD39_491C_9028_E38A4EF649FD
#ifdef __cplusplus
extern "C" {
#endif
#include "config_manager.h"
// Firmware update configuration items

#if defined(CONFIG_LOGGER_BUILD_MODE_DEV)
#define CFG_FW_UPDATE_CHANNEL 1
#else
#define CFG_FW_UPDATE_CHANNEL 0
#endif

// NVS keys for each module (moved from unified_config.c)
#define NVS_KEY_FW_UPDATE "fw_cfg"

#define CFG_FW_UPDATE_ITEM_LIST(l) l(update_enabled) l(update_channel)
extern const char *const config_fw_update_items[];
extern const size_t config_fw_update_item_count;

#define CFG_FW_UPDATE_CHANNEL_ITEM_LIST(l) l(stable) l(unstable)
extern const char *const config_fw_update_channels[];
extern const size_t config_fw_update_channels_count;

// Firmware update configuration structure
typedef struct cfg_fw_update_s {
	uint16_t version;
	uint8_t update_enabled; // Enable firmware updates
	uint8_t update_channel; // Update channel: 0=stable, 1=unstable
} __attribute__((packed, aligned(4))) cfg_fw_update_t;

#define CFG_FW_UPDATE_DEFAULTS()                                               \
	{.version = 1, .update_enabled = 1, .update_channel = CFG_FW_UPDATE_CHANNEL}

#define CFG_ENUM_FW(l) cfg_fw_update_##l,
enum cfg_fw_update_item_e { CFG_FW_UPDATE_ITEM_LIST(CFG_ENUM_FW) };

typedef enum {
	FW_UPDATE_CHANNEL_PROD,
	FW_UPDATE_CHANNEL_DEV,
} fw_update_channel_t;

struct strbf_s;
bool config_fw_update_value_str(size_t index, struct strbf_s *sb,
								uint8_t *type);
uint8_t get_fw_update_item_values(size_t index, struct strbf_s *sb);
bool get_fw_update_item_descriptions(size_t index, struct strbf_s *sb);
bool config_fw_update_set_next_value(
	size_t index); // Cycle to next value (for UI)
// Firmware update group functions
bool config_fw_update_get_item(size_t index, config_item_info_t *info);
bool config_fw_update_set_item(size_t index, const char *value);
uint8_t config_fw_update_handle(void);
void config_fw_update_register(void);

#ifdef __cplusplus
}
#endif

#endif /* CBB4ADE6_CD39_491C_9028_E38A4EF649FD */
