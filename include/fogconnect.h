#ifndef FOGCONNECT_H
#define FOGCONNECT_H

typedef enum fc_transport_protocol {
    FOG_TRANSPORT_PROTOCOL_RTC = 0,
    FOG_TRANSPORT_PROTOCOL_UDP = 1,
    FOG_TRANSPORT_PROTOCOL_UTP,
    FOG_TRANSPORT_PROTOCOL_KCP,
    FOG_TRANSPORT_PROTOCOL_SCTP,
    FOG_TRANSPORT_PROTOCOL_QUIC,
} fc_transport_protocol;
#define FOG_TRANSPORT_PROTOCOL_DATACHANNEL	FOG_TRANSPORT_PROTOCOL_RTC

#define FOG_EVENT_EOF		    0x10	/**< eof file reached. */
#define FOG_EVENT_ERROR		    0x20	/**< unrecoverable error encountered. */
#define FOG_EVENT_TIMEOUT	    0x40	/**< user-specified timeout reached. */
#define FOG_EVENT_CONNECTED	    0x80	/**< connect operation finished. */
#define FOG_EVENT_DISCONNECT	0x100	/**< peer disconnect. */

typedef int   fc_signal_read_cb(void *data);
typedef void  fc_connect_cb(void *conn_info, short events, void *cb_arg);
typedef void  fc_recv_cb(void *conn_info, void *cb_arg, void *buf, int size);
typedef void  fc_close_cb(void *conn_info, void *cb_arg);

struct fc_ctx;

typedef struct fc_signal_server {
    unsigned short  port;           /**< 服务器监听端口号。*/
    char *url;                      /**< 服务器域名或IP。 */
    char *path;                     /**< ‘/ws’或‘/wss’. */
    const char *certificate;        /**< 证书路径。（可选）。*/
    const char *privatekey;         /**< 私钥文件路径。（可选）。*/
    struct fc_ctx  *ctx;
} fc_signal_server;

/*
    fc_signal_init: 链接信令服务器，返回0表示成功链接，1表示超时。
    参数请具体看struct fc_signal_server.
*/
int fc_signal_init(struct fc_signal_server *info);


/*
    fc_signal_setcb: 设置接受信令服务信息回调函数。
    ctx: fc_init的返回值。
    callback: 信令服务器返回信息的处理过程。 

    注：当回调函数完成处理后，返回非零值，fogconnect引擎将会继续处理（一般不是感兴趣的信息请返回非零值，反之）。
*/
void fc_signal_setcb(struct fc_ctx *ctx, fc_signal_read_cb *callback);


/*
    fc_signal_write_data: 向信令服务器发送数据（json格式）。
    ctx: fc_init的返回值。
    buf: 待传输的缓存。
    size: 缓存数据大小。
*/
int fc_signal_write_data(struct fc_ctx *ctx, const char *buf, int size);


/*
    fc_init: 初始化FogConnect.
    返回非NULL表示成功，失败为NULL.
*/
struct fc_ctx  *fc_init(void);


/*
    fc_release: 退出fogconnect.
    参数为: fc_init的返回值。
*/
void   fc_release(struct fc_ctx *ctx);


/*
    fc_passive_link_setcb: 设置被动连接时的回调函数。
    ctx: 为fc_init返回值。
    callback: 被连接后的处理过程。
*/
//void  fc_connect_setcb(struct fc_ctx *ctx, fc_connect_cb *callback);
void  fc_passive_link_setcb(struct fc_ctx *ctx, fc_connect_cb *callback);


/*
    fc_connect: 雾节点建立P2P连接。
    ctx: 为fc_init返回值。
    by_id: 信息由调度服务器返回，测试阶段可以写为对端的mac地址。
    tansport_protocol: 具体看enum tansport_protocol，这里确认本次连接后，数据传输使用的传输协议。
    use_service: 主动连接端，过通这个参数通知用户层处理的具体服务（主要是被连接端能执行指定的服务过程，默认为0）。
    conn_callback: 当P2P连接后，内部就会调用用户层回调函数来处理。
    cb_arg: 用户层私有数据。
    注：在回调函数被调用时需要保存相关的信息。具体看fc_send，fc_event_setcb函数需要使用的参数。
*/
int  fc_connect(struct fc_ctx *ctx, const char *by_id, fc_transport_protocol protocol,
                 int use_service, fc_connect_cb *conn_callback, void *cb_arg);


/*
    fc_event_setcb: 设置回调函数。
    conn_info: 为连接后的连接对象。
    recv：传输层受到数据后，调用应用层的数据处理函数（被动调用）。
    close:当连接断开，或超时将调用（被动调用）。
*/
int  fc_event_setcb(void *conn_info, fc_recv_cb *recv, fc_close_cb *close);


/*
    fc_send: 向雾节点发送数据。
    conn_info: 为连接后的连接对象。
    buf: 待传输的缓存。
    size: 缓存数据大小。
*/
int  fc_send(void *conn_info, void *buf, int size);


/*
    fc_is_passive: 判断节点连接的主/被动方式。
    conn_info: 为连接后的连接对象。
    返回值：     0表示主动发起链接，1表示被动被连接。
*/
int fc_is_passive(void *conn_info);


/*
    fc_get_convention_number: 获取双方约定数，以便处理相关过程。
    conn_info: 为连接后的连接对象。
    返回值：
    convention_n 各个位的定义。可以看出定义了的分部都是由fc_connect函数的相关参数
   （int protocol, int use_service）来确定。
    31    23    15    7     0
    |-----|-----|-----|-----|
                  |     |-------------->低4位（0～3）表示双方约定的传输协议。
                     |-------->8位(4~11)表示应用层自定定义的约定。
    |----------->(12~31)未定义。
*/
unsigned int  fc_get_convention_number(void *conn_info);


/*
    fc_set_userdata: 连接时，设置用户私有数据。
    conn_info: 为连接后的连接对象。
    user_data: 用户私有数据指针。
*/
void fc_set_userdata(void *conn_info, void *user_data);


/*
    fc_get_userdata: 获得连接时，设置用户私有数据。
    conn_info: 为连接后的连接对象。
*/
void *fc_get_userdata(void *conn_info);


/*
    fc_get_uploadspeed: 获得一个连接的上传速度。
    conn_info: 为连接后的连接对象。
*/
float fc_get_uploadspeed(void *conn_info);


/*
    fc_get_downloadspeed: 获得一个连接的下载速度。
    conn_info: 为连接后的连接对象。
*/
float fc_get_downloadspeed(void *conn_info);


/*
    fc_get_socket_fd: 获得一个连接的socket.
    conn_info: 为连接后的连接对象。
*/
int   fc_get_socket_fd(void *conn_info);


/*
    fc_get_local_addr: 获得与peer连接后本地的IP：PORT。
    conn_info: 为连接后的连接对象。
*/
struct sockaddr_in *fc_get_local_addr(void *connect_info);


/*
    fc_get_remote_addr: 获得与peer通信的IP：PORT。
    conn_info: 为连接后的连接对象。
*/
struct sockaddr_in *fc_get_remote_addr(void *conn_info);


/*
    fc_get_peer_mac: 获得与peer通信的mac信息。
    conn_info: 为连接后的连接对象。
*/
int fc_get_peer_mac(void *conn_info, char* out_buf, int len);


/*
    fc_set_separate: 设置后表示，连接断开时套接字不会被关闭。
    conn_info: 为连接后的连接对象。
*/
void fc_set_separate(void *connect_info);


/*
    fc_disconnect: 断开一个连接。
    conn_info: 为连接后的连接对象。
*/
void fc_disconnect(void *conn_info);


/*
*/
void fc_lock(void *conn_info);


/*
*/
void fc_unlock(void *conn_info);


/*
*/
void* fc_malloc(int size);


/*
*/
void fc_free(void *buf);


/*
    给节点设一个id.
*/
void fc_set_id(const char *id);
#endif
