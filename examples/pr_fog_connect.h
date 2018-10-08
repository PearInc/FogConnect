
#ifndef PR_FOG_CONNECT_H
#define PR_FOG_CONNECT_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <event2/buffer.h>
#include <event2/event.h>

#include "fogconnect.h"

#define SIGNAL_SERVER_URL "47.52.153.245"
#define TRANSPORT_PROTOCOL PR_TRANSPORT_PROTOCOL_KCP

#define SETUP(id, concb, msgcb, closecb)  \
do {    \
    pear_fog_connect_init(id);\
    pear_fogconnect_set_callback(pear_callbacks, concb, msgcb, closecb);\
    pear_signal_init();\
} while(0)


typedef void (*pear_connecting_callback_cb_p)(void*, short, void*);

typedef void (*pear_connecting_cb_p)(void* arg);

typedef void (*pear_msg_callback_cb_p)(void* arg);

typedef void (*pear_close_cb_p)(void* arg);


typedef struct pr_usr_data_s {
    void* pr_connect;

    pear_connecting_cb_p conncb;
    pear_msg_callback_cb_p msgcb;
    pear_close_cb_p closecb;

    void* context;
    struct evbuffer* buff;
} pr_usr_data_t;

pr_usr_data_t* pear_usr_data_new(pear_connecting_cb_p ccb, pear_msg_callback_cb_p mcb, pear_close_cb_p clcb);

void pear_usr_data_free(void *arg);

//------------------------------------------------------


int pear_fog_connect_init(const char* server_id);

void pear_connect_release();

void pear_signal_init();

void pear_callbacks(void* pr_connect, short events, void* arg);

// for the server part
void pear_fogconnect_set_callback(pear_connecting_callback_cb_p cb, pear_connecting_cb_p ccb, pear_msg_callback_cb_p msb, pear_close_cb_p closecb);

// for the client part
int pear_connect_peer(const char* id);

#endif // !PR_FOG_CONNECT_H