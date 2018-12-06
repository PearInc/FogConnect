#ifndef PUBSUB_H
#define PUBSUB_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "codec.h"
#include "fogconnect.h"
#include "pr_fog_connect.h"


#define SEND_MESSAGE(client, message) \
do {    \
    fog_connectiion_info* ud = (fog_connectiion_info*)client->arg;    \
    pr_send_peer(ud->pr_connect, message, strlen(message)); \
    g_free(message); \
} while(0)

typedef void (*subscribe_cb_p)(const char* tp, const char* content);

typedef struct {
    void* arg;
    subscribe_cb_p subscribe_cb;
} pubsub_client;


bool pubsubclient_subscribe(pubsub_client* client, const char* tp, subscribe_cb_p subscribe_cb);

void pubsubclient_unsubscribe(pubsub_client* client, const char* tp);

bool pubsubclient_publish(pubsub_client* client, const char* tp, const char* content);

void pubsubclient_on_message(pubsub_client* client);

#endif