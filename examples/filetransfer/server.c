#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"
#include "ser.h"

const char* g_file = NULL;

void connecting_cb(void* arg)
{
    printf("conn_cb\n");
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
}

void msg_cb(void* arg)
{
    printf("msg_cb and send the file %s\n", g_file);
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;

    FILE* fp;
    char buf[8*1024];
    if ((fp=fopen(g_file, "r")) == NULL) return;

    fseek(fp, 0L, SEEK_END);
    uint64_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    memset(buf, 0, 8);
    ser_writedata64(size, buf);
    pr_send_peer(ud->pr_connect, buf, 8);

    size_t n;
    while((n=fread(buf, 1, sizeof(buf), fp)) != 0) {
        pr_send_peer(ud->pr_connect, buf, n);
    }
}

void close_cb(void* arg)
{
    
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        g_file = argv[1];
        pear_set_up("1e:34:a1:44:2c:1c", connecting_cb, msg_cb, close_cb);

        for (int i = 0; i < 100; i++) {
            sleep(2);
        }
        pear_connect_release();
    } else {
        printf("arg is too less");
    }
    return 0;
}



