#ifndef KII_JSON_H
#define KII_JSON_H

#include "kii.h"

typedef jsmntok_t* kii_json_object;
typedef struct kii_json_context {
    const char* json_string;
    size_t json_string_len;
    jsmntok_t tokens[KII_JSON_TOKEN_NUM];
} kii_json_context;

int prv_kii_json_context_init(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        kii_json_context* out_context);

int prv_kii_json_context_close(kii_t* kii, kii_json_context* context);

int prv_kii_json_get_root_object(kii_t* kii, const kii_json_context* context);

int prv_kii_json_get_child_value(
        kii_t* kii,
        const kii_json_context* context,
        const kii_json_object parent,
        const char* name,
        kii_json_object* out_value);

int prv_kii_json_get_string(
        kii_t* kii,
        const kii_json_context* context,
        const kii_json_object object,
        char* out_buf,
        int out_buf_size);

// TODO: remove this function.
int prv_kii_jsmn_get_tokens(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        jsmntok_t* tokens,
        size_t token_num);

// TODO: remove this function.
int prv_kii_jsmn_get_value(
        const char* json_string,
        size_t json_string_len,
        const jsmntok_t* tokens,
        const char* name,
        jsmntok_t** out_token);

#endif
