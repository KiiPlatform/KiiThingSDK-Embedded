#ifndef KII_JSON_H
#define KII_JSON_H

#include "kii.h"

#define KII_JSON_OBJECT jsmntok_t*

int prv_kii_json_parse(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        KII_JSON_OBJECT* out_object);

int prv_kii_json_get_object(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        const KII_JSON_OBJECT root,
        const char* name,
        KII_JSON_OBJECT* out_object);

int prv_kii_json_get_object_value(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        const KII_JSON_OBJECT object,
        char* out_buf,
        int out_buf_size);

#endif
