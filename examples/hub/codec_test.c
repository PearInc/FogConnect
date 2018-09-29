
#include "codec.h"
#include <string.h>
#include <event2/buffer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


int main()
{
    struct evbuffer* buff = evbuffer_new();
    const char* msg = "pub hello\r\nworld\r\n";
    evbuffer_add(buff, msg, strlen(msg));
    GString* cmd = g_string_new(NULL);
    GString* topic = g_string_new(NULL);
    GString* content = g_string_new(NULL);
    parseMessage(buff, cmd, topic, content);

    printf("cmd is: %s, size %ld\n", cmd->str, cmd->len);
    printf("topic is: %s, size %ld\n", topic->str, topic->len);
    printf("content is: %s, size %ld\n", content->str, content->len);
    printf("the length of the evbuff is %ld", evbuffer_get_length(buff));
    evbuffer_free(buff);
    g_string_free(cmd, true);
    g_string_free(topic, true);
    g_string_free(content, true);
    return 0;
}

