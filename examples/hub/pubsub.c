#include "pubsub.h"

bool pubsubclient_subscribe(pubsub_client* client, const char* tp, subscribe_cb_p subscribe_cb)
{
    char* message = g_strdup_printf("sub %s\r\n", tp);
    SEND_MESSAGE(client, message);
    client->subscribe_cb = subscribe_cb;
}

void pubsubclient_unsubscribe(pubsub_client* client, const char* tp)
{
    char* message = g_strdup_printf("unsub %s\r\n", tp);
    SEND_MESSAGE(client, message);
}

bool pubsubclient_publish(pubsub_client* client, const char* tp, const char* content)
{
    char* message = g_strdup_printf("pub %s\r\n%s\r\n", tp, content);
    SEND_MESSAGE(client, message);
}


void pubsubclient_on_message(pubsub_client* client)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)client->arg;
    enum ParseResult result = kSuccess;
    while (result == kSuccess) {
        GString* cmd = g_string_new(NULL);
        GString* tp = g_string_new(NULL);
        GString* content = g_string_new(NULL);
        result = parseMessage(ud->buff, cmd, tp, content);
        if (result == kSuccess) {
            if (strcmp(cmd->str, "pub") == 0 && client->subscribe_cb != NULL) {
                client->subscribe_cb(tp->str, content->str);
            }
        } else if (result == kError) {
            // shutdown
        }
    }
}


