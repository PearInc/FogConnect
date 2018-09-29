
#include "codec.h"
#include <stdlib.h>
#include <stdio.h>

#define BUFFSIZE 1024

enum ParseResult parseMessage(struct evbuffer* buff, GString* cmd, GString* topic, GString* content)
{
    enum ParseResult result = kError;
    struct evbuffer_ptr crlf = evbuffer_search_eol(buff, NULL, NULL, EVBUFFER_EOL_CRLF);
    char c[BUFFSIZE];
    memset(c, 0, BUFFSIZE);
    if (crlf.pos != -1) {
        // set cmd
        struct evbuffer_ptr space = evbuffer_search(buff, " ", 1, NULL);
        if (crlf.pos != space.pos) {

            evbuffer_remove(buff, c, space.pos);
            g_string_printf(cmd, "%s", c);
            memset(c, 0, BUFFSIZE);

            // remove the " "        
            evbuffer_drain(buff, 1);

            // set topic
            evbuffer_remove(buff, c, crlf.pos - space.pos - 1);
            g_string_printf(topic, "%s", c);
            memset(c, 0, BUFFSIZE);

            // remove the "\r\n"
            evbuffer_drain(buff, 2);
            // set content
            if (strcmp(cmd->str, "pub") == 0) {
                crlf = evbuffer_search_eol(buff, NULL, NULL, EVBUFFER_EOL_CRLF);
                if (crlf.pos != -1) {
                    evbuffer_remove(buff, c, crlf.pos);
                    g_string_printf(content, "%s", c);
                    memset(c, 0, BUFFSIZE);
                    evbuffer_drain(buff, 2);
                    result = kSuccess;
                } else {
                    result = kContinue;
                }
            } else {
                evbuffer_readln(buff, NULL, EVBUFFER_EOL_CRLF);
                result = kSuccess;
            }
        } else {
            result = kError;
        }
    } else {
        result = kContinue;
    }
    return result;
}
