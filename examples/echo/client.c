
#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"

void on_connect(void *arg) {
    fog_connection_info *ud = (fog_connection_info *)arg;
    char *msg = strdup("hello\r\n");
    fog_send_data(ud->pr_connect, msg, strlen(msg));
    printf("sending: %s\n", msg);
    free(msg);
}

void on_receive(void *arg) {
    fog_connection_info *ud = (fog_connection_info *)arg;
    size_t len = 0;
    char *msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("receiving: %s\n", msg);
        free(msg);
        fog_disconnect(ud->pr_connect);
    }
}

void on_close(void *arg) {
}

int main() {
    fog_setup("**:**:**:**:**:2c");
    fog_connect_peer("**:**:**:**:**:1c", FOG_TRANSPORT_PROTOCOL_KCP, on_connect, on_receive, on_close);
    getchar();
    fog_exit();
    return 0;
}

