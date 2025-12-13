#ifndef C587243F_DD74_4424_AF56_5063E0A655EE
#define C587243F_DD74_4424_AF56_5063E0A655EE

#ifdef __cplusplus
extern "C" {
#endif
#include "sconfig.h"
#include "config_lock.h"
#include "config_observer.h"
#include "config_screen.h"
#include "config_fw_update.h"
#include "config_main.h"
#include "config_admin.h"
#include "config_ubx.h"
#include "config_gps.h"
#include "config_advanced.h"
#include "config_manager.h"

#define SCFG_GROUP_LIST(l) \
l(GROUP_UBX) \
l(GROUP_GPS) \
l(GROUP_SCREEN) \
l(GROUP_FW_UPDATE)

#define SCFG_GROUP_SCR_LIST(l) \
l(GROUP_STAT_SCREENS)

#define SCFG_GROUP_OTHER_LIST(l) \
l(GROUP_MAIN) \
l(GROUP_ADVANCED) \
l(GROUP_ADMIN)

// Configuration groups (for screen/REST API access)
// These represent conceptual groups, not individual items
#define ENUM_ALL(l) SCFG_##l,
typedef enum sconfig_group_e {
    SCFG_GROUP_LIST(ENUM_ALL)
    SCFG_GROUP_OTHER_LIST(ENUM_ALL)
    SCFG_GROUP_COUNT
} sconfig_group_t;

#define ENUM_CYCLE(l) SCFG_CYCLE_##l,
typedef enum sconfig_cycle_group_e { 
    SCFG_GROUP_LIST(ENUM_CYCLE)
    SCFG_GROUP_SCR_LIST(ENUM_CYCLE)
    SCFG_CYCLE_GROUP_COUNT
} sconfig_cycle_group_t;

#ifdef __cplusplus
}
#endif

#endif /* C587243F_DD74_4424_AF56_5063E0A655EE */
