#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"

char CLRF[2] = "\r\n";

void on_connect(void *arg) {
}

void on_receive(void *arg) {
    fog_connection_info *ud = (fog_connection_info *)arg;
    size_t len = 0;
    char *msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        fog_send_data(ud->pr_connect, msg, len);
        fog_send_data(ud->pr_connect, CLRF, sizeof(CLRF));
        printf("sending: %s\n", msg);
        free(msg);
    }
}

void on_close(void *arg) {
}

int main() {
    fog_setup("1e:34:a1:44:2c:1c");
    fog_service_set_callback(on_connect, on_receive, on_close);
    getchar();
    fog_exit();
    return 0;
}


