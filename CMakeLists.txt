
SET(SRCS
    config_lock.c
    logger_common.c
    common_cfg.c
    logger_buffer_pool.c
    config_manager.c
    config_manager_admin.c
    config_manager_advanced.c
    config_manager_fw_update.c
    config_manager_gps.c
    config_manager_main.c
    config_manager_screen.c
    config_manager_ubx.c
    config_observer.c
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    REQUIRES esp_system esp_event esp_timer sconfig strutil
    PRIV_REQUIRES logger_config
)

if(CONFIG_GPS_LOG_USE_CJSON)
    target_link_libraries(${COMPONENT_LIB} PUBLIC idf::json)
endif()
if(CONFIG_GPS_LOG_USE_CCAN_JSON)
    target_link_libraries(${COMPONENT_LIB} PUBLIC idf::ccan_json)
endif()