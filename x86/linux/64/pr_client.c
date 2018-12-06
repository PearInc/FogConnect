#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <errno.h>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <glib.h>
#include "fogconnect.h"


struct fog_usr_data 
{
    void* pr_data;
    int    socket;
    struct bufferevent* bev;
    struct event_base*  base;
};

void pr_recv_data(void* pr_conn, void* cb_arg, void* buf, int size)
{
    int l_size;
    struct pr_user_data* user_data = (cb_arg);
    int send_count = 0;

    if (buf && size)
    {
        int s_len = send(user_data->socket, buf, size, 0);
        while(s_len < 0)
        {
			s_len = send(user_data->socket, buf, size, 0);
			if(send_count > 5) break;
			else if(s_len == size) break;
			g_usleep(50000);
			send_count++;
		}
    }
    return;
}

void  pr_close_connect(void* connect, void* cb_arg)
{
    int is_socket = 0;
    struct pr_user_data* arg = (cb_arg);

    if (cb_arg == NULL) return;
    if (arg->pr_data) fog_lock(arg->pr_data);
    if (arg->socket != -1)
    {
        is_socket = 1;
        bufferevent_enable(arg->bev, EV_WRITE | EV_PERSIST);
    }
    if (arg->pr_data)  fog_unlock(arg->pr_data);
    while (is_socket) 
    {
        if (arg->socket == -1) break;
        g_usleep(50000);
    } 
    return;
}

void pr_closed_free(void* connect, void* cb_arg)
{
    printf("pr_close_connect!\n");
    g_free(cb_arg);
}

#define  pr_recv_data_1 pr_recv_data
#define  pr_close_connect_1 pr_closed_free

void pr_socket_read_cb(struct bufferevent *bev, void *arg)
{
	int ret 					= 0;
	size_t len					= 0;
    char msg[16*1024 +1] 		= {0 ,};
    struct pr_user_data* user_data = arg;

    len = bufferevent_read(bev, msg, sizeof(msg)-1);
    if (len != 0)
    {
        msg[len] = '\0';
        printf("%s!\n",msg);
        fog_send_data(user_data->pr_data, msg, len);
    }
    return;
}

void fog_socket_event_cb(struct bufferevent* bev, short events, void* arg)
{
    struct pr_user_data* user_data = (struct pr_user_data*)arg;

    if(events & BEV_EVENT_EOF) printf("events & BEV_EVENT_EOF!\n");
    if(events & BEV_EVENT_ERROR) printf("events & BEV_EVENT_ERROR!\n");
    if(events & BEV_EVENT_TIMEOUT) printf("events & BEV_EVENT_TIMEOUT!\n");
    if(events & BEV_EVENT_CONNECTED) return;
    if (user_data->pr_data) fog_lock(user_data->pr_data);
    if (user_data->socket != -1) close(user_data->socket);
    user_data->socket = -1;
    if (user_data->pr_data) fog_unlock(user_data->pr_data);
    bufferevent_free(bev);
}

void fog_socket_send_cb(struct bufferevent *bev, void *arg)
{
	struct pr_user_data* user_data = (struct pr_user_data*)arg;
	//event_base_loopbreak(user_data->base);
    fog_socket_event_cb(bev, BEV_EVENT_EOF, arg);
    bufferevent_setcb(bev, NULL, NULL, NULL, NULL);
}

void pr_set_third_callback(void* pr_conn)
{
/*
   convention_n 各个位的定义。可以看出定义了的分部都是由fog_connect函数的相关参数（int protocol, int connect_server）来确定。
   31    23    15    7     0
// |-----|-----|-----|-----|
                  |     |-------------->低4位（0～3）表示双方约定的传输协议。
                     |-------->8位(4~11)表示应用层自定定义的约定。
   |----------->(12~31)未定义。
*/
    uint32_t convention_n = fog_get_convention_number(pr_conn);
    switch(((convention_n & 0xff0) >> 4))
    {
        case 0:
            fog_event_setcb(pr_conn, pr_recv_data, pr_closed_free);
            break;
        //以下为示例，根据应用需要来决定。
        case 1:
            fog_event_setcb(pr_conn, pr_recv_data_1, pr_close_connect_1);
            //fog_send_data(pr_conn, "I am fog data!", strlen("I am fog data!"));
            break;
        case 2:
            //fog_event_setcb(pr_conn, pr_recv_data_2, pr_close_connect_2);
            break;
        default:
            //fog_event_setcb(pr_conn, pr_recv_data_default, pr_close_connect_default);
            break;
    }
}

void fog_connect_callback(void* pr_conn, short events, void* cb_arg)
{
    struct fog_usr_data* arg = (cbarg);
    switch (events)
    {
        case FOG_EVENT_CONNECTED:
            if (arg)
            {
                arg->pr_data = pr_conn;
                //设置回调，以便应用读取雾节点数据。
                pr_set_third_callback(pr_conn);
                bufferevent_setcb(arg->bev, pr_socket_read_cb, fog_socket_send_cb, fog_socket_event_cb, cb_arg);
	            bufferevent_enable(arg->bev, EV_READ  | EV_PERSIST);
            }
            else if (fog_connect_is_passive(pr_conn))
            {
#if 0
                //表示当前雾节点，被其它雾节点发起了链接，当前雾节点为被动方。
                struct fog_usr_data* cbarg_data = g_malloc0(sizeof(struct fog_usr_data));
                pr_connect_set_userdata(pr_connect, cbarg_data);
                //保存链接信息以便其它过程使用。
                cb_arg_data->pr_data = pr_conn;
#endif
            }
            break;
        case FOG_EVENT_EOF:
        case FOG_EVENT_ERROR:
            printf("connect fail!\n");
            if (arg->pr_data == NULL) bufferevent_setcb(arg->bev, NULL, fog_socket_send_cb, NULL, cb_arg);
            pr_close_connect(pr_conn, cb_arg);
            fog_disconnect(pr_conn);
            break;
        case FOG_EVENT_TIMEOUT:
            printf("time out!\n");
            if (arg->pr_data == NULL) bufferevent_setcb(arg->bev, NULL, fog_socket_send_cb, NULL, cb_arg);
            pr_close_connect(pr_conn, cb_arg);
            fog_disconnect(pr_conn);
            break;
        default:
            break;
    }
    return;
}

//TCP server
struct event_info
{
	struct event_base	*base;
	void				*info;
};

void fog_listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
                      struct sockaddr* sock, int socklen, void* arg)
{
	struct event_info* conn_info = (struct event_info*)arg;

    //为这个客户端分配一个bufferevent
	struct bufferevent* bev =  bufferevent_socket_new(conn_info->base, fd, BEV_OPT_CLOSE_ON_FREE);
    struct pr_user_data* user_data = g_malloc0(sizeof(struct pr_user_data));

	user_data->bev      = bev;
    user_data->base     = conn_info->base;
    user_data->socket   = bufferevent_getfd(bev);

    int pr_udp = fog_connect(conn_info->info,
                                "20:76:93:93:93:93",
                                FOG_TRANSPORT_PROTOCOL_KCP,
                                0, //注意这个参数，与服务的选择相关联。请看pr_set_third_callback函数的处理。
                                fog_connect_callback, 
                                user_data);
	return;
}

void pr_socket_tcp_init(void* ctx ,int port)
{
	struct sockaddr_in sin;
	struct event_info*  l_info = g_malloc0(sizeof(struct event_info));
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(port);
	struct event_base *base = event_base_new();

	l_info->base = base;
	l_info->info = ctx;
    struct evconnlistener *listener
            = evconnlistener_new_bind(base, fog_listener_cb, l_info,
                                      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,
                                      10, (struct sockaddr*)&sin,
                                      sizeof(struct sockaddr_in));
    if(listener == NULL) 
    {
		printf("[test] evconnlistener_new_bind =%d\n" , errno);
		goto erro;
	}

    //evutil_socket_t fd = evconnlistener_get_fd(listener);
    printf("evconnlistener_new_bind!\n");
    event_base_dispatch(base);
    evconnlistener_free(listener);
erro:
    printf("evconnlistener_free\n");
    event_base_free(base);
    g_free(l_info);
    return;
}

void pr_set_signal_info(struct fog_signal_server* signal_info)
{
    if (signal_info == NULL) exit(0);
    signal_info->url  = "47.52.153.245";
    signal_info->type = "/ws";
    signal_info->certificate = NULL;
    signal_info->privatekey  = NULL;
    signal_info->port = 7600;
}

int main(int argc, char *argv[])
{
    int count = 100;

    struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);

    //初始化fogconnect组件。
    void* ctx = fog_init();
    if (!ctx) return 0;

    //测试时，设置的ID，这么为MAC地址。
    set_id("ea:34:a1:44:1c:1c");

    //设置被动时的回调函数。
    fog_connect_setcb(ctx, fog_connect_callback);
    printf("ctx = %p\n", ctx);

    //以下以连接信令服务器的操作。
    struct fog_signal_server* signal_info = malloc(sizeof(struct fog_signal_server));
    signal_info->ctx  = ctx;
    pr_set_signal_info(signal_info);
    fog_signal_init(signal_info);
    pr_socket_tcp_init(ctx, 31134);
#if 0
    //以下为主动对雾节点发起的链接。
    {
        //quic protocol
    void* cbarg_udp = g_malloc0(sizeof(struct fog_usr_data));
    int pr_udp = pr_connect_peer(ctx, "ee:34:a1:44:2c:1c",
                               PR_TRANSPORT_PROTOCOL_QUIC, 
                               1, //注意这个参数，与服务的选择相关联。请看pr_set_third_callback函数的处理。
                               fog_connect_callback, user_data);
    }
#endif
    printf("Please press any key to exit... ");
	getchar();
    fog_release(ctx);
    return 0;
}














