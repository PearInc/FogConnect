#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"

void on_connect(void* arg)
{
    printf("conn_cb\n");
}

void on_receive(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("receiving %s\n", msg);
        int len = strlen(msg)+2;
        char* return_msg = (char*)malloc(len+1);
        sprintf(return_msg, "%s\r\n", msg);
        fog_send_data(ud->pr_connect, return_msg, len);
        printf("sending: %s\n", msg);
        free(msg);
        free(return_msg);
    }
}

void on_close(void* arg)
{
}

int main()
{
    fog_setup("1e:34:a1:44:2c:1c");

    fog_service_set_callback(on_connect, on_receive, on_close);
    
    getchar();

    fog_exit();
    return 0;
}


