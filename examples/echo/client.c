
#include <glib.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"

void connecting_cb(void* arg)
{
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    char* msg = g_strdup("hello\r\n");
    pr_send_peer(ud->pr_connect, msg, strlen(msg));
    free(msg);
}

void msg_cb(void* arg)
{
    printf("msg cb\n");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s\n", msg);
        free(msg);
    }
}

void close_cb(void* arg)
{
    // call this function when the connection is closed
}

int main()
{
    SETUP("1e:34:a1:44:2c:2c", connecting_cb, msg_cb, close_cb);
    pear_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0;i<100;i++) {
        sleep(2);
    }
    pear_fog_connect_release();

    return 0;
}

