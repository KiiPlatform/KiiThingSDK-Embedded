#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <jsmn.h>

#include "kii_json.h"

#ifndef KII_JSON_TOKEN_NUM
/**
 * @def KII_JSON_TOKEN_NUM
 * @brief Json token size
 *
 * KII_JSON_TOKEN_NUM defines size of JSON can be parsed. By default
 * it is set to 128. If you've got error on JSON parsing in SDK, You
 * can increase the size of KII_JSON_TOKEN_NUM so that avoid error on
 * parsing large JSON. To change the size, please specify the size of
 * KII_JSON_TOKEN_NUM on build.
 */
#define KII_JSON_TOKEN_NUM 128
#endif


static int prv_jsmn_token_num(const char* json_string, size_t json_string_len)
{
    jsmn_parser parser;

    assert(json_string != NULL);
    assert(json_string_len >= 0);

    jsmn_init(&parser);

    return jsmn_parse(&parser, json_string, json_string_len, NULL, 0);
}

static int prv_kii_jsmn_get_tokens(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        jsmntok_t* tokens,
        size_t token_num)
{
    jsmn_parser parser;
    int ret = -1;
    int parse_result = JSMN_ERROR_NOMEM;

    assert(json_string != NULL);
    assert(tokens != NULL);


    jsmn_init(&parser);
    parse_result = jsmn_parse(&parser, json_string, json_string_len, tokens,
            token_num);
    if (parse_result >= 0) {
        ret = 0;
    } else if (parse_result == JSMN_ERROR_NOMEM) {
        ret = -1;
        M_KII_LOG(kii->kii_core.logger_cb(
            "Not enough tokens were provided\r\n"));
    } else if (parse_result == JSMN_ERROR_INVAL) {
        ret = -1;
        M_KII_LOG(kii->kii_core.logger_cb(
            "Invalid character inside JSON string\r\n"));
    } else if (parse_result == JSMN_ERROR_PART) {
        ret = -1;
        M_KII_LOG(kii->kii_core.logger_cb(
            "The string is not a full JSON packet, more bytes expected\r\n"));
    } else {
        ret = -1;
        M_KII_LOG(kii->kii_core.logger_cb(
            "Unexpected error: %d\r\n", parse_result));
    }

    return ret;
}

static int prv_kii_jsmn_get_value(
        const char* json_string,
        size_t json_string_len,
        const jsmntok_t* tokens,
        const char* name,
        jsmntok_t** out_token)
{
    int i = 0;
    int index = 1;
    int ret = -1;

    assert(json_string != NULL);
    assert(tokens != NULL);
    assert(name != NULL && strlen(name) > 0);
    assert(out_token != NULL);

    if (tokens[0].type != JSMN_OBJECT && tokens[0].size < 2) {
        goto exit;
    }

    for (i = 0; i < tokens[0].size; ++i) {
        const jsmntok_t* key_token = tokens + index;
        const jsmntok_t* value_token = tokens + index + 1;
        int key_len = key_token->end - key_token->start;
        if (key_token->type != JSMN_STRING) {
            goto exit;
        }
        if (strlen(name) == key_len &&
                strncmp(name, json_string + key_token->start, key_len) == 0) {
            ret = 0;
            *out_token = (jsmntok_t*)value_token;
            break;
        }
        switch (value_token->type) {
            case JSMN_STRING:
            case JSMN_PRIMITIVE:
                index += 2;
                break;
            case JSMN_OBJECT:
            case JSMN_ARRAY:
                {
                    int num = prv_jsmn_token_num(
                            json_string + value_token->start,
                            json_string_len - value_token->start);
                    if (num < 0) {
                        ret = -1;
                        goto exit;
                    }
                    index += (num + 1);
                }
                break;
        }
    }

exit:
    return ret;
}

static kii_json_parse_result_t prv_kii_json_check_object_fields(
        const char* json_string,
        size_t json_string_len,
        const jsmntok_t* tokens,
        kii_json_field_t* fields)
{
    int i;

    switch (tokens[0].type)
    {
        case JSMN_ARRAY:
            return KII_JSON_PARSE_UNEXPECTED_ARRAY;
        case JSMN_OBJECT:
            break;
        default:
            return KII_JSON_PARSE_INVALID;
    }

    for (i = 0; fields[i].name != NULL; ++i)
    {
        jsmntok_t* value = NULL;
        int result = 0;
        kii_json_field_t* field = &fields[i];

        result = prv_kii_jsmn_get_value(json_string, json_string_len, tokens,
                field->name, &value);
        if (result != 0 || value == NULL)
        {
            field->result = KII_JSON_FIELD_PARSE_NOT_FOUND;
            continue;
        }
        switch (value->type)
        {
            case JSMN_PRIMITIVE:
                field->type = KII_JSON_FIELD_TYPE_PRIMITIVE;
                break;
            case JSMN_OBJECT:
                field->type = KII_JSON_FIELD_TYPE_OBJECT;
                break;
            case JSMN_ARRAY:
                field->type = KII_JSON_FIELD_TYPE_ARRAY;
                break;
            case JSMN_STRING:
                field->type = KII_JSON_FIELD_TYPE_STRING;
                break;
            default:
                /* programming error */
                assert(0);
                break;
        }
        field->start = value->start;
        field->end = value->end;
        if (field->field_copy_buff == NULL) {
            field->result = KII_JSON_FIELD_PARSE_SUCCESS;
        } else {
            size_t len = value->end - value->start;
            if (field->field_copy_buff_size <= len) {
                field->result = KII_JSON_FIELD_PARSE_COPY_FAILED;
            } else {
                memcpy(field->field_copy_buff, json_string + value->start, len);
                field->field_copy_buff[len] = '\0';
                field->result = KII_JSON_FIELD_PARSE_SUCCESS;
            }
        }
    }

    return KII_JSON_PARSE_SUCCESS;
}

kii_json_parse_result_t kii_json_read_object(
        kii_t* kii,
        const char* json_string,
        size_t json_string_len,
        kii_json_field_t* fields)
{
    kii_json_parse_result_t ret = KII_JSON_PARSE_INVALID;
    int result = -1;
    jsmntok_t tokens[KII_JSON_TOKEN_NUM];

    assert(json_string != NULL);
    assert(json_string_len > 0);

    result = prv_kii_jsmn_get_tokens(kii, json_string, json_string_len,
            tokens, sizeof(tokens) / sizeof(tokens[0]));
    if (result == 0)
    {
        ret = prv_kii_json_check_object_fields(json_string, json_string_len,
                tokens, fields);
    }

    return ret;
}

void kii_json_set_field(
        kii_json_field_t* field,
        const char* name,
        char* copy_buff,
        size_t copy_buff_size)
{
    assert(field != NULL);
    assert(name != NULL);

    field->name = name;
    field->field_copy_buff = copy_buff;
    field->field_copy_buff_size = copy_buff_size;
}

int kii_json_is_string_field(kii_t* kii, const kii_json_field_t* field)
{
    assert(kii != NULL);
    assert(field != NULL);

    if (field->type != KII_JSON_FIELD_TYPE_STRING ||
            field->result != KII_JSON_FIELD_PARSE_SUCCESS) {
        M_KII_LOG(kii->kii_core.logger_cb(
            "\"%s\" field is not string: %d %d\r\n",
                field->name, field->type, field->result));
        return 1;
    }
    return 0;
}
