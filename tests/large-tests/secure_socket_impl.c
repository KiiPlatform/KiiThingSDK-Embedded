#include "secure_socket_impl.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <openssl/crypto.h>
#include <openssl/err.h>

/* Suppress warnings, because OpenSSL was deprecated in Mac. */
#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

kch_sock_code_t
    s_connect_cb(void* sock_ctx, const char* host,
            unsigned int port)
{
    int sock, ret;
    struct hostent *servhost;
    struct sockaddr_in server;
    SSL *ssl = NULL;
    SSL_CTX *ssl_ctx = NULL;

    servhost = gethostbyname(host);
    if (servhost == NULL) {
        printf("failed to get host.\n");
        return KIISOCK_FAIL;
    }
    memset(&server, 0x00, sizeof(server));
    server.sin_family = AF_INET;
    /* More secure. */
    memcpy(&(server.sin_addr), servhost->h_addr, servhost->h_length);

    /* Get Port number */
    server.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("failed to init socket.\n");
        return KIISOCK_FAIL;
    }

    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) == -1 ){
        printf("failed to connect socket.\n");
        return KIISOCK_FAIL;
    }

    SSL_library_init();
    const SSL_METHOD *method =
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
        TLSv1_2_client_method();
#else
        TLS_client_method();
#endif
    ssl_ctx = SSL_CTX_new(method);
    if (ssl_ctx == NULL){
        printf("failed to init ssl context.\n");
        return KIISOCK_FAIL;
    }

    ssl = SSL_new(ssl_ctx);
    if (ssl == NULL){
        printf("failed to init ssl.\n");
        return KIISOCK_FAIL;
    }

    ret = SSL_set_fd(ssl, sock);
    if (ret == 0){
        printf("failed to set fd.\n");
        return KIISOCK_FAIL;
    }

    ret = SSL_connect(ssl);
    if (ret != 1) {
        int sslErr= SSL_get_error(ssl, ret);
        char sslErrStr[120];
        ERR_error_string_n(sslErr, sslErrStr, 120);
        printf("failed to connect: %s\n", sslErrStr);
        return KIISOCK_FAIL;
    }

    ssl_context_t* ctx = (ssl_context_t*)sock_ctx;
    ctx->socket = sock;
    ctx->ssl = ssl;
    ctx->ssl_ctx = ssl_ctx;
    return KIISOCK_OK;
}

kch_sock_code_t
    s_send_cb(void* socket_context,
            const char* buffer,
            size_t length)
{
    ssl_context_t* ctx = (ssl_context_t*)socket_context;
    int ret = SSL_write(ctx->ssl, buffer, length);
    if (ret > 0) {
        return KIISOCK_OK;
    } else {
        printf("failed to send\n");
        return KIISOCK_FAIL;
    }
}

kch_sock_code_t
    s_recv_cb(void* socket_context,
            char* buffer,
            size_t length_to_read,
            size_t* out_actual_length)
{
    ssl_context_t* ctx = (ssl_context_t*)socket_context;
    int ret = SSL_read(ctx->ssl, buffer, length_to_read);
    if (ret > 0) {
        *out_actual_length = ret;
        return KIISOCK_OK;
    } else {
        printf("failed to receive:\n");
        /* TOOD: could be 0 on success? */
        *out_actual_length = 0;
        return KIISOCK_FAIL;
    }
}

kch_sock_code_t
    s_close_cb(void* socket_context)
{
    ssl_context_t* ctx = (ssl_context_t*)socket_context;
    int ret = SSL_shutdown(ctx->ssl);
    if (ret != 1) {
        int sslErr = SSL_get_error(ctx->ssl, ret);
        if (sslErr == SSL_ERROR_SYSCALL) {
            /* This is OK.*/
            /* See https://www.openssl.org/docs/ssl/SSL_shutdown.html */
            ret = 1;
        } else {
            char sslErrStr[120];
            ERR_error_string_n(sslErr, sslErrStr, 120);
            printf("failed to shutdown: %s\n", sslErrStr);
        }
    }
    close(ctx->socket);
    SSL_free(ctx->ssl);
    SSL_CTX_free(ctx->ssl_ctx);
    if (ret != 1) {
        printf("failed to close:\n");
        return KIISOCK_FAIL;
    }
    return KIISOCK_OK;
}

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif
