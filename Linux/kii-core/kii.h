#ifndef _kii_
#define _kii_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/** bool type definition */
typedef enum kii_bool_t {
    KII_FALSE = 0,
    KII_TRUE
} kii_bool_t;

/** HTTP client code returned by callback implementation */
typedef enum kii_http_client_code_t {
    /** retrun this ocde when operation completed. */
    KII_HTTPC_OK = 0,
    /** return this code when operation failed. */
    KII_HTTPC_FAIL,
    /** return this code when operation is in progress.
     *  SDK calls the callback again until the operation
     *  completed or failed.
     */
    KII_HTTPC_AGAIN
} kii_http_client_code_t;

/** callback for preparing HTTP request line.
 * application implement this callback with the HTTP client
 * in the target environment.
 * @return KII_HTTPC_OK on success, KII_HTTPC_FAIL on error.<br>
 * do not return KII_HTTPC_AGAIN from this callback.
 * @param [in] http_context context object defined by application.
 * @param [in] method HTTP method
 * @param [in] host host name
 * @param [in] path resource path following to host in uri.
 */
typedef kii_http_client_code_t
        (*KII_HTTPCB_SET_REQUEST_LINE)(
                void* http_context,
                const char* method,
                const char* host,
                const char* path);
/** callback for preparing HTTP request header.
 * application implement this callback with the HTTP client
 * in the target environment.<br>
 * this callback is called per one header line.
 * @return KII_HTTPC_OK on success, KII_HTTPC_FAIL on error.<br>
 * do not return KII_HTTPC_AGAIN from this callback.
 * @param [in] http_context context object defined by application.
 * @param [in] key name of the header field.
 * @param [in] value value of the header field.
 */
typedef kii_http_client_code_t
        (*KII_HTTPCB_SET_HEADER)(
                void* http_context,
                const char* key,
                const char* value);

/** callback for preparing HTTP request body.
 * application implement this callback with the HTTP client
 * in the target environment.<br>
 * this callback is called per one header line.
 * @return KII_HTTPC_OK on success, KII_HTTPC_FAIL on error.<br>
 * do not return KII_HTTPC_AGAIN from this callback.
 * @param [in] http_context context object defined by application.
 * @param [in] body_data request body data.
 */
typedef kii_http_client_code_t
        (*KII_HTTPCB_SET_BODY)(
                void* http_context,
                const char* body_data);

/** callback for execution of HTTP request.
 * application implement this callback with the HTTP client
 * in the target environment.<br>
 * @return KII_HTTPC_OK on success, KII_HTTPC_FAIL on error.<br>
 * KII_HTTPC_AGAIN can be retuned from this callback.<br>
 * This option may be useful if you execute kii_run() in function which
 * takes care of other events.
 * @param [out] response_code HTTP response code
 * @param [out] response_body pointer refers the HTTP response body 
 */
typedef kii_http_client_code_t
        (*KII_HTTPCB_EXECUTE)(
                void* http_context,
                int* response_code,
                char** response_body);

/** callback for logging.
 * SDK uses this function for logging.
 * If you want to enable logging,
 * set pointer of this function in kii_t#logger_cb.
 * Logging is only enabled in DEBUG build.
 */
typedef void
        (*KII_LOGGER)(
                const char* format,
                ...
                );
/** error code returned by SDK apis. */
typedef enum kii_error_code_t {
    KIIE_OK = 0,
    KIIE_FAIL
} kii_error_code_t;

/** represents state of SDK. */
typedef enum kii_state_t {
    /** SDK is idle. */
    KII_STATE_IDLE = 0,
    /** SDK is ready to execute kii_run() */
    KII_STATE_READY,
    /** SDK is executing request. kii_run() should be called to finish
     * operation
     */
    KII_STATE_EXECUTE
} kii_state_t;


/** represents author of SDK api. */
typedef struct kii_author_t
{
    /** ID of the author */
    char* author_id;
    /** access token of the author */
    char* access_token;
} kii_author_t;

/** object manages context of api execution. */
typedef struct kii_t
{
    /** Kii Cloud application id */
    char* app_id;
    /** Kii Cloud application key */
    char* app_key;
    /** Kii Cloud application host.
     *  value is depending on your site of application.
     *  Site JP : "api-jp.kii.com"
     *  Site US : "api.kii.com"
     *  Site CN : "api-cn2.kii.com"
     *  Site SG : "api-sg.kii.com"
     */
    char* app_host;
    /** buffer used to communicate with KiiCloud.
     *  application allocate memory before calling apis.
     */
    char* buffer;
    /** size of buffer */
    size_t buffer_size;
    /** HTTP response code.
     * value is set by implementation of KII_HTTPCB_EXECUTE
     */
    int response_code;
    /** HTTP response body 
     * value is set by implementation of KII_HTTPCB_EXECUTE
     */
    char* response_body;
    /** author of the api.
     * set author object before execute api requires authorization.
     */
    kii_author_t* author;

    /** application's context object used by HTTP callback implementations.
     * Should be allocated and set before execute apis.
     */
    void* http_context;
    /** request line callback function pointer
     * Should be set before execute apis.
     */
    KII_HTTPCB_SET_REQUEST_LINE http_set_request_line_cb;
    /** request header callback function pointer
     * Should be set before execute apis.
     */
    KII_HTTPCB_SET_HEADER http_set_header_cb;
    /** request body callback function pointer
     * Should be set before execute apis.
     */
    KII_HTTPCB_SET_BODY http_set_body_cb;
    /** execute HTTP request function pointer
     * Should be set before execute apis.
     */
    KII_HTTPCB_EXECUTE http_execute_cb;
    /** logging callback function pointer */
    KII_LOGGER logger_cb;
    char _http_request_path[256];

    kii_state_t _state;
} kii_t;


/** represents scope of bucket/ topic. */
typedef enum kii_scope_type_t {
    KII_SCOPE_APP,
    KII_SCOPE_USER,
    KII_SCOPE_GROUP,
    KII_SCOPE_THING
} kii_scope_type_t;

/** represents bucket */
typedef struct kii_bucket_t {
    kii_scope_type_t scope;
    char* scope_id;
    char* bucket_name;
} kii_bucket_t;

/** represents topic */
typedef struct kii_topic_t {
    kii_scope_type_t scope;
    char* scope_id;
    char* topic_name;
} kii_topic_t;

/** obtain current state of SDK.
 * @return state of SDK.
 */
kii_state_t kii_get_state(kii_t* kii);

/** execute HTTP request.
 * application calls this method again
 * until the state becomes KII_STATE_IDLE,
 * @return result of execution.
 */
kii_error_code_t kii_run(kii_t* kii);

/** prepare request of regiser thing.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] thing_data JSON object represents thing to be registered.<br>
 * for details of format, please refer to<br>
 * http://documentation.kii.com/rest/apps-collection/application/thing-collection/#method-thingsResourceType-POST
 */
kii_error_code_t
kii_register_thing(kii_t* kii,
        const char* thing_data);

/** prepare request of thing authentication.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] vendor_thing_id thing id given by vendor on registration.
 * @param [in] password password of thing given by vendor on registration.
 */
kii_error_code_t
kii_thing_authentication(kii_t* kii,
        const char* vendor_thing_id,
        const char* password);

/** prepare request of create object.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to which object belongs.
 * @param [in] object_data key-value of object data in JSON.
 * @param [in] opt_object_content_type content type of object.<br>
 * if NULL is given, "application/json" is used.
 */
kii_error_code_t
kii_create_new_object(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_data,
        const char* opt_object_content_type
        );

/** prepare request of create object with id.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to which object belongs.
 * @param [in] object_id id of the object.
 * @param [in] object_data key-value of object data in JSON.
 * @param [in] opt_object_content_type content type of object.<br>
 * if NULL is given, "application/json" is used.
 */
kii_error_code_t
kii_create_new_object_with_id(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_id,
        const char* object_data,
        const char* opt_object_content_type
        );

/** prepare request of patch object.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to which object belongs.
 * @param [in] object_id id of the object.
 * @param [in] patch_data key-value of patch data in JSON.<br>
 * key-value pair which is not contained this patch
 * is not updated after execution.
 * @param [in] opt_etag etag of the object. if this value is specified,
 * if-match header is sent to Kii Cloud. in a result,
 * patch request only success when the version of the object in client and
 * server matches.<br>
 * If NULL is given, no version check is executed and patch is forcibly applied.
 */
kii_error_code_t
kii_patch_object(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_id,
        const char* patch_data,
        const char* opt_etag);

/** prepare request of replace object.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to which object belongs.
 * @param [in] object_id id of the object.
 * @param [in] replace_data key-value of replacement data in JSON.<br>
 * key-value pair which is not containd this data is deleted after execution.
 * @param [in] opt_etag etag of the object. if this value is specified,
 * if-match header is sent to Kii Cloud. in a result,
 * replace request only success when the version of the object in client and
 * server matches.<br>
 * If NULL is given,
 * no version check is executed and replace is forcibly applied.
 */
kii_error_code_t
kii_replace_object(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_id,
        const char* replace_data,
        const char* opt_etag);

/** prepare request of get object.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to which object belongs.
 * @param [in] object_id id of the object.
 */
kii_error_code_t
kii_get_object(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_id);

/** prepare request of delete object.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket object to be created.
 * @param [in] object_id id of the object.
 */
kii_error_code_t
kii_delete_object(
        kii_t* kii,
        const kii_bucket_t* bucket,
        const char* object_id);

/** prepare request of subscribe bucket.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * after the execution of request succeeded, you can receive push notification
 * when event happens in the bucket.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to be subscribed.
 */
kii_error_code_t
kii_subscribe_bucket(kii_t* kii,
        const kii_bucket_t* bucket);

/** prepare request of subscribe bucket.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] bucket to be unsubscribed.
 */
kii_error_code_t
kii_unsubscribe_bucket(kii_t* kii,
        const kii_bucket_t* bucket);

/** prepare request of create topic.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * after execution of the request succeeded, you can subscribe topic to receive
 * message sent to the topic.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] topic to be created.
 */
kii_error_code_t
kii_create_topic(kii_t* kii,
        const kii_topic_t* topic);

/** prepare request of delete topic.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] topic to be deleted.
 */
kii_error_code_t
kii_delete_topic(kii_t* kii,
        const kii_topic_t* topic);

/** prepare request of subscribe topic.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * after the execution of request succeeded, you can receive push notification
 * when the message is sent to the topic.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] topic to be subscribed.
 */
kii_error_code_t
kii_subscribe_topic(kii_t* kii,
        const kii_topic_t* topic);

/** prepare request of unsubscribe topic.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] topic to be unsubscribed.
 */
kii_error_code_t
kii_unsubscribe_topic(kii_t* kii,
        const kii_topic_t* topic);

/** prepare request of install push for thing.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * application get installationRegistrationID field in the response body to
 * obtain MQTT endpoint.
 * for details of response, please refer to<br>
 * http://documentation.kii.com/rest/apps-collection/application/installations/
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] development flag indicate whether the installation is
 * development of production.
 */
kii_error_code_t
kii_install_thing_push(kii_t* kii,
        kii_bool_t development);

/** prepare request of get MQTT endpoint.
 * after this method succeeded, state of SDK becomes KII_STATE_READY.<br>
 * execute kii_run() to send the request to Kii Cloud.
 * application get the information of MQTT endpoint from the response body
 * to connect and receive push notification sent from Kii Cloud
 * for details of response, please refer to<br>
 * http://documentation.kii.com/rest/#notification_management-manage_the_user_thing_device_installation-get_the_mqtt_endpoint_information
 * @return result of preparation.
 * @param [in] kii SDK object.
 * @param [in] installation_id obtained from response body of
 * kii_install_thing_push()
 */
kii_error_code_t
kii_get_mqtt_endpoint(kii_t* kii,
        const char* installation_id);

#ifdef __cplusplus
}
#endif

#endif
