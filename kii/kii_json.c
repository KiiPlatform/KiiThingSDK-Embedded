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

static void prv_kii_json_set_error_message(
        kii_json_t* kii_json,
        const char* message)
{
    strncpy(kii_json->error_string_buff, message,
            kii_json->error_string_length);
    kii_json->error_string_buff[kii_json->error_string_length - 1] = '\0';
}

static int prv_jsmn_token_num(const char* json_string, size_t json_string_len)
{
    jsmn_parser parser;

    assert(json_string != NULL);
    assert(json_string_len >= 0);

    jsmn_init(&parser);

    return jsmn_parse(&parser, json_string, json_string_len, NULL, 0);
}

static int prv_kii_jsmn_get_tokens(
        kii_json_t* kii_json,
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
        prv_kii_json_set_error_message(kii_json,
                "Not enough tokens were provided");
    } else if (parse_result == JSMN_ERROR_INVAL) {
        ret = -1;
        prv_kii_json_set_error_message(kii_json,
                "Invalid character inside JSON string");
    } else if (parse_result == JSMN_ERROR_PART) {
        ret = -1;
        prv_kii_json_set_error_message(kii_json,
                "The string is not a full JSON packet, more bytes expected");
    } else {
        char buf[50];
        ret = -1;
        snprintf(buf, sizeof(buf) / sizeof(buf[0]),
                "Unexpected error: %d", parse_result);
        buf[31] = '\0';
        prv_kii_json_set_error_message(kii_json, buf);
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

static kii_json_field_type_t prv_kii_json_to_kii_json_field_type(
        jsmntype_t jsmn_type)
{
    switch (jsmn_type)
    {
        case JSMN_PRIMITIVE:
            return KII_JSON_FIELD_TYPE_PRIMITIVE;
        case JSMN_OBJECT:
            return KII_JSON_FIELD_TYPE_OBJECT;
        case JSMN_ARRAY:
            return KII_JSON_FIELD_TYPE_ARRAY;
        case JSMN_STRING:
            return KII_JSON_FIELD_TYPE_STRING;
        default:
            /* programming error */
            assert(0);
            return KII_JSON_FIELD_TYPE_PRIMITIVE;
    }
}

static int prv_kii_json_jsmn_string_to_kii_json_string(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_object_to_kii_json_object(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_array_to_kii_json_array(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_primitive_to_kii_json_primitive(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_primitive_to_kii_json_integer(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_primitive_to_kii_json_long(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_primitive_to_kii_json_double(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static int prv_kii_json_jsmn_primitive_to_kii_json_boolean(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;

}
static int prv_kii_json_jsmn_primitive_to_kii_json_null(
        const jsmntok_t* token,
        const char* json_string,
        kii_json_field_t* field)
{
    // TODO: implement me.
    return 0;
}

static kii_json_parse_result_t prv_kii_json_check_object_fields(
        const char* json_string,
        size_t json_string_len,
        const jsmntok_t* tokens,
        kii_json_field_t* fields)
{
    int i;
    kii_json_parse_result_t retval = KII_JSON_PARSE_SUCCESS;

    switch (tokens[0].type)
    {
        case JSMN_ARRAY:
            return KII_JSON_PARSE_ROOT_TYPE_ERROR;
        case JSMN_OBJECT:
            break;
        default:
            return KII_JSON_PARSE_INVALID_INPUT;
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
            retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
            field->result = KII_JSON_FIELD_PARSE_NOT_FOUND;
            continue;
        }

        if (field->type == KII_JSON_FIELD_TYPE_ANY) {
            field->type = prv_kii_json_to_kii_json_field_type(value->type);
        }

        field->start = value->start;
        field->end = value->end;

        switch (field->type) {
            case KII_JSON_FIELD_TYPE_STRING:
                if (prv_kii_json_jsmn_string_to_kii_json_string(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_OBJECT:
                if (prv_kii_json_jsmn_object_to_kii_json_object(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_ARRAY:
                if ( prv_kii_json_jsmn_array_to_kii_json_array(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_PRIMITIVE:
                if (prv_kii_json_jsmn_primitive_to_kii_json_primitive(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_INTEGER:
                if (prv_kii_json_jsmn_primitive_to_kii_json_integer(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_LONG:
                if (prv_kii_json_jsmn_primitive_to_kii_json_long(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_DOUBLE:
                if (prv_kii_json_jsmn_primitive_to_kii_json_double(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_BOOLEAN:
                if (prv_kii_json_jsmn_primitive_to_kii_json_boolean(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_NULL:
                if (prv_kii_json_jsmn_primitive_to_kii_json_null(value,
                                json_string, field) == 0) {
                    retval = KII_JSON_PARSE_PARTIAL_SUCCESS;
                }
                break;
            case KII_JSON_FIELD_TYPE_ANY:
            default:
                /* programming error */
                assert(0);
                return 0;
        }
    }

    return retval;
}

kii_json_parse_result_t kii_json_read_object(
        kii_json_t* kii_json,
        const char* json_string,
        size_t json_string_len,
        kii_json_field_t* fields)
{
    kii_json_parse_result_t ret = KII_JSON_PARSE_INVALID_INPUT;
    int result = -1;
    jsmntok_t tokens[KII_JSON_TOKEN_NUM];

    assert(json_string != NULL);
    assert(json_string_len > 0);

    result = prv_kii_jsmn_get_tokens(kii_json, json_string, json_string_len,
            tokens, sizeof(tokens) / sizeof(tokens[0]));
    if (result == 0)
    {
        ret = prv_kii_json_check_object_fields(json_string, json_string_len,
                tokens, fields);
    }

    return ret;
}
