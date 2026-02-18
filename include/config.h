#ifndef C587243F_DD74_4424_AF56_5063E0A655EE
#define C587243F_DD74_4424_AF56_5063E0A655EE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// Configuration Item Type Definitions (migrated from sconfig)
// ============================================================================

typedef enum {
	SCONFIG_ITEM_TYPE_BOOL = 0,
	SCONFIG_ITEM_TYPE_INT8,
	SCONFIG_ITEM_TYPE_INT16,
	SCONFIG_ITEM_TYPE_INT32,
	SCONFIG_ITEM_TYPE_INT64,
	SCONFIG_ITEM_TYPE_UINT8,
	SCONFIG_ITEM_TYPE_UINT16,
	SCONFIG_ITEM_TYPE_UINT32,
	SCONFIG_ITEM_TYPE_UINT64,
	SCONFIG_ITEM_TYPE_STRING,
	SCONFIG_ITEM_TYPE_BLOB,
	SCONFIG_ITEM_TYPE_COLOR,
	SCONFIG_ITEM_TYPE_IP,
	SCONFIG_ITEM_TYPE_MAX
} sconfig_item_type_t;

typedef union {
	bool bool1;
	int8_t int8;
	int16_t int16;
	int32_t int32;
	int64_t int64;
	uint8_t uint8;
	uint16_t uint16;
	uint32_t uint32;
	uint64_t uint64;
	char *str;
	struct blob {
		uint8_t *data;
		size_t length;
	} blob;
} sconfig_item_value_t;

typedef struct config_item {
	char *key;
	sconfig_item_type_t type;
	bool secret;
	sconfig_item_value_t def;

	// Optional metadata for enhanced functionality
	const char *description; // Human-readable description
	const char *
		*enum_names; // Array of string names for enum values (NULL terminated)
	size_t enum_count;		  // Number of enum values
	sconfig_item_value_t min; // Minimum value (for numeric types)
	sconfig_item_value_t max; // Maximum value (for numeric types)
	uint8_t group_id;		  // Group identifier for logical grouping
} sconfig_item_t;

#define CONFIG_VALUE_UNCHANGED "\x1a\x1a\x1a\x1a\x1a\x1a\x1a\x1a"

// ============================================================================

// #define SCFG_GROUP_LIST(l) l(UBX) l(GPS) l(SCREEN) l(FW_UPDATE)

// #define SCFG_GROUP_SCR_LIST(l) l(STAT_SCREENS)

// #define SCFG_GROUP_OTHER_LIST(l)
// 	SCFG_GRP_WIFI(l)
// 	l(ADVANCED) SCFG_GRP_ADMIN(l)

// // Configuration groups (for screen/REST API access)
// // These represent conceptual groups, not individual items
// #define ENUM_ALL(l) SCFG_GROUP_##l,
// typedef enum sconfig_group_e {
// 	SCFG_GROUP_LIST(ENUM_ALL) SCFG_GROUP_OTHER_LIST(ENUM_ALL) SCFG_GROUP_COUNT
// } sconfig_group_t;

// #define ENUM_CYCLE(l) SCFG_CYCLE_GROUP_##l,
// typedef enum sconfig_cycle_group_e {
// 	SCFG_GROUP_LIST(ENUM_CYCLE) SCFG_GROUP_SCR_LIST(ENUM_CYCLE)
// 		SCFG_CYCLE_GROUP_COUNT
// } sconfig_cycle_group_t;

const char *sconfig_group_names(uint8_t id);
bool config_manager_is_group_default_hidden(uint8_t group);

#ifdef __cplusplus
}
#endif

#endif /* C587243F_DD74_4424_AF56_5063E0A655EE */
