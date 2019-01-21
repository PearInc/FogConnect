LDFLAGS := -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
	-Wunused -fstrict-aliasing -Wextra -Wformat=2 -Winit-self -Winline \
	-Wpacked -Wpointer-arith -Wmissing-format-attribute -Wmissing-noreturn \
	-Wnested-externs -Wstrict-prototypes -Wunsafe-loop-optimizations \
	-Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter \
	-Wcast-align -Wformat-nonliteral -Wformat-security -Wswitch-default \
	-Wmissing-include-dirs -Wno-aggregate-return -Wunused-but-set-variable \
	-Warray-bounds -Wold-style-definition -Wsign-compare -Wlarger-than=65537


LDFLAGS += -Wno-undef  				# sofiasip is not -Wundef--safe
LDFLAGS += -Wno-redundant-decls  		# sofiasip also contains redundant declarations
LDFLAGS += -Wno-override-init  			# we need this for JANUS_PLUGIN_INIT and JANUS_TRANSPORT_INIT

CFLAGS += -g -O3 -fdata-sections -ffunction-sections -fno-stack-protector -std=gnu99 \
	-fPIC -rdynamic -I ../../../../include -I/usr/include/glib-2.0\
	-I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LIBS := -L./ -lpthread  -lfogconnect -lfogquic  -lwebsockets -lcrypto -lssl -lusrsctp -lglib-2.0 -levent -ljansson

server_SRCS = pr_server.c
server_OBJS = pr_server.o
client_SRCS = pr_client.c
client_OBJS = pr_client.o

all : test_server test_client

test_server: $(server_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
test_server.o: $(server_SRCS)
	$(CC) $(CFLAGS) $@ -c $^

test_client: $(client_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
test_client.o: $(client_SRCS)
	$(CC) $(CFLAGS)  $@ -c $^

.PHONY : test_server test_client
clean:
	rm -rf ./*.o  ./test_*




