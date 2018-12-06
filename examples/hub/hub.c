
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <unistd.h>
#include <stdbool.h>

#include "codec.h"
#include "fogconnect.h"
#include "pr_fog_connect.h"


typedef struct {
    GString* topic;
    GString* content;
    GPtrArray* audidences;
} topic;


static topic* topic_new(const char* tp)
{
    topic* t = g_new(topic, 1);
    t->topic = g_string_new(tp);
    t->content = g_string_new(NULL);
    t->audidences = g_ptr_array_new();
    return t;
}

static void topic_free(void* tp)
{
    if (tp == NULL) return;
    topic* t = (topic*)tp;
    g_string_free(t->content, true);
    g_string_free(t->topic, true);
    g_ptr_array_unref(t->audidences);
    g_free(t);
}

static GString* topic_make_message(topic* t)
{
    GString* ret = g_string_new(NULL);
    g_string_printf(ret, "pub %s\r\n%s\r\n",t->topic->str, t->content->str);
    return ret;
}

static void topic_add(topic* t, void* pr_conn)
{
    g_ptr_array_add(t->audidences, pr_conn);
    GString* msg = topic_make_message(t);
    pr_send_peer(pr_conn, msg->str, msg->len);
    g_string_free(msg, true);
}


static void topic_remove(topic* t, void* pr_conn)
{
    for (int i = 0; i < t->audidences->len; i++) {
        void* ptr = g_ptr_array_index(t->audidences, i);
        if (ptr == pr_conn) {
            g_ptr_array_remove_index_fast(t->audidences, i);
            return;
        }
    }
}


static void topic_publish(topic* t, const char* content)
{
    printf("publish %s %s\n", t->topic->str, content);
    g_string_printf(t->content, "%s", content);
    GString* msg = topic_make_message(t);
    for (int i=0; i < t->audidences->len; i++) { 
        void* pr_connect = g_ptr_array_index(t->audidences, i);
        pr_send_peer(pr_connect, msg->str, msg->len);
    }
}

typedef struct {
    GHashTable* topics;
} pubsubserver;

pubsubserver* server = NULL;


static void pubsubserver_init()
{
    if (server == NULL) {
        server = g_new(pubsubserver, 1);
        server->topics = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, topic_free);
    }
}

static topic* get_topic(pubsubserver* s, const char* tp)
{
    gpointer value = NULL;
    bool ok = g_hash_table_lookup_extended(s->topics, tp, NULL, &value);
    topic* ret = (topic*)value;
    if (!ok) {
        char* t = g_strdup(tp);
        topic* to = topic_new(tp);
        g_hash_table_insert(s->topics, t, to);
        ret = to;
    }
    return ret;
}


static void do_subscribe(pubsubserver* s, void* arg, const char* tp)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    GHashTable* connection_subscription = (GHashTable*)ud->context;
    char* tpc = g_strdup(tp);
    g_hash_table_add(connection_subscription, tpc);
    topic* t = get_topic(s, tp);
    topic_add(t, ud->on_connect);
}


static void do_unsubscribe(pubsubserver* s, void* arg, const char* tp)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    GHashTable* connection_subscription = (GHashTable*)ud->context;
    g_hash_table_remove(connection_subscription, tp);
}


static void do_publish(pubsubserver* s, const char* source, const char* tp, const char* content)
{
    topic* t = get_topic(s, tp);
    topic_publish(t, content);
}

static void time_publish(pubsubserver* s) 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t seconds = tv.tv_sec;
    char timebuff[65];
    memset(timebuff, 0, 65);
    
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    snprintf(timebuff, 65, "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year+1900,tm_time.tm_mon+1,tm_time.tm_mday,
            tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
    
    do_publish(s, "internal", "utc_time", timebuff);
}


static void connecting_cb(void* arg)
{
    printf("conn_cb\n");
    GHashTable* connection_subscription = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    ud->context = connection_subscription;
}


static void msg_cb(void* arg)
{
    printf("msg cb\n");
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    enum ParseResult result = kSuccess;
    while(result == kSuccess) {
        GString* cmd = g_string_new(NULL);
        GString* tp = g_string_new(NULL);
        GString* content = g_string_new(NULL);
        result = parseMessage(ud->buff, cmd, tp, content);
        if (result == kSuccess) {
            if (strcmp(cmd->str, "pub") == 0) {
                do_publish(server, "client", tp->str, content->str);
            } else if (strcmp(cmd->str, "sub") == 0) {
                do_subscribe(server, arg, tp->str);
            } else if (strcmp(cmd->str, "unsub") == 0) {
                do_unsubscribe(server, arg, tp->str);
            } else {
                result = kError;
                // shutdown
            }
        } else if (result == kError) {
            // shutdown
        }
    }
}

static void close_cb(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    GHashTable* connection_subscription = (GHashTable*)ud->context;

    GHashTableIter iter;
    gpointer key;
    g_hash_table_iter_init(&iter, connection_subscription);
    while(g_hash_table_iter_next(&iter, &key, NULL)){
        char* tp = (char*)key;
        do_unsubscribe(server, arg, tp);
    }

    g_hash_table_unref(connection_subscription);
}

static void timeout_cb(int fd, short events, void* arg)
{
    time_publish(server);
}

int main()
{
    evthread_use_pthreads();
    struct event_base* base = event_base_new();

    pubsubserver_init();
    fog_set_up("1e:34:a1:44:2c:1c", connecting_cb, msg_cb, close_cb);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    struct event* timer = event_new(base, -1, EV_PERSIST | EV_TIMEOUT, timeout_cb, event_self_cbarg());
    event_add(timer, &tv);
    event_base_dispatch(base);

    fog_connect_release();
    return 0;
}






