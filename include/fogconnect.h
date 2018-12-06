#ifndef FOGCONNECT_H
#define FOGCONNECT_H

typedef enum fog_transport_protocol
{
    FOG_TRANSPORT_PROTOCOL_RTC = 0,
    FOG_TRANSPORT_PROTOCOL_UDP = 1,
    FOG_TRANSPORT_PROTOCOL_UTP,
    FOG_TRANSPORT_PROTOCOL_KCP,
    FOG_TRANSPORT_PROTOCOL_SCTP,
    FOG_TRANSPORT_PROTOCOL_QUIC,
}fog_transport_protocol;


#define FOG_TRANSPORT_PROTOCOL_DATACHANNEL	FOG_TRANSPORT_PROTOCOL_RTC

#define FOG_EVENT_EOF		    0x10	/**< eof file reached. */
#define FOG_EVENT_ERROR		    0x20	/**< unrecoverable error encountered. */
#define FOG_EVENT_TIMEOUT	    0x40	/**< user-specified timeout reached. */
#define FOG_EVENT_CONNECTED	    0x80	/**< connect operation finished. */
#define FOG_EVENT_DISCONNECT	0x100	/**< peer disconnect. */

typedef void  fog_connect_cb(void* conn_info, short events, void* cb_arg);
typedef void  fog_recv_cb(void* conn_info, void* cb_arg, void* buf, int size);
typedef void  fog_close_cb(void* conn_info, void* cb_arg);

typedef struct fog_signal_server
{
    unsigned short  port;           /**< 服务器监听端口号。*/  
    void* ctx;
    char* url;                      /**< 服务器域名或IP。 */
    char* type;                     /**< ‘/ws’或‘/wss’. */
    const char* certificate;        /**< 证书路径。（可选）。*/
    const char* privatekey;         /**< 私钥文件路径。（可选）。 */
}fog_signal_server;

struct fog_ctx;

/*
    fog_signal_init: 链接信令服务器，返回0表示成功链接，1表示超时。
    参数请具体看struct fog_signal_server.
*/
int    fog_signal_init(struct fog_signal_server* info); 


/*
    fog_init: 初始化FogConnect.
    返回非NULL表示成功，失败为NULL.
*/
struct fog_ctx*  fog_init(void);          


/*
    fog_release: 退出fogconnect.
    参数为: fog_init的返回值。
*/
void   fog_release(struct fog_ctx* ctx);


/*
    fog_connect_setcb: 设置被动时的回调函数,主要是被连接端可以更具相关定义协议作出相关反映。
    ctx: 为fog_init返回值。
    callback: 被连接后的处理过程。
*/
void  fog_connect_setcb(struct fog_ctx* ctx, fog_connect_cb* callback);


/*
    fog_connect: 连接雾节点。
    ctx: 为fog_init返回值。
    by_id: 信息由调度服务器返回，测试阶段可以写为对端的mac地址。
    tansport_protocol: 具体看enum tansport_protocol，这里确认本次连接后，数据传输使用的协议。
    use_service: 主动连接端，过通这个参数通知用户层处理的具体服务（主要是被连接端能跑到指定的服务过程）。
    conn_callback: 当P2P连接后，模块就会调用用户层设置连接后的处理。
    cb_arg: 用户层私有数据。
    注：在回调函数被调用是需要保存相关的信息。具体看fog_send_data，fog_event_setcb函数需要使用的参数。
*/
int  fog_connect(struct fog_ctx* ctx, const char* by_id, fog_transport_protocol protocol, 
                int use_service, fog_connect_cb* conn_callback, void* cb_arg);


/*
    fog_event_setcb: 设置回调函数。
    conn_info: 为连接后的连接对象。
    recv：传输层受到数据后，调用应用层的数据处理函数（被动调用）。
    close:当连接断开，或超时将被调用。
*/
int  fog_event_setcb(void* conn_info, fog_recv_cb* recv, fog_close_cb* close);


/*
    fog_send_data: 向雾节点发送数据。 
    conn_info: 为连接后的连接对象。
    buf: 待传输的缓存。
    size: 缓存数据大小。
*/
int  fog_send_data(void* conn_info, void* buf, int size);


/*
    fog_connect_is_passive: 判断节点连接的主/被动方式。
    conn_info: 为连接后的连接对象。
    返回值：     0表示主动发起链接，1表示被动被连接，
*/
int fog_connect_is_passive(void* conn_info);


/*
    fog_get_convention_number: 获取双方约定数，以便处理相关过程。
    conn_info: 为连接后的连接对象。
    返回值：
   convention_n 各个位的定义。可以看出定义了的分部都是由fog_connect函数的相关参数
   （int protocol, int use_service）来确定。
   31    23    15    7     0
   |-----|-----|-----|-----|
                  |     |-------------->低4位（0～3）表示双方约定的传输协议。
                     |-------->8位(4~11)表示应用层自定定义的约定。
   |----------->(12~31)未定义。
*/
unsigned int  fog_get_convention_number(void* conn_info);


/*
    fog_set_userdata: 连接时，设置用户私有数据。
    conn_info: 为连接后的连接对象。
    user_data: 用户私有数据指针。
*/
void fog_set_userdata(void* conn_info, void* user_data);


/*
    fog_get_userdata: 获得连接时，设置用户私有数据。
    conn_info: 为连接后的连接对象。
*/
void* fog_get_userdata(void* conn_info);


/*
    fog_get_uploadspeed: 获得一个连接的上传速度。
    conn_info: 为连接后的连接对象。
*/
float fog_get_uploadspeed(void* conn_info);


/*
    fog_get_uploadspeed: 获得一个连接的下载速度。
    conn_info: 为连接后的连接对象。
*/
float fog_get_downloadspeed(void* conn_info);


/*
    fog_get_socket: 获得一个连接的socket.
    conn_info: 为连接后的连接对象。
*/
int   fog_get_socket(void* conn_info);


/*
    fog_get_remote_addr: 获得与peer通信的IP：PORT。
    conn_info: 为连接后的连接对象。
*/
struct sockaddr_in* fog_get_remote_addr(void* conn_info);


/*
    fog_disconnect: 断开一个连接。
    conn_info: 为连接后的连接对象。
*/
void fog_disconnect(void* conn_info);


/*
*/
void fog_lock(void* conn_info);


/*
*/
void fog_unlock(void* conn_info);


/*
    仅用于测试阶段，给节点设一个id.
*/
void set_id(const char* id);
#endif