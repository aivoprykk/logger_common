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
#include "config_wifi.h"
#include "config_admin.h"
#include "config_ubx.h"
#include "config_gps.h"
#include "config_advanced.h"
#include "config_manager.h"

#define SCFG_GROUP_LIST(l) \
l(UBX) \
l(GPS) \
l(SCREEN) \
l(FW_UPDATE)

#define SCFG_GROUP_SCR_LIST(l) \
l(STAT_SCREENS)

#define SCFG_GROUP_OTHER_LIST(l) \
SCFG_GRP_WIFI(l) \
l(ADVANCED) \
SCFG_GRP_ADMIN(l)

// Configuration groups (for screen/REST API access)
// These represent conceptual groups, not individual items
#define ENUM_ALL(l) SCFG_GROUP_##l,
typedef enum sconfig_group_e {
    SCFG_GROUP_LIST(ENUM_ALL)
    SCFG_GROUP_OTHER_LIST(ENUM_ALL)
    SCFG_GROUP_COUNT
} sconfig_group_t;

#define ENUM_CYCLE(l) SCFG_CYCLE_GROUP_##l,
typedef enum sconfig_cycle_group_e { 
    SCFG_GROUP_LIST(ENUM_CYCLE)
    SCFG_GROUP_SCR_LIST(ENUM_CYCLE)
    SCFG_CYCLE_GROUP_COUNT
} sconfig_cycle_group_t;

const char * sconfig_group_names(sconfig_group_t id);
bool config_manager_is_group_default_hidden(sconfig_group_t group);

#ifdef __cplusplus
}
#endif

#endif /* C587243F_DD74_4424_AF56_5063E0A655EE */
