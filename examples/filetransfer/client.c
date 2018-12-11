#include <event2/buffer.h>
#include <event2/event.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>

#include "fogconnect.h"
#include "pr_fog_connect.h"
#include "ser.h"

const char* g_file = NULL;

pthread_mutex_t mutex;

time_t start;
time_t end;
int64_t bytes_read;

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

void on_connect(void* arg)
{
    printf("connection_cb\n");
    char* msg = strdup("hello\r\n");
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    fog_send_data(ud->pr_connect, msg, strlen(msg));
    free(msg);
    struct file_data* f = file_data_new();
    ud->context = (void*)f;
    start = time(NULL);
}

void on_close(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
    struct file_data* f = (struct file_data*)ud->context;
    fclose(f->fp);
    bytes_read = f->length;
    free(f);
    end = time(NULL);
    pthread_mutex_unlock(&mutex);
}


void on_receive(void* arg)
{
    fog_connectiion_info* ud = (fog_connectiion_info*)arg;
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
        // printf("get the file %ld of total %ld\n", f->length, f->size);

        char* msg = (char*)malloc(length);
        evbuffer_remove(ud->buff, msg, length);
        int r = fwrite(msg, length, 1, f->fp); 
        free(msg);
        if (r != 1) {
            printf("file write error\n");
            on_close(arg);
        }
        if (f->length == f->size) {
            on_close(arg);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("arg is too less");
        return -1;
    }
    g_file = argv[1];

    fog_setup("1e:34:a1:44:2c:2c");
    fog_connect_peer("1e:34:a1:44:2c:1c", FOG_TRANSPORT_PROTOCOL_KCP, on_connect, on_receive, on_close);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    double seconds = (double)(end-start);
    printf("seconds: %f\n", seconds);
    double speed = (double)bytes_read/(1024*1024*seconds);
    printf("\nthe speed is %f mb/s\n", speed);
    fog_exit();
    return 0;
}

