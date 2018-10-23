#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"
#include "ser.h"

const char* g_file = NULL;

struct file_data {
    FILE* fp;
    size_t size;
    uint64_t length;
};

struct file_data* file_data_new()
{
    struct file_data* f = (struct file_data*)malloc(sizeof(struct file_data));
    f->size = -1;
    f->length = 0;
    f->fp = fopen(g_file, "w");
    if (f->fp==NULL) exit(-1);

    return f;
}

void connecting_cb(void* arg)
{
    printf("connection_cb\n");
    char* msg = strdup("hello\r\n");
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    pr_send_peer(ud->pr_connect, msg, strlen(msg));
    free(msg);
    struct file_data* f = file_data_new();
    ud->context = (void*)f;
}

void close_cb(void* arg);

void msg_cb(void* arg)
{
    printf("msg cb\n");
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    struct file_data* f = (struct file_data*)ud->context;
    if (f->size == -1) {
        // get the file size
        char buff[8];
        memset(buff, 0, 8);
        evbuffer_remove(ud->buff, buff, 8);
        f->size = ser_readdata64(buff);
        printf("total file size is %ld\n", f->size);
    } else {
        size_t length = evbuffer_get_length(ud->buff);
        f->length += length;
        printf("get the msg length is %ld of total %ld\n", f->length, f->size);

        char* msg = (char*)malloc(length);
        evbuffer_remove(ud->buff, msg, length);
        int r = fwrite(msg, length, 1, f->fp); 
        free(msg);
        if (r != 1) {
            printf("file write error\n");
            close_cb(arg);
            exit(-1);
        }
        if (f->length == f->size) {
            close_cb(arg);
            exit(0);
        }
    }
}

void close_cb(void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    struct file_data* f = (struct file_data*)ud->context;
    fclose(f->fp);
    free(f);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("arg is too less");
        return -1;
    }
    g_file = argv[1];

    pear_set_up("1e:34:a1:44:2c:2c", connecting_cb, msg_cb, close_cb);
    pear_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0;i<100;i++) {
        sleep(2);
    }
    pear_connect_release();

    return 0;
}

