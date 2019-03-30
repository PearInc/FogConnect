
#ifndef PR_FOG_CONNECT_H
#define PR_FOG_CONNECT_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <event2/buffer.h>
#include <event2/event.h>

#include "fogconnect.h"

#define SIGNAL_SERVER_URL "47.52.153.245"

/**
 * 
 * The protocols is:
 * enum transport_protocol
 * {
 *      // protocol names
 *      PR_TRANSPORT_PROTOCOL_RTC = 0,
 *      PR_TRANSPORT_PROTOCOL_UDP,
 *      PR_TRANSPORT_PROTOCOL_UTP,
 *      PR_TRANSPORT_PROTOCOL_KCP,
 *      PR_TRANSPORT_PROTOCOL_SCTP,
 *      PR_TRANSPORT_PROTOCOL_QUIC,
 * };
 */

#define TRANSPORT_PROTOCOL FOG_TRANSPORT_PROTOCOL_KCP

typedef void (*event_cb)(void*, short, void*);

typedef void (*connect_cb)(void* arg);

typedef void (*receive_cb)(void* arg);

typedef void (*close_cb)(void* arg);


typedef struct {
    void* pr_connect;

    connect_cb on_connect;
    receive_cb on_recv;
    close_cb on_close;

    void* context;
    struct evbuffer* buff;
} fog_connection_info;

void fog_set_callbacks(fog_connection_info* ud, connect_cb on_connect, receive_cb on_recv, close_cb on_close);

//------------------------------------------------------
void fog_setup(const char* server_id);

void fog_service_set_callback(connect_cb on_connect, receive_cb on_recv, close_cb on_close);

void fog_exit();

// for the client part

int fog_connect_peer(const char* id, int protocol, connect_cb on_connect, receive_cb on_recv, close_cb on_close);

#endif // !PR_FOG_CONNECT_H