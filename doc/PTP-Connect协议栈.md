
## 以下为 API 使用说明:
- 第一个回调：节点主动连接或被连接,当连接上时应用层来决定怎么处理这个连接动作(回调函数名为pr_connect_callback)。
```c
    //设置被动时的回调函数。
    pr_fogconnect_set_callback(ctx, pr_connect_callback);
    
    //quic protocol
    void* cbarg_udp = g_malloc0(sizeof(struct pr_usr_data));
    int pr_udp = pr_connect_peer(ctx, "ee:34:a1:44:2c:1c",
                               PR_TRANSPORT_PROTOCOL_QUIC, 
                               1, //注意这个参数，与服务的选择相关联。请看pr_set_third_callback函数的处理。
                               pr_connect_callback, cbarg_udp);
```

- 第二个回调：在建立了P2P连接后，在pr_connect_callback被调用时来注册这个连接获得数据后，应用层对数据的处理（回调函数名为pr_recv_data）
```c
pr_event_setcb(pr_connect, pr_recv_data, pr_close_connect);
```

- 第三个回调：在建立了P2P连接后，在pr_connect_callback被调用时来注册这个连接断开和超时后，应用层对这个事件的处理（回调函数名为pr_close_connect）
```c
pr_event_setcb(pr_connect, pr_recv_data, pr_close_connect);
```

- 操作一：应用层私有数据与连接对象绑定过程（被动连接的情况下）
```c
//表示当前雾节点，被其它雾节点发起了链接，当前雾节点为被动方。
struct pr_usr_data* cbarg_data = g_malloc0(sizeof(struct pr_usr_data));
pr_connect_set_userdata(pr_connect, cbarg_data);
```

- 操作二：主要是双方约定最终这个连接使用应层的那个服务。
```c
    uint32_t convention_n = pr_get_convention_number(pr_connect);
    switch(((convention_n & 0xff0) >> 4))
```

- 保存：应用层保存连接对象（void* pr_connect)
```c
//保存链接信息以便其它过程使用。
cbarg_data->pr_data = pr_connect;
```
