
#include <assert.h>
#include <malloc.h>
#include <memory.h>

#include "pr_fog_connect.h"

static void pear_callbacks(void* pr_connect, short events, void* arg);

static void pear_signal_init();

static int pear_connect_init(const char* server_id);

static void pear_connect_set_callback(pear_callback_p cb, pear_connecting_cb_p ccb, pear_message_callback_cb_p msb, pear_close_cb_p closecb);

static void* ctx = NULL;


static void pear_msg_cb(void* pr_connect, void* arg, void* buf, int size)
{
    pear_usr_data_t* user_data = (pear_usr_data_t*)arg;
    evbuffer_add(user_data->buff, (char*)buf, size);
    user_data->msgcb(user_data);
}


static void pear_close_cb(void* connect, void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    ud->closecb(ud);
    pear_usr_data_free(ud);
}


pear_usr_data_t* pear_usr_data_new(pear_connecting_cb_p ccb, pear_message_callback_cb_p mcb, pear_close_cb_p clcb)
{
    pear_usr_data_t* ret = (pear_usr_data_t*)malloc(sizeof(pear_usr_data_t));
    // g_new(pear_usr_data_t, 1);
    ret->pr_connect = NULL;
    ret->buff = evbuffer_new();
    ret->closecb = clcb;
    ret->conncb = ccb;
    ret->msgcb = mcb;
    return ret;
}

void pear_usr_data_free(void *arg)
{
    if (arg == NULL) return;
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    evbuffer_free(ud->buff);
    free(ud);
}

static int pear_connect_init(const char* server_id)
{
    if (ctx != NULL) return 0;
    ctx = pr_fogconnect_init();
    if (ctx == NULL) return -1;
    // set the id
    set_id(server_id);
    return 0;
}


void pear_connect_release()
{
    if (ctx != NULL) pr_fogconnect_release(ctx);
}


static void pear_signal_init()
{
    struct pr_signal_server* signal_info = malloc(sizeof(struct pr_signal_server));
    memset(signal_info, 0, sizeof(struct pr_signal_server));
    signal_info->ctx = ctx;
    signal_info->url = SIGNAL_SERVER_URL;
    signal_info->type = "/ws";
    signal_info->certificate = NULL;
    signal_info->privatekey = NULL;
    signal_info->port = 7600;
    pr_init_signal(signal_info);
}


static pear_connecting_cb_p g_ccb = NULL;
static pear_message_callback_cb_p g_msb = NULL;
static pear_close_cb_p g_closecb = NULL;

static void pear_callbacks(void* pr_connect, short events, void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    switch (events) {
        case PR_EVENT_CONNECTED: {
            if (ud != NULL) {
                ud->pr_connect = pr_connect;
            } else if (pr_connect_is_passive(pr_connect)) {
                // this fog node is connected by other fog node
                ud = pear_usr_data_new(g_ccb, g_msb, g_closecb);
                pr_connect_set_userdata(pr_connect, ud);
                ud->pr_connect = pr_connect;
            }
            // set the call back, for handle the msg between the fog nodes
            pr_event_setcb(pr_connect, pear_msg_cb, pear_close_cb);
            
            // connect callback
            ud->conncb(ud);
            break;
        }
        case PR_EVENT_EOF: 
        case PR_EVENT_ERROR: 
        case PR_EVENT_TIMEOUT: {
            printf("connect fail!\n");
            break;
        }
        default: break;
    }
}


void pear_set_up(server_id_, connect_cb_, message_cb_, close_cb_)
{
    pear_connect_init(server_id_);
    pear_connect_set_callback(pear_callbacks, connect_cb_, message_cb_, close_cb_);
    pear_signal_init();
}


static void pear_connect_set_callback(pear_callback_p cb, pear_connecting_cb_p ccb, pear_message_callback_cb_p msb, pear_close_cb_p closecb)
{
    g_ccb = ccb;
    g_msb = msb;
    g_closecb = closecb;
    pr_fogconnect_set_callback(ctx, cb);
}


int pear_connect_peer(const char* id)
{
    pear_usr_data_t* ud = pear_usr_data_new(g_ccb, g_msb, g_closecb);
    return pr_connect_peer(ctx, id, TRANSPORT_PROTOCOL, 1, pear_callbacks, ud);
}
