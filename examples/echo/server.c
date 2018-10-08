#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"

void connecting_cb(void* arg)
{
    printf("conn_cb\n");
}

void msg_cb(void* arg)
{
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s from the peer\n", msg);
        int len = strlen(msg)+2;
        char* return_msg = (char*)malloc(len+1);
        sprintf(return_msg, "%s\r\n", msg);
        pr_send_peer(ud->pr_connect, return_msg, len);
        printf("sending the msg %s back to the peer\n", msg);
        free(msg);
        free(return_msg);
    }
}

void close_cb(void* arg)
{
}

int main()
{
    SETUP("1e:34:a1:44:2c:1c", connecting_cb, msg_cb, close_cb);

    for (int i = 0; i < 100; i++) {
        sleep(2);
    }
    pear_connect_release();

    return 0;
}


