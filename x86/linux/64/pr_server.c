#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <glib.h>
#include "fogconnect.h"


struct pr_user_data 
{
    void* pr_data;
    int    socket;
    struct bufferevent* bev;
    struct event_base* base;
};

void  pr_socket_tcp_init_client(void* pr_conn);
#if 1
void pr_recv_data(void* pr_conn, void* cb_arg, void* buf, int size)
{
    int l_size;
    struct pr_user_data* user_data = (cb_arg);
    int send_count = 0;

    if (buf && size)
    {
        fog_lock(user_data->pr_data);
        if (user_data->socket == -1)
        {
            GError* error = NULL;
            fog_unlock(user_data->pr_data);
            g_free(user_data);
            fog_set_userdata(pr_conn, NULL);
            pr_socket_tcp_init_client(pr_conn);
            while(1)
            {
                void* userdata =  NULL;
                userdata = fog_get_userdata(pr_conn);
                if (userdata == (void*)-1)
                {
                    GError* error = NULL;
                    g_thread_try_new("fog_disconnect", fog_disconnect, pr_conn, &error);
                    return;
                }
                else if (userdata) break;
                g_usleep(100000);
            }
        }
        else fog_unlock(user_data->pr_data);
        int s_len = send(user_data->socket, buf, size, 0);
        while(s_len < 0)
        {
			s_len = send(user_data->socket, buf, size, 0);
			if(send_count > 5) break;
			else if(s_len == size) break;
			send_count++;
		}
    }
    return;
}

#define 	HOST_PORT		4998
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
        fog_send_data(user_data->pr_data, msg, len);
    }
    return;
}

void fog_socket_event_cb(struct bufferevent *bev, short events, void *arg)
{
	struct pr_user_data* user_data = (struct pr_user_data*)arg;

    if (events & BEV_EVENT_EOF) printf("events & BEV_EVENT_EOF!\n");
    if (events & BEV_EVENT_ERROR) printf("events & BEV_EVENT_ERROR!\n");
    if (events & BEV_EVENT_TIMEOUT) printf("events & BEV_EVENT_TIMEOUT!\n");
    if (events & BEV_EVENT_CONNECTED) return;
    fog_lock(user_data->pr_data);
    user_data->socket = -1;
    fog_unlock(user_data->pr_data);
}

void fog_socket_send_cb(struct bufferevent *bev, void *arg)
{
	struct pr_user_data* user_data = (struct pr_user_data*)arg;
	event_base_loopbreak(user_data->base);
	bufferevent_setcb(bev, NULL, NULL, NULL, NULL);
	fog_socket_event_cb(bev, BEV_EVENT_EOF, arg);
}


//TCP server
struct event_info
{
	struct event_base*  base;
	struct bufferevent* bev;
};

void pr_dispatch(void* info) 
{
    struct event_info* ev = info;
    event_base_dispatch(ev->base);
    bufferevent_free(ev->bev);
    event_base_free(ev->base);
    g_free(ev);
    printf("bufferevent_free!\n");
	g_thread_unref(g_thread_self());
}

void  pr_socket_tcp_init_client(void* pr_conn)
{
    struct pr_user_data* user_data = g_malloc0(sizeof(struct pr_user_data));
    struct event_base* base = event_base_new();
    struct bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HOST_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr, sizeof(server_addr));

    user_data->pr_data  = pr_conn;
	user_data->bev      = bev;
    user_data->base     = base;
    user_data->socket   = bufferevent_getfd(bev);
    fog_set_userdata(pr_conn, user_data);
    bufferevent_setcb(bev, pr_socket_read_cb, fog_socket_send_cb, fog_socket_event_cb, user_data);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    GError* error = NULL;
    struct event_info* ev = g_malloc0(sizeof(struct event_info));
    ev->bev = bev;
    ev->base = base;
    g_thread_try_new("thread_dispatch", pr_dispatch, ev, &error);
    return;
}

void  pr_close_connect(void* connect, void* cb_arg)
{
    int is_socket = 0;
    struct pr_user_data* arg = (cb_arg);

    if (cb_arg == NULL) return;
    fog_lock(arg->pr_data);
    if (arg->socket != -1)
    {
        is_socket = 1;
        bufferevent_enable(arg->bev, EV_WRITE | EV_PERSIST);
    }
    fog_unlock(arg->pr_data);
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
    struct pr_user_data* arg = (cb_arg);
    switch (events)
    {
        case FOG_EVENT_CONNECTED:
            printf("set  callback!\n");
            if (arg) arg->pr_data = pr_conn;
            else if (fog_connect_is_passive(pr_conn))
            {
                pr_socket_tcp_init_client(pr_conn);
                pr_set_third_callback(pr_conn);
/*
                //表示当前雾节点，被其它雾节点发起了链接，当前雾节点为被动方。
                struct pr_user_data* cb_arg_data = g_malloc0(sizeof(struct pr_user_data));
                fog_set_userdata(pr_conn, cb_arg_data);
                //保存链接信息以便其它过程使用。
                cb_arg_data->pr_data = pr_conn;
*/              
                break;
            }
            //设置回调，以便应用读取雾节点数据。
            pr_set_third_callback(pr_conn);
            break;
        case FOG_EVENT_EOF:
        case FOG_EVENT_ERROR:
            printf("connect fail!\n");
            pr_close_connect(pr_conn, cb_arg);
            fog_disconnect(pr_conn);
            break;
        case FOG_EVENT_TIMEOUT:
            printf("time out!\n");
            pr_close_connect(pr_conn, cb_arg);
            fog_disconnect(pr_conn);
            break;
        default:
            break;
    }
    return;
}
#endif

#if (__linux__ || __unix__)
#include <net/if.h>
#include <sys/ioctl.h>
#endif
char g_mac_buf[32] = {0,};
int  pr_get_mac(char * mac_address)
{
    char buf[1024];
    int success = 0;
	memset(buf, 0, 1024);

    if(mac_address != g_mac_buf  && g_mac_buf[0] != 0)
    {
		memcpy(mac_address, g_mac_buf , strlen(g_mac_buf));
		return 0;
	}

#ifdef DDDD
#elif  (__linux__ || __unix__)

	struct ifreq ifr;
	struct ifconf ifc;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { return -1;  /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { return -2; /* handle error */ }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; it++)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (! (ifr.ifr_flags & IFF_LOOPBACK))
            { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
                {
                    success = 1;
                    break;
                }
            }
        } else { /* handle error */ }
    }    
    unsigned char mac[6];
    if (success) 
    {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		if( mac_address != g_mac_buf ) memcpy(mac_address, g_mac_buf , strlen(g_mac_buf));
    }
	close(sock);
#else	
	FILE *fp = popen("ifconfig en0 | awk '/ether/{print $2}'", "r");
	fscanf(fp, "%s", mac_address);
	pclose(fp);
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    int count = 100;

    struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);

    //初始化fogconnect组件。
    printf("fog_init!\n");
    void* ctx = fog_init();
    if (!ctx) return 0;

    pr_get_mac(g_mac_buf);
    printf("local mac = %s!\n", g_mac_buf);

    //测试时，设置的ID，这么为MAC地址。
    set_id(g_mac_buf);

    //设置被动时的回调函数。
    fog_connect_setcb(ctx, fog_connect_callback);
    printf("ctx = %p\n", ctx);

    //以下以连接信令服务器的操作。
    struct fog_signal_server* signal_info = g_malloc0(sizeof(struct fog_signal_server));
    signal_info->ctx  = ctx;
    signal_info->url  = "47.52.153.245";

    signal_info->type = "/ws";
    signal_info->certificate = NULL;
    signal_info->privatekey  = NULL;

    signal_info->port = 7600;
    fog_signal_init(signal_info);
#if 1
    //以下为主动对雾节点发起的链接。
    {
#ifdef TEST_CLENET
        //quic protocol
    void* user_data = g_malloc0(sizeof(struct pr_user_data));
    int pr_udp = fog_connect(ctx, "ee:34:a1:44:2c:1c",
                               atoi(argv[1]), 
                               1, //注意这个参数，与服务的选择相关联。请看pr_set_third_callback函数的处理。
                               fog_connect_callback, user_data);
#endif
    }
        printf("Please press any key to exit... ");
	getchar();
    fog_release(ctx);
#endif
    return 0;
}