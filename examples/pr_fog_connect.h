
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

#define TRANSPORT_PROTOCOL PR_TRANSPORT_PROTOCOL_UDP

typedef void (*pear_callback_p)(void*, short, void*);

typedef void (*pear_connecting_cb_p)(void* arg);

typedef void (*pear_message_callback_cb_p)(void* arg);

typedef void (*pear_close_cb_p)(void* arg);


typedef struct pear_usr_data_s {
    void* pr_connect;

    pear_connecting_cb_p conncb;
    pear_message_callback_cb_p msgcb;
    pear_close_cb_p closecb;

    void* context;
    struct evbuffer* buff;
} pear_usr_data_t;

pear_usr_data_t* pear_usr_data_new(pear_connecting_cb_p ccb, pear_message_callback_cb_p mcb, pear_close_cb_p clcb);

void pear_usr_data_free(void *arg);

//------------------------------------------------------
void pear_set_up(server_id_, connect_cb_, message_cb_, close_cb_);

void pear_connect_release();

// for the client part
int pear_connect_peer(const char* id);

#endif // !PR_FOG_CONNECT_H