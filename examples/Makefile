LDFLAGS := -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
	-Wunused -fstrict-aliasing -Wextra -Wformat=2 -Winit-self -Winline \
	-Wpacked -Wpointer-arith -Wmissing-format-attribute -Wmissing-noreturn \
	-Wnested-externs -Wstrict-prototypes -Wunsafe-loop-optimizations \
	-Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter \
	-Wcast-align -Wformat-nonliteral -Wformat-security -Wswitch-default \
	-Wmissing-include-dirs -Wno-aggregate-return -Wunused-but-set-variable \
	-Warray-bounds -Wold-style-definition -Wsign-compare -Wlarger-than=65537


CFLAGS += -g -O0 -fdata-sections -ffunction-sections -fno-stack-protector -std=gnu99 \
	-fPIC -rdynamic -I../include -I./ -DINET -DINET6

LIBS :=  -L../dist/x86/linux/64/  -lfogconnect  -lwebsockets -levent -ljansson -lglib-2.0 -lpthread


filetransfer_server_SRCS = ./fog_connect.c  ./filetransfer/server.c ./filetransfer/ser.c
filetransfer_server_OBJS = ./fog_connect.o ./filetransfer/server.o  ./filetransfer/ser.o

filetransfer_client_SRCS = ./fog_connect.c ./filetransfer/client.c  ./filetransfer/ser.c
filetransfer_client_OBJS = ./fog_connect.o ./filetransfer/client.o  ./filetransfer/ser.o

echo_server_SRCS	= ./fog_connect.c  ./echo/server.c
echo_server_OBJS	= ./fog_connect.o  ./echo/server.o

echo_client_SRCS	= ./fog_connect.c  ./echo/client.c
echo_client_OBJS	= ./fog_connect.o  ./echo/client.o

get_addr_SRCS = ./getaddr/pr_get_addr.c
get_addr_OBJS = ./getaddr/pr_get_addr.o


all : filetransfer_server  filetransfer_client echo_server echo_client get_addr

filetransfer_server: $(filetransfer_server_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
filetransfer_server.o: $(filetransfer_server_SRCS)
	$(CC) $(CFLAGS) $@ -c $^

filetransfer_client: $(filetransfer_client_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
filetransfer_client.o: $(filetransfer_client_SRCS)
	$(CC) $(CFLAGS) $@ -c $^

echo_server: $(echo_server_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
echo_server.o: $(echo_server_SRCS)
	$(CC) $(CFLAGS) $@ -c $^

echo_client: $(echo_client_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
echo_client.o: $(echo_client_SRCS)
	$(CC) $(CFLAGS) $@ -c $^

get_addr: $(get_addr_OBJS)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
get_addr.o: $(get_addr_SRCS)
	$(CC) $(CFLAGS) $@ -c $^


.PHONY : filetransfer_server  filetransfer_client echo_server echo_client get_addr
clean: 
	rm ./filetransfer_server ./filetransfer_client ./echo_server ./echo_client ./get_addr ./*.o ./filetransfer/*.o ./echo/*.o ./getaddr/*.o
