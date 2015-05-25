#include "kii_socket_impl.h"
#include <netdb.h>
#include <string.h>
#include <stdio.h>

kii_socket_code_t
    connect_cb(kii_socket_context_t* socket_context, const char* host,
            unsigned int port)
{
    int sock;
    struct hostent *servhost;
    struct sockaddr_in server;
    
    servhost = gethostbyname(host);
    if (servhost == NULL) {
        return KII_SOCKETC_FAIL;
    }
    memset(&server, 0x00, sizeof(server));
    server.sin_family = AF_INET;
    memcpy(&(server.sin_addr), servhost->h_addr, servhost->h_length);

    server.sin_port = htons(port);
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        return KII_SOCKETC_FAIL;
    }

    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) == -1 ){
        return KII_SOCKETC_FAIL;
    }
	
    socket_context->socket = sock;
    return KII_SOCKETC_OK;
}

kii_socket_code_t
    send_cb(kii_socket_context_t* socket_context,
            const char* buffer,
            size_t length)
{
    int ret;
    int sock;

    sock = socket_context->socket;
    ret = send(sock, buffer, length, 0);
    if (ret > 0) {
        return KII_SOCKETC_OK;
    } else {
        return KII_SOCKETC_FAIL;
    }
}

kii_socket_code_t
    recv_cb(kii_socket_context_t* socket_context,
            char* buffer,
            size_t length_to_read,
            size_t* out_actual_length)
{
    int ret;
    int sock;

    sock = socket_context->socket;
    ret = recv(sock, buffer, length_to_read, 0);
    if (ret > 0) {
     *out_actual_length = ret;
     return KII_SOCKETC_OK;
    } else {
        return KII_SOCKETC_FAIL;
    }
}

kii_socket_code_t
    close_cb(kii_socket_context_t* socket_context)
{
    int sock;
    sock = socket_context->socket;

    close(sock);
    socket_context->socket = -1;
    return KII_SOCKETC_OK;
}
/* vim:set ts=4 sts=4 sw=4 et fenc=UTF-8 ff=unix: */
