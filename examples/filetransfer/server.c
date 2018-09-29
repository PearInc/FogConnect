
#include <glib.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"
#include "ser.h"

const char* g_file = NULL;

GString* read_file(const char* filename)
{
    GString* content = g_string_new(NULL);
    FILE* fp = fopen(filename, "rb");
    if (fp) {
        const int kBufSize = 1024*1024;
        char iobuf[kBufSize];
        setbuffer(fp, iobuf, sizeof(iobuf));

        char buf[kBufSize];
        char string_buf[kBufSize];
        size_t nread = 0;
        while((nread = fread(buf, 1, sizeof(buf), fp)) > 0) {
            g_string_append_len(content, buf, nread);
        }
        fclose(fp);
    }
    return content;
}

void connecting_cb(void* arg)
{
    printf("conn_cb\n");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
}

void msg_cb(void* arg)
{
    printf("msg_cb and send the file %s\n", g_file);
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;


    GString* file_content = read_file(g_file);
    
    char buf[8];
    memset(buf, 0, 8);
    ser_writedata64(file_content->len, buf);

    printf("send the msg length is %ld", file_content->len);
    pr_send_peer(ud->pr_connect, buf, 8);
    pr_send_peer(ud->pr_connect, file_content->str, file_content->len);
    g_string_free(file_content, true);
}

void close_cb(void* arg)
{
    
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        g_file = argv[1];
        SETUP("1e:34:a1:44:2c:1c", connecting_cb, msg_cb, close_cb);

        for (int i = 0; i < 100; i++) {
            sleep(2);
        }
        pear_fog_connect_release();
    } else {
        printf("arg is too less");
    }
    return 0;
}



