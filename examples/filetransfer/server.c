#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include "fogconnect.h"
#include "fog_connect.h"
#include "ser.h"

const char *g_file = NULL;

void on_connect(void *arg) {
    printf("conn_cb\n");
    fc_info *ud = (fc_info *)arg;
}

void on_recv(void *arg) {
    printf("msg_cb and send the file %s\n", g_file);
    fc_info *ud = (fc_info *)arg;

    FILE *fp;
    char buf[8 * 1024];
    if ((fp = fopen(g_file, "r")) == NULL) return;

    fseek(fp, 0L, SEEK_END);
    uint64_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    memset(buf, 0, 8);
    ser_writedata64(size, buf);
    fc_send(ud->pr_connect, buf, 8);

    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) != 0) {
        fc_send(ud->pr_connect, buf, n);
    }
    fclose(fp);
}

void on_close(void *arg) {

}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        g_file = argv[1];
        fc_setup("1e:34:a1:44:2c:1c");
        fc_set_callback(on_connect, on_recv, on_close);
        getchar();
        fc_exit();
    } else {
        printf("arg is too less");
    }
    return 0;
}



