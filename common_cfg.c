#include "common_cfg.h"
#include "logger_common.h"
#if defined(CONFIG_GPS_LOG_USE_CJSON)
#include "cJSON.h"
#else
#include "json.h"
#endif
#include <string.h>

uint8_t set_hhu(void *item, uint8_t *val, uint8_t def) {
#if defined(CONFIG_GPS_LOG_USE_CJSON)
    cJSON *value = (cJSON *)item;
    if (value && value->type == cJSON_Number) {
        if(def || *val != value->valuedouble) {
            *val = value->valuedouble;
#else
    JsonNode *value = (JsonNode *)item;
    if (value && value->tag == JSON_NUMBER) {
        if(def || *val != value->data.number_) {
            *val = value->data.number_;
#endif
            return 0;
        }
        return 1;
    }
    return 255;
}

uint8_t set_u(void *item, uint16_t *val, uint8_t def) {
#if defined(CONFIG_GPS_LOG_USE_CJSON)
    cJSON *value = (cJSON *)item;
    if (value && value->type == cJSON_Number) {
        if(def || *val != value->valuedouble) {
            *val = value->valuedouble;
#else    
    JsonNode *value = (JsonNode *)item;
    if (value && value->tag == JSON_NUMBER) {
        if(def || *val != value->data.number_) {
            *val = value->data.number_;
#endif
            return 0;
        }
        return 1;
    }
    return 255;
}

uint8_t set_f(void *item, float *val, uint8_t def) {
#if defined(CONFIG_GPS_LOG_USE_CJSON)
    cJSON *value = (cJSON *)item;
    if (value && value->type == cJSON_Number) {
        if(def || *val != value->valuedouble) {
            *val = value->valuedouble;
#else
    JsonNode *value = (JsonNode *)item;
    if (value && value->tag == JSON_NUMBER) {
        if(def || *val != value->data.number_) {
            *val = value->data.number_;
#endif
            return 0;
        }
        return 1;
    }
    return 255;
}

uint8_t set_bit(void *item, uint8_t *val, int bit, uint8_t def) {
#if defined(CONFIG_GPS_LOG_USE_CJSON)
    cJSON *value = (cJSON *)item;
    if (value && value->type == cJSON_Number) {
        if(def || GETBIT(*val, bit) != value->valuedouble) {
            if(value->valuedouble == 0) CLRBIT(*val, bit);
            else SETBIT(*val, bit);
#else
    JsonNode *value = (JsonNode *)item;
    if (value && value->tag == JSON_NUMBER) {
        if(def || GETBIT(*val, bit) != value->data.number_) {
            if(value->data.number_ == 0) CLRBIT(*val, bit);
            else SETBIT(*val, bit);
#endif
            return 0;
        }
        return 1;
    }
    return 255;
}

uint8_t set_c(void *item, char *val, uint8_t def) {
#if defined(CONFIG_GPS_LOG_USE_CJSON)
    cJSON *value = (cJSON *)item;
    if (value && value->type == cJSON_String) {
        if(def || strcmp(val, value->valuestring)) {
            size_t len = strlen(value->valuestring);
            memcpy(val, value->valuestring, len);
#else
    JsonNode *value = (JsonNode *)item;
    if (value && value->tag == JSON_STRING) {
        if(def || strcmp(val, value->data.string_)) {
            size_t len = strlen(value->data.string_);
            memcpy(val, value->data.string_, len);
#endif
            val[len] = 0;
            return 0;
        }
        return 1;
    }
    return 255;
}
