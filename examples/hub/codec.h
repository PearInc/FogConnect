#ifndef CODEC_H
#define CODEC_H

#include <event2/buffer.h>
#include <glib.h>

enum ParseResult{
    kError,
    kSuccess,
    kContinue
};

enum ParseResult parseMessage(struct evbuffer* buff, GString* cmd, GString* topic, GString* content);

#endif