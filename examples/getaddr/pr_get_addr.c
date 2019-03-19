#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "fogconnect.h"

struct pr_user_data
{
    int    socket;
    struct sockaddr_in  local_addr;
    struct sockaddr_in  remote_addr;
    void*  usr_data;
};

void fog_connect_callback(void* pr_conn, short events, void* cb_arg) {
    struct sockaddr_in  *tmp_remote;
    struct sockaddr_in  *tmp_local;

    struct pr_user_data* arg = (cb_arg);
    switch (events) {
        case FOG_EVENT_CONNECTED:
            if (!arg || fog_connect_is_passive(pr_conn)) {
                arg = fog_malloc(sizeof(struct pr_user_data));
                if (arg == NULL) {
                    fog_disconnect(pr_conn);
                    return;
                }
            }

            arg->socket = fog_get_socket(pr_conn);
            tmp_remote = fog_get_remote_addr(pr_conn);
            if (tmp_remote) memcpy(&arg->remote_addr, tmp_remote, sizeof(struct sockaddr_in));
            tmp_local  = fog_get_local_addr(pr_conn);
            if (tmp_local) memcpy(&arg->local_addr, tmp_local, sizeof(struct sockaddr_in));
            fog_connect_set_separate(pr_conn);
            fog_disconnect(pr_conn);
#if 1
            printf( "connect info: %s:%d ------->",
            inet_ntoa(arg->local_addr.sin_addr), htons(arg->local_addr.sin_port));
            printf("%s:%d\n",inet_ntoa(arg->remote_addr.sin_addr), htons(arg->remote_addr.sin_port));
            if (arg) fog_free(arg);
#endif
            if (arg->socket != -1) close(arg->socket);
            break;
        case FOG_EVENT_EOF:
        case FOG_EVENT_ERROR:
        case FOG_EVENT_TIMEOUT:
            fog_disconnect(pr_conn);
            break;
        default:
            break;
    }
}


void pr_set_signal_info(struct fog_signal_server* signal_info)
{
    if (signal_info == NULL) exit(0);
    signal_info->url  = "122.152.200.206";
    signal_info->path = "/ws";
    signal_info->certificate = NULL;
    signal_info->privatekey  = NULL;
    signal_info->port = 7600;
}

#if (__linux__ || __unix__)
#include <net/if.h>
#include <sys/ioctl.h>
#endif
char g_mac_buf[32] = {0,};
int  fog_get_mac(char* mac_address) {
    char buf[1024];
    int success = 0;
	memset(buf, 0, 1024);

    if (mac_address != g_mac_buf  && g_mac_buf[0] != 0) {
		memcpy(mac_address, g_mac_buf , strlen(g_mac_buf));
		return 0;
	}

#ifdef __test__
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

    for (; it != end; it++) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        } else { /* handle error */ }
    }
    unsigned char mac[6];
    if (success) {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		if( mac_address != g_mac_buf ) memcpy(mac_address, g_mac_buf , strlen(g_mac_buf));
    }
	close(sock);
#else
	FILE* fp = popen("ifconfig en0 | awk '/ether/{print $2}'", "r");
	fscanf(fp, "%s", mac_address);
	pclose(fp);
#endif
    return 0;
}

int main(int argc, char *argv[]) {
    int count = 100;

    struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);

    //初始化fogconnect组件。
    void* ctx = fog_init();
    if (!ctx) return 0;

    //测试时，设置的ID，这么为MAC地址。
    //set_id("ee:34:a1:44:1c:1c");
    fog_get_mac(g_mac_buf);
    set_id(g_mac_buf);

    //设置被动时的回调函数。
    fog_passive_link_setcb(ctx, fog_connect_callback);
    printf("ctx = %p\n", ctx);

    //以下以连接信令服务器的操作。
    struct fog_signal_server* signal_info = malloc(sizeof(struct fog_signal_server));
    signal_info->ctx  = ctx;
    pr_set_signal_info(signal_info);
    fog_signal_init(signal_info);

    if ( argc > 1 ) {
        usleep(100000);
        //以下为主动对雾节点发起的链接。
        //UDP protocol
        void* user_data = fog_malloc(sizeof(struct pr_user_data));
        int pr_udp = fog_connect(ctx, argv[1],
                               FOG_TRANSPORT_PROTOCOL_UDP,
                               0, //注意这个参数，与服务的选择相关联。请看pr_set_third_callback函数的处理。
                               fog_connect_callback, user_data);
    
    }
    //printf("Please press any key to exit... \n");
	while(1){
        usleep(1000);
    }
    
    fog_release(ctx);
    return 0;
}