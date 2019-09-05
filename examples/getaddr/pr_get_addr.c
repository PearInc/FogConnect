#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "fogconnect.h"

struct pr_user_data {
    int    socket;
    struct sockaddr_in  local_addr;
    struct sockaddr_in  remote_addr;
    void  *usr_data;
};

void fc_connect_callback(void *pr_conn, short events, void *cb_arg) {
    struct sockaddr_in  *tmp_remote;
    struct sockaddr_in  *tmp_local;
    struct pr_user_data *arg = (cb_arg);
    switch (events) {
    case FOG_EVENT_CONNECTED:
        if (!arg) {
            /*
                被连接端，cb_arg为NULL.
            */
            arg = fc_malloc(sizeof(struct pr_user_data));
            if (arg == NULL) {
                fc_disconnect(pr_conn);
                return;
            }
        }
        arg->socket = fc_get_socket_fd(pr_conn);
        tmp_remote = fc_get_remote_addr(pr_conn);
        if (tmp_remote) memcpy(&arg->remote_addr, tmp_remote, sizeof(struct sockaddr_in));
        tmp_local  = fc_get_local_addr(pr_conn);
        if (tmp_local) memcpy(&arg->local_addr, tmp_local, sizeof(struct sockaddr_in));
        /*
            标记本次连接，在调用fc_disconnect后仅仅只是脱离fogconnect的控制。
        */
        fc_set_separate(pr_conn);
        fc_disconnect(pr_conn);

        /*
            在这里处理获取的连接信息。
            arg->socket(本次连接的套接字),
            arg->local_addr(本次连接的local ip:port),
            arg->remote_addr(本次连接的remote ip:port),
            可以在程序中任意使用，传输协议为UDP.
        */
        
        /*
            如想在其它地方，使用本次连接信息来通信，这不需要释放和关闭套接字。
        */

#if 1
            printf( "connect info: %s:%d ------->",
            inet_ntoa(arg->local_addr.sin_addr), htons(arg->local_addr.sin_port));
            printf("%s:%d\n",inet_ntoa(arg->remote_addr.sin_addr), htons(arg->remote_addr.sin_port));
            if (arg) fc_free(arg);
#endif

        if (arg) fc_free(arg);
        if (arg->socket != -1) close(arg->socket);
        break;
    case FOG_EVENT_EOF:
    case FOG_EVENT_ERROR:
    case FOG_EVENT_TIMEOUT:
        fc_disconnect(pr_conn);
        break;
    default:
        break;
    }
}


void pr_set_signal_info(struct fc_signal_server *signal_info) {
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

int  fc_get_mac(char *mac_address) {
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
    if (sock == -1) {
        return -1;  /* handle error*/
    };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return -2; /* handle error */
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; it++) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
                // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        } else {
            /* handle error */
        }
    }
    unsigned char mac[6];
    if (success) {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4],
                mac[5]);
        if (mac_address != g_mac_buf) memcpy(mac_address, g_mac_buf , strlen(g_mac_buf));
    }
    close(sock);
    #else
    FILE *fp = popen("ifconfig en0 | awk '/ether/{print $2}'", "r");
    fscanf(fp, "%s", mac_address);
    pclose(fp);
    #endif
    return 0;
}

//传入peer mac地址
int main(int argc, char *argv[]) {
    //初始化fogconnect组件。
    void *ctx = fc_init();
    if (!ctx) return 0;

    //测试时，设置的ID，这么为MAC地址。
    //fc_set_id("ee:34:a1:44:1c:1c");
    fc_get_mac(g_mac_buf);
    fc_set_id(g_mac_buf);

    //设置被动时的回调函数。
    fc_passive_link_setcb(ctx, fc_connect_callback);
    //以下以连接信令服务器的操作。
    struct fc_signal_server *signal_info = fc_malloc(sizeof(struct fc_signal_server));
    signal_info->ctx  = ctx;
    pr_set_signal_info(signal_info);
    fc_signal_init(signal_info);

    if (argc > 1) {
        //以下为主动对雾节点发起的链接。
        //UDP protocol
        void *user_data = fc_malloc(sizeof(struct pr_user_data));
        int pr_udp = fc_connect(ctx, argv[1], FOG_TRANSPORT_PROTOCOL_UDP,
                                0, fc_connect_callback, user_data);
    }

    printf("Please press any key to exit... \n");
    fc_release(ctx);
    return 0;
}
