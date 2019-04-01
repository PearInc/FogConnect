
#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "fogconnect.h"
#include "fog_connect.h"

void on_connect(void *arg) {
    fc_info *ud = (fc_info *)arg;
    char *msg = strdup("hello\r\n");
    fc_send_data(ud->pr_connect, msg, strlen(msg));
    printf("sending: %s\n", msg);
    fc_free(msg);
}

void on_recv(void *arg) {
    fc_info *ud = (fc_info *)arg;
    size_t len = 0;
    char *msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("receiving: %s\n", msg);
        fc_free(msg);
        fc_disconnect(ud->pr_connect);
    }
}

void on_close(void *arg) {
}

int main() {
    fc_setup("**:**:**:**:**:2c");
    fc_connect_peer("**:**:**:**:**:1c", FOG_TRANSPORT_PROTOCOL_KCP, on_connect, on_recv, on_close);
    getchar();
    fc_exit();
    return 0;
}

