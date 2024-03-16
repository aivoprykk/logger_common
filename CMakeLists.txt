
idf_component_register(
    SRCS "logger_common.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_system
    PRIV_REQUIRES esp_timer
)
