
idf_component_register(
    SRCS "logger_common.c" "common_cfg.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_system esp_event
    PRIV_REQUIRES esp_timer
)

if(CONFIG_GPS_LOG_USE_CJSON)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::json)
else()
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::ccan_json)
endif()