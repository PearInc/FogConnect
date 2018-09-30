
#include <unistd.h>
#include "pubsub.h"

static pubsub_client* client = NULL;
GString* g_topic = NULL;
GString* g_content = NULL;

static void connection()
{
    pubsubclient_publish(client, g_topic->str, g_content->str);
    printf("publish %s  %s\n", g_topic->str, g_content->str);
}

static void connecting_cb(void* arg)
{
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    client->arg = arg;
    for (int i=0; i < 1000; i++) {
        g_string_printf(g_content, "conten: count %d", i);
        sleep(2);
        connection();
    }
}

static void msg_cb(void* arg)
{
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
}

static void close_cb(void* arg)
{
    g_free(client);
    client = NULL;
    g_string_free(g_topic, true);
    g_topic = NULL;
    g_string_free(g_content, true);
    g_content = NULL;
}


int main()
{
    SETUP("1e:34:a1:44:2c:2c", connecting_cb, msg_cb, close_cb);
    client = g_new(pubsub_client, 1);
    g_topic = g_string_new("test_topic");
    g_content = g_string_new(NULL);

    pear_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0; i < 100; i++) {
        sleep(2);
    }

    pear_fog_connect_release();

    return 0;
}
