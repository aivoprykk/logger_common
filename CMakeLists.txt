
SET(SRCS
    logger_common.c
    logger_buffer_pool.c
)

if(CONFIG_LOGGER_VIBRATION_ENABLED)
    list(APPEND SRCS
        logger_vibration.c
    )
endif()

if(CONFIG_LOGGER_COMMON_USE_CFG)
    list(APPEND SRCS
        cfg/config_lock.c
        cfg/config_manager.c
        cfg/config_manager_admin.c
        cfg/config_manager_advanced.c
        cfg/config_manager_fw_update.c
        cfg/config_manager_gps.c
        cfg/config_manager_wifi.c
        cfg/config_manager_screen.c
        cfg/config_manager_ubx.c
        cfg/config_observer.c
        cfg/config_registry.c
        cfg/config_storage.c
        cfg/unified_config.c
    )

    if(CONFIG_GPS_LOG_USE_CJSON OR CONFIG_GPS_LOG_USE_CCAN_JSON)
        list(APPEND SRCS
            common_cfg.c
        )
    endif()
endif()

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include" "."
    REQUIRES esp_system esp_event esp_timer wear_levelling nvs_flash
)

if(CONFIG_LOGGER_COMMON_USE_CFG)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::strutil)
endif()

if(CONFIG_GPS_LOG_USE_CJSON)
    target_link_libraries(${COMPONENT_LIB} PUBLIC idf::json)
endif()

if(CONFIG_GPS_LOG_USE_CCAN_JSON)
    target_link_libraries(${COMPONENT_LIB} PUBLIC idf::ccan_json)
endif()
if(CONFIG_LOGGER_VIBRATION_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::driver)
endif()