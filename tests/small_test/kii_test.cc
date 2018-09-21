#include <stdio.h>
#include <stdarg.h>

// Suppress warnings in gtest.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <kii.h>

#define KII_OBJECTID_SIZE 36

#define KII_UPLOADID_SIZE 64

#define DEF_APP_HOST "api-development-jp.internal.kii.com"
#define DEF_APP_ID "84fff36e"
#define DEF_APP_KEY "e45fcc2d31d6aca675af639bc5f04a26"
#define DEF_THING_ID "th.53ae324be5a0-26f8-4e11-a13c-03da6fb2"
#define DEF_ACCESS_TOKEN "ablTGrnsE20rSRBFKPnJkWyTaeqQ50msqUizvR_61hU"
#define DEF_BUCKET "myBucket"
#define DEF_OBJECT "myObject"
#define DEF_TOPIC "myTopic"
#define DEF_DUMMY_KEY "DummyHeader"
#define DEF_DUMMY_VALUE "DummyValue"
#define DEF_DUMMY_HEADER DEF_DUMMY_KEY ":" DEF_DUMMY_VALUE
#define DEF_UPLOAD_ID "MjAxNTEyMDMhODVmNWljMm42ZmRnODltcmgyOWkwd2lvOA"
#define DEF_SDK_VERSION "sn=te;sv=1.2.5"

static char APP_HOST[] = DEF_APP_HOST;
static char APP_ID[] = DEF_APP_ID;
static char APP_KEY[] = DEF_APP_KEY;
static char THING_ID[] = DEF_THING_ID;
static char ACCESS_TOKEN[] = DEF_ACCESS_TOKEN;
static char BUCKET[] = DEF_BUCKET;
static char OBJECT[] = DEF_OBJECT;
static char TOPIC[] = DEF_TOPIC;
static char UPLOAD_ID[] = DEF_UPLOAD_ID;

typedef struct _test_context
{
    const char *send_body;
    int send_counter;
    const char *recv_body;
    int recv_counter;
} test_context_t;

static void logger_cb(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
}

static kii_socket_code_t common_connect_cb(
        kii_socket_context_t* socket_context,
        const char* host,
        unsigned int port)
{
    EXPECT_NE((kii_socket_context_t*)NULL, socket_context);
    EXPECT_STREQ(APP_HOST, host);
    EXPECT_EQ(443, port);

    if (socket_context != NULL) {
        test_context_t* ctx = (test_context_t*)socket_context->app_context;
        EXPECT_NE((test_context_t*)NULL, ctx);
        if (ctx != NULL) {
            ctx->send_counter = 0;
            ctx->recv_counter = 0;
        }
    }

    return KII_SOCKETC_OK;
}

static kii_socket_code_t common_send_cb(
        kii_socket_context_t* socket_context,
        const char* buffer,
        size_t length)
{
    test_context_t* ctx = (test_context_t*)socket_context->app_context;

    EXPECT_NE((char*)NULL, buffer);
    EXPECT_TRUE(0 < length);

    EXPECT_STREQ(&(ctx->send_body[ctx->send_counter]), buffer);
    ctx->send_counter += length;

    return KII_SOCKETC_OK;
}

static kii_socket_code_t common_recv_cb(
        kii_socket_context_t* socket_context,
        char* buffer,
        size_t length_to_read,
        size_t* out_actual_length)
{
    test_context_t* ctx = (test_context_t*)socket_context->app_context;

    EXPECT_NE((char*)NULL, buffer);
    EXPECT_TRUE(0 < length_to_read);

    strncpy(buffer, &(ctx->recv_body[ctx->recv_counter]), length_to_read);
    *out_actual_length = strlen(buffer);
    ctx->recv_counter += *out_actual_length;

    return KII_SOCKETC_OK;
}

static kii_socket_code_t common_close_cb(kii_socket_context_t* socket_context)
{
    EXPECT_NE((kii_socket_context_t*)NULL, socket_context);
    return KII_SOCKETC_OK;
}

static void init(
        kii_t* kii,
        char* buffer,
        int buffer_size,
        test_context_t* test_ctx)
{
    kii_http_context_t* http_ctx;
    memset(kii, 0x00, sizeof(kii_t));

    kii_init(kii, APP_HOST, APP_ID, APP_KEY);

    http_ctx = &(kii->kii_core.http_context);
    http_ctx->buffer = buffer;
    http_ctx->buffer_size = buffer_size;
    http_ctx->connect_cb = common_connect_cb;
    http_ctx->send_cb = common_send_cb;
    http_ctx->recv_cb = common_recv_cb;
    http_ctx->close_cb = common_close_cb;
    http_ctx->socket_context.app_context = test_ctx;

    kii->kii_core.logger_cb = logger_cb;

    strcpy(kii->kii_core.author.author_id, THING_ID);
    strcpy(kii->kii_core.author.access_token, ACCESS_TOKEN);
}

static void initBucket(kii_bucket_t* bucket)
{
    bucket->scope = KII_SCOPE_THING;
    bucket->scope_id = THING_ID;
    bucket->bucket_name = BUCKET;
}

static void initTopic(kii_topic_t* topic)
{
    topic->scope = KII_SCOPE_THING;
    topic->scope_id = THING_ID;
    topic->topic_name = TOPIC;
}

TEST(kiiTest, authenticate)
{
    int err;
    char buffer[4096];
    kii_t kii;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/oauth2/token HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"content-length:59\r\n"
"\r\n"
"{\"username\":\"VENDOR_THING_ID:1426830900\",\"password\":\"1234\"}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/json;charset=UTF-8\r\n"
"Date: Fri, 25 Sep 2015 11:07:16 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 200\r\n"
"X-Varnish: 726929556\r\n"
"Content-Length: 176\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"id\" : \"" DEF_THING_ID "\",\n"
"  \"access_token\" : \"" DEF_ACCESS_TOKEN "\",\n"
"  \"expires_in\" : 2147483639,\n"
"  \"token_type\" : \"Bearer\"\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    strcpy(kii.kii_core.author.author_id, "");
    strcpy(kii.kii_core.author.access_token, "");

    err = kii_thing_authenticate(&kii, "1426830900", "1234");
    ASSERT_EQ(0, err);

    ASSERT_STREQ(THING_ID, kii.kii_core.author.author_id);
    ASSERT_STREQ(ACCESS_TOKEN, kii.kii_core.author.access_token);
}

TEST(kiiTest, register)
{
    int err;
    char buffer[4096];
    kii_t kii;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/vnd.kii.ThingRegistrationAndAuthorizationRequest+json\r\n"
"content-length:67\r\n"
"\r\n"
"{\"_vendorThingID\":\"4792\",\"_password\":\"1234\",\"_thingType\":\"my_type\"}";
    const char* recv_body =
"HTTP/1.1 201 Created\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ThingRegistrationAndAuthorizationResponse+json;charset=UTF-8\r\n"
"Date: Tue, 29 Sep 2015 05:42:52 GMT\r\n"
"Location: https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 201\r\n"
"X-Varnish: 1950832960\r\n"
"Content-Length: 208\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"_thingID\" : \"" DEF_THING_ID "\",\n"
"  \"_vendorThingID\" : \"4972\",\n"
"  \"_created\" : 1443505372909,\n"
"  \"_disabled\" : false,\n"
"  \"_accessToken\" : \"" DEF_ACCESS_TOKEN "\"\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    strcpy(kii.kii_core.author.author_id, "");
    strcpy(kii.kii_core.author.access_token, "");

    err = kii_thing_register(&kii, "4792", "my_type", "1234");
    ASSERT_EQ(0, err);

    ASSERT_STREQ(THING_ID, kii.kii_core.author.author_id);
    ASSERT_STREQ(ACCESS_TOKEN, kii.kii_core.author.access_token);
}

TEST(kiiTest, object_create)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    char object_id[KII_OBJECTID_SIZE + 1];
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 201 Created\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectCreationResponse+json;charset=UTF-8\r\n"
"Date: Wed, 30 Sep 2015 06:18:20 GMT\r\n"
"ETag: \"1\"\r\n"
"Location: https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/rw:" DEF_BUCKET "/objects/10e9d740-673b-11e5-ac56-123143070e33\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 201\r\n"
"X-Varnish: 734543441\r\n"
"Content-Length: 123\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"objectID\" : \"10e9d740-673b-11e5-ac56-123143070e33\",\n"
"  \"createdAt\" : 1443593908916,\n"
"  \"dataType\" : \"application/json\"\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_create(&kii, &bucket, "{}", "application/json", object_id);
    ASSERT_EQ(0, err);

    ASSERT_STREQ("10e9d740-673b-11e5-ac56-123143070e33", object_id);
}

TEST(kiiTest, object_create_with_id)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"PUT https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 201 Created\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Wed, 30 Sep 2015 06:51:15 GMT\r\n"
"ETag: \"1\"\r\n"
"Location: https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/rw:" DEF_BUCKET "/objects/" DEF_OBJECT "\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 201\r\n"
"X-Varnish: 734584055\r\n"
"Content-Length: 65\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"createdAt\" : 1443595884290,\n"
"  \"modifiedAt\" : 1443595884290\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_create_with_id(&kii, &bucket, OBJECT, "{}",
            "application/json");
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_patch)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"PATCH https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 09:23:02 GMT\r\n"
"ETag: \"3\"\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 200\r\n"
"X-Varnish: 1976833323\r\n"
"Content-Length: 136\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\"_owner\":\"" DEF_THING_ID "\",\"_created\":1443595884290,\"_id\":\"" DEF_OBJECT "\",\"_modified\":1444728170385,\"_version\":\"3\"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_patch(&kii, &bucket, OBJECT, "{}", NULL);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, replace_object)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"PUT https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 09:31:27 GMT\r\n"
"ETag: \"4\"\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 201\r\n"
"X-Varnish: 734584055\r\n"
"Content-Length: 65\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"createdAt\" : 1443595884290,\n"
"  \"modifiedAt\" : 1444728675797\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_replace(&kii, &bucket, OBJECT, "{}", NULL);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_delete)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"DELETE https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 09:50:41 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 204\r\n"
"X-KII-APPID: " DEF_APP_ID "\r\n"
"X-Varnish: 338159476\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_delete(&kii, &bucket, OBJECT);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_get)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"GET https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 09:36:11 GMT\r\n"
"ETag: \"4\"\r\n"
"Last-Modified: Tue, 13 Oct 2015 09:31:15 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 200\r\n"
"X-Varnish: 757607985\r\n"
"Content-Length: 136\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\"_owner\":\"" DEF_THING_ID "\",\"_created\":1443595884290,\"_id\":\"" DEF_OBJECT "\",\"_modified\":1444728675797,\"_version\":\"4\"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_get(&kii, &bucket, OBJECT);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_upload_body_at_once)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"PUT https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectBodyUpdateResponse+json;charset=UTF-8\r\n"
"Date: Thu, 03 Dec 2015 05:33:09 GMT\r\n"
"ETag: \"awkzybqxot3i12w8m7belhj2a\"\r\n"
"Last-Modified: Thu, 03 Dec 2015 05:31:31 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 200\r\n"
"Content-Length: 34\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"modifiedAt\" : 1449120691863\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_upload_body_at_once(&kii, &bucket, OBJECT,
            "application/json", "{}", 2);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_init_upload_body)
{
    int err;
    char buffer[4096];
    char upload_id[KII_UPLOADID_SIZE + 1];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body/uploads HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/vnd.kii.startobjectbodyuploadrequest+json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"accept:application/vnd.kii.startobjectbodyuploadresponse+json\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectBodyUpdateResponse+json;charset=UTF-8\r\n"
"Date: Thu, 03 Dec 2015 06:13:56 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 200\r\n"
"Content-Length: 67\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"uploadID\" : \"" DEF_UPLOAD_ID "\"\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_init_upload_body(&kii, &bucket, OBJECT, upload_id);
    ASSERT_EQ(0, err);

    ASSERT_STREQ(UPLOAD_ID, upload_id);
}

TEST(kiiTest, object_upload_body)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    kii_chunk_data_t chunk;
    const char* send_body =
"PUT https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body/uploads/" DEF_UPLOAD_ID "/data HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"Accept: application/json, application/*+json\r\n"
"Content-Range: bytes=0-1/2\r\n"
"content-length:2\r\n"
"\r\n"
"{}";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"ClientID: " DEF_APP_ID "\r\n"
"Date: Thu, 03 Dec 2015 06:17:34 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;
    char chunk_body[] = "{}";
    char body_content_type[] = "application/json";

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    chunk.chunk = chunk_body;
    chunk.body_content_type = body_content_type;
    chunk.length = 2;
    chunk.position = 0;
    chunk.total_length = 2;
    err = kii_object_upload_body(&kii, &bucket, OBJECT, UPLOAD_ID, &chunk);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_commit_upload)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body/uploads/" DEF_UPLOAD_ID "/status/committed HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"ClientID: " DEF_APP_ID "\r\n"
"Date: Thu, 03 Dec 2015 06:13:58 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_object_commit_upload(&kii, &bucket, OBJECT, UPLOAD_ID, 1);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, object_download_body_at_once)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    unsigned int length;
    const char* send_body =
"GET https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"Accept: */*\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/json\r\n"
"Date: Thu, 03 Dec 2015 09:32:05 GMT\r\n"
"ETag: \"85f5ic2n6fdg89mrh29i0wio84\"\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 200\r\n"
"Content-Length: 2\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    length = 0;
    err = kii_object_download_body_at_once(&kii, &bucket, OBJECT, &length);
    ASSERT_EQ(0, err);

    ASSERT_EQ(2, length);
    ASSERT_STREQ("{}", kii.kii_core.response_body);
}

TEST(kiiTest, object_download_body)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    unsigned int actual_length;
    unsigned int total_length;
    const char* send_body =
"GET https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/objects/" DEF_OBJECT "/body HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"Range: bytes=0-1\r\n"
"Accept: */*\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 206 Partial Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Range: bytes 0-1/2\r\n"
"Content-Type: application/json\r\n"
"Date: Thu, 03 Dec 2015 09:54:31 GMT\r\n"
"ETag: \"85f5ic2n6fdg89mrh29i0wio8\"\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 206\r\n"
"Content-Length: 2\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    actual_length = 0;
    total_length = 0;
    err = kii_object_download_body(&kii, &bucket, OBJECT, 0, 2, &actual_length,
            &total_length);
    ASSERT_EQ(0, err);

    ASSERT_EQ(2, actual_length);
    ASSERT_EQ(2, total_length);
    ASSERT_STREQ("{}", kii.kii_core.response_body);
}

TEST(kiiTest, push_subscribe_bucket)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/filters/all/push/subscriptions/things HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 10:34:39 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 204\r\n"
"X-KII-APPID: " DEF_APP_ID "\r\n"
"X-Varnish: 338282590\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_push_subscribe_bucket(&kii, &bucket);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, push_unsubscribe_bucket)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_bucket_t bucket;
    const char* send_body =
"DELETE https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/buckets/" DEF_BUCKET "/filters/all/push/subscriptions/things/" DEF_THING_ID " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Tue, 13 Oct 2015 10:35:18 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 204\r\n"
"X-KII-APPID: " DEF_APP_ID "\r\n"
"X-Varnish: 757764423\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initBucket(&bucket);

    err = kii_push_unsubscribe_bucket(&kii, &bucket);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, push_subscribe_topic)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_topic_t topic;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/topics/" DEF_TOPIC "/push/subscriptions/things HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Date: Fri, 13 Nov 2015 10:59:08 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initTopic(&topic);

    err = kii_push_subscribe_topic(&kii, &topic);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, push_unsubscribe_topic)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_topic_t topic;
    const char* send_body =
"DELETE https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/topics/" DEF_TOPIC "/push/subscriptions/things/" DEF_THING_ID " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/vnd.kii.ObjectUpdateResponse+json;charset=UTF-8\r\n"
"Date: Fri, 13 Nov 2015 10:58:56 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initTopic(&topic);

    err = kii_push_unsubscribe_topic(&kii, &topic);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, push_create_topic)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_topic_t topic;
    const char* send_body =
"PUT https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/topics/" DEF_TOPIC " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Date: Fri, 13 Nov 2015 09:59:15 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initTopic(&topic);

    err = kii_push_create_topic(&kii, &topic);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, push_delete_topic)
{
    int err;
    char buffer[4096];
    kii_t kii;
    kii_topic_t topic;
    const char* send_body =
"DELETE https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/things/" DEF_THING_ID "/topics/" DEF_TOPIC " HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 204 No Content\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Date: Fri, 13 Nov 2015 09:56:44 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 204\r\n"
"Connection: keep-alive\r\n"
"\r\n";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    initTopic(&topic);

    err = kii_push_delete_topic(&kii, &topic);
    ASSERT_EQ(0, err);
}

TEST(kiiTest, server_code_execute)
{
    int err;
    char buffer[4096];
    kii_t kii;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/apps/" DEF_APP_ID "/server-code/versions/current/test_code HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
"authorization:bearer " DEF_ACCESS_TOKEN "\r\n"
"\r\n";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Date: Fri, 13 Nov 2015 09:59:15 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"X-HTTP-Status-Code: 200\r\n"
"Content-Type: application/json;charset=UTF-8\r\n"
"Content-Length: 29\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\"returnValue\":\"returnValue\"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    err = kii_server_code_execute(&kii, "test_code", NULL);
    ASSERT_EQ(0, err);

    ASSERT_STREQ("{\"returnValue\":\"returnValue\"}",
            kii.kii_core.response_body);
}

TEST(kiiTest, api_call)
{
    int err;
    char buffer[4096];
    kii_t kii;
    const char* send_body =
"POST https://" DEF_APP_HOST "/api/oauth2/token HTTP/1.0\r\n"
"host:" DEF_APP_HOST "\r\n"
"x-kii-appid:" DEF_APP_ID "\r\n"
"x-kii-appkey:" DEF_APP_KEY "\r\n"
"x-kii-sdk:" DEF_SDK_VERSION "\r\n"
"content-type:application/json\r\n"
DEF_DUMMY_HEADER "\r\n"
"content-length:59\r\n"
"\r\n"
"{\"username\":\"VENDOR_THING_ID:1426830900\",\"password\":\"1234\"}";
    const char* recv_body =
"HTTP/1.1 200 OK\r\n"
"Accept-Ranges: bytes\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Expose-Headers: Content-Type, Authorization, Content-Length, X-Requested-With, ETag, X-Step-Count\r\n"
"Age: 0\r\n"
"Cache-Control: max-age=0, no-cache, no-store\r\n"
"Content-Type: application/json;charset=UTF-8\r\n"
"Date: Fri, 25 Sep 2015 11:07:16 GMT\r\n"
"Server: nginx/1.2.3\r\n"
"Via: 1.1 varnish\r\n"
"X-HTTP-Status-Code: 200\r\n"
"X-Varnish: 726929556\r\n"
"Content-Length: 176\r\n"
"Connection: keep-alive\r\n"
"\r\n"
"{\n"
"  \"id\" : \"" DEF_THING_ID "\",\n"
"  \"access_token\" : \"" DEF_ACCESS_TOKEN "\",\n"
"  \"expires_in\" : 2147483639,\n"
"  \"token_type\" : \"Bearer\"\n"
"}";
    test_context_t ctx;

    ctx.send_body = send_body;
    ctx.recv_body = recv_body;

    init(&kii, buffer, 4096, &ctx);

    strcpy(kii.kii_core.author.author_id, "");
    strcpy(kii.kii_core.author.access_token, "");
    kii.kii_core.response_code = 0;
    kii.kii_core.response_body = NULL;

    err = kii_api_call_start(&kii, "POST", "api/oauth2/token",
            "application/json", KII_FALSE);
    ASSERT_EQ(0, err);
    err = kii_api_call_append_body(&kii,
            "{\"username\":\"VENDOR_THING_ID:1426830900\",", 41);
    ASSERT_EQ(0, err);
    err = kii_api_call_append_body(&kii, "\"password\":\"1234\"}", 18);
    ASSERT_EQ(0, err);
    err = kii_api_call_append_header(&kii, DEF_DUMMY_KEY, DEF_DUMMY_VALUE);
    ASSERT_EQ(0, err);
    err = kii_api_call_run(&kii);
    ASSERT_EQ(0, err);

    ASSERT_EQ(200, kii.kii_core.response_code);

    ASSERT_TRUE(kii.kii_core.response_body != NULL);
    ASSERT_STREQ(
            "{\n"
            "  \"id\" : \"" DEF_THING_ID "\",\n"
            "  \"access_token\" : \"" DEF_ACCESS_TOKEN "\",\n"
            "  \"expires_in\" : 2147483639,\n"
            "  \"token_type\" : \"Bearer\"\n"
            "}",
            kii.kii_core.response_body);
}
