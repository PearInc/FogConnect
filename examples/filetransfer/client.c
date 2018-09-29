
#include <glib.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"
#include "ser.h"


struct file_data {
    size_t size;
    GString* file_content;
};

struct file_data* file_data_new()
{
    struct file_data* f = g_new(struct file_data, 1);
    f->size = -1;
    f->file_content = g_string_new(NULL);
    return f;
}

void connecting_cb(void* arg)
{
    printf("connection_cb\n");
    char* msg = g_strdup("hello");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    pr_send_peer(ud->pr_connect, msg, strlen(msg));
    struct file_data* f = file_data_new();
    ud->context = (void*)f;
    free(msg);
}

void msg_cb(void* arg)
{
    printf("msg cb\n");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
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
        printf("get the msg length is %ld\n", length);
        char* msg = g_new(char, length);
        evbuffer_remove(ud->buff, msg, length);
        g_string_append_len(f->file_content, msg, length);
        printf("total rece length %ld\n", f->file_content->len);
        free(msg);
        if (f->file_content->len == f->size) {
            // write it to the file
            FILE* file = fopen("download_file", "w");
            if (file != NULL) {
                if(fwrite(f->file_content->str, f->file_content->len, 1, file)!=1){
                    printf("file write error\n");
                } else {
                    printf("suscessful write the file\n");
                }
                fclose(file);
            }
            exit(0);
        }
    }
}

void close_cb(void* arg)
{
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    struct file_data* f = (struct file_data*)ud->context;
    if (f->size == f->file_content->len) {
        g_string_free(f->file_content, true);
        g_free(f);
    }
}

int main()
{
    SETUP("1e:34:a1:44:2c:2c", connecting_cb, msg_cb, close_cb);
    pear_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0;i<100;i++) {
        sleep(2);
    }
    pear_fog_connect_release();

    return 0;
}

