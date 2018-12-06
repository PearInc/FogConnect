
#include <unistd.h>

#include "pubsub.h"

// "test_topic", "utc_time"


static pubsub_client* client = NULL;


static void subscription(const char* tp, const char* content)
{
    printf("%s: %s\n", tp, content);
}

static void connection()
{
    pubsubclient_subscribe(client, "utc_time", subscription);
}

static void connecting_cb(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    client->arg = arg;
    connection();
}

static void msg_cb(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    pubsubclient_on_message(client);
}

static void close_cb(void* arg)
{
    g_free(client);
    client = NULL;
}


int main()
{
    fog_set_up("1e:34:a1:44:2c:3c", connecting_cb, msg_cb, close_cb);
    client = g_new(pubsub_client, 1);

    fog_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0;i<100;i++) {
        sleep(2);
    }

    fog_connect_release();

    return 0;
}

