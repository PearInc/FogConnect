#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "fogconnect.h"
#include "fog_connect.h"

char CRLF[2] = "\r\n";

void on_connect(void *arg) {
}

void on_recv(void *arg) {
    fc_info *ud = (fc_info *)arg;
    size_t len = 0;
    char *msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        fc_send_data(ud->pr_connect, msg, len);
        fc_send_data(ud->pr_connect, CRLF, sizeof(CRLF));
        printf("sending: %s\n", msg);
        free(msg);
    }
}

void on_close(void *arg) {
}

int main() {
    fc_setup("**:**:**:**:**:1c");
    fc_service_set_callback(on_connect, on_recv, on_close);
    getchar();
    fc_exit();
    return 0;
}


