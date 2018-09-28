#ifndef FOGCONNECT_H
#define FOGCONNECT_H

enum transport_protocol
{
    // protocol names
    PR_TRANSPORT_PROTOCOL_RTC = 0,
    PR_TRANSPORT_PROTOCOL_UDP,
    PR_TRANSPORT_PROTOCOL_UTP,
    PR_TRANSPORT_PROTOCOL_KCP,
    PR_TRANSPORT_PROTOCOL_SCTP,
    PR_TRANSPORT_PROTOCOL_QUIC,
};
#define PR_TRANSPORT_PROTOCOL_DATACHANNEL	PR_TRANSPORT_PROTOCOL_RTC

#define PR_EVENT_EOF		0x10	/**< eof file reached */
#define PR_EVENT_ERROR		0x20	/**< unrecoverable error encountered */
#define PR_EVENT_TIMEOUT	0x40	/**< user-specified timeout reached */
#define PR_EVENT_CONNECTED	0x80	/**< connect operation finished. */


typedef void  pr_callback_t(void* pr_connect, short events, void* cbarg);
typedef void  pr_recv_peer_t(void* pr_connect, void* cbarg, void* buf, int size);
typedef void  pr_close_peer_t(void* pr_connect, void* cbarg);


struct pr_signal_server
{
    ushort port;                    //服务器监听端口号。  
    void* ctx;
    char* url;                      //服务器域名或IP。    
    char* type;                     //‘/ws’或‘/wss’。
    const char* certificate;        //证书路径。（可选）
    const char* privatekey;         //私钥文件路径。（可选）
};

//链接信令服务器，返回0表示成功链接，1表示超时。
/*
    参数请具体看struct pr_signal_server。
*/
int    pr_init_signal(struct pr_signal_server* info); 


//初始化FogConnect。返回0表示成功，<0失败。
void*  pr_fogconnect_init(void);


//退出FogConnect。
/*
    参数为pr_fogconnect_init的返回值。
*/
void   pr_fogconnect_release(void*);


//设置被动时的回调函数,主要是被连接端可以更具相关定义协议作出相关反映。
/*
    ctx： 为pr_fogconnect_init返回值。
    callback：被连接后的处理过程。
*/
void  pr_fogconnect_set_callback(void* ctx, pr_callback_t* callback);



//连接雾节点。
/*
    ctx： 为pr_fogconnect_init返回值。
    peerID：信息由调度服务器返回，测试阶段可以写为对端的mac地址。
    protocol: 具体看enum tansport_protocol，这里确认本次连接后，数据传输使用的协议。
    connect_server: 主，被连接端，过通这个参数通知第三方处理的具体服务（主要是被连接端能跑到指定的服务过程）。
    callback: 当P2P连接后，模块就会调用第三方设置连接后的处理。
    cbarg: 第三方的私有数据。
    注：在回调函数被调用是需要保存相关的信息。具体看pr_send_peer，pr_event_setcb函数需要使用的参数。
*/
int  pr_connect_peer(void* ctx, char* peerID, int protocol, int connect_server,
                      pr_callback_t* callback, void* cbarg);



/*
    pr_connect: 为连接后，回调函数传入的参数看pr_callback_t。
    recv：传输层受到数据后，调用应用层的数据处理函数（被动调用）。
    close:当连接断开，或超时将被调用。
*/
int  pr_event_setcb(void* pr_connect, pr_recv_peer_t* recv, pr_close_peer_t* close);



//向雾节点发送数据。
/*
    pr_connect: 为连接后，回调函数传入的参数看pr_callback_t。
    buf：待传输的缓存。
    size:缓存数据大小。
*/
int  pr_send_peer(void* pr_connect, void* buf, int size);



//仅用于测试阶段，给节点设一个peer_id
/*
    char* peer_id: 目前使用mac地址作为peer_id进行识别。
*/
void set_id(char* peer_id);
#endif
