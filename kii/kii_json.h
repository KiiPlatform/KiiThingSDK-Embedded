#ifndef KII_UTILS_H
#define KII_UTILS_H

#include "kii.h"

int prv_kii_jsmn_get_tokens(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        jsmntok_t* tokens,
        size_t token_num);

int prv_kii_jsmn_copy_string_value(
        const char* json_string,
        size_t json_string_len,
        jsmntok_t* toknes,
        const char* key,
        char* coping_buf);

#endif
