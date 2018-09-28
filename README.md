## 雾连接传输组件（FogConnet）

FogConnet 是用于P2P网络中连接，调度，传输等功能于一体的组件。

### 架构图
![fog_connectstack](./doc/images/fogconnectstack.png)

### 特性
1. 支持多种传输控制协议(QUIC,RTC,KCP,uTP,SCTP等等)。
2. 探测NAT类型，并收集和维护用于P2P连接的IP：PORT列表。
3. 支持双向“打洞”和高级端口预测。
4. NAT类型最优匹配组合策略。
5. 连接控制与物理距离最近原则。
6. UDP协议作为传输层。
7. 支持多种传输控制协议(QUIC, RTC, KCP, uTP, SCTP等等)。
8. 具有中继功能。
9. 所有网络信号采用事件机制处理。
10. 对资源消耗极少（一般运行状态下内存占用3～5M,峰值不超过50M）。
11. API简单，易懂，支持多种方式接入。


## 快速开始
- 阅读[编译步骤](doc/getting_started.md)了解如何开始使用.

## 性能测试

|开始时间 | 建立连接时间 | 建立datachannel时间|建立连接时长(单位S)|建立DataChannel时长(单位S)|总时间|
| ------- | ------- | --------- | --------- | ------------ | ---------- |
|40.201|42.12|43.187|1.919|1.067|2.986|
|18.005|19.934|21.054|1.929|1.12|3.049|　
|28.88|30.795|31.851|1.915|1.056|2.971|
|57.951|59.866|60.879|1.915|1.013|2.928|　
|14.454|16.534|17.433|2.08|0.899|2.979|　
|15.07|17.064|18.016|1.994|0.952|2.946|
|50.173|52.167|	53.153|1.994|0.986|2.98|
|19.378|21.282|22.285|1.904|1.003|2.907|　
|40.897|42.956|43.992|2.059|1.036|3.095|　
|7.248|9.163|10.209|1.915|1.046|2.961|
|　	平均值|||1.9624|1.0178|2.9802|

![benchmark](doc/images/P2P建立连接时间.png)

### 系统环境
Ubuntu 16.04.3 LTS \
Intel(R) Core(TM) i5-7500 CPU @ 2.40 GHz \
Mem: 7840 MB
## Examples

### Echo server
```C
#include "pr_fog_connect.h"

void connecting_cb(void* arg) {
    printf("conn_cb\n");
}

void msg_cb(void* arg) {
    printf("msg_cb\n");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s\n", msg);
        char* return_msg = g_strdup_printf("%s\r\n", msg);
        pr_send_peer(ud->pr_connect, return_msg, strlen(return_msg));
        free(msg);
        free(return_msg);
    }
}

void close_cb(void* pr_connect, void* arg) {
    pear_usr_data_free(arg);
}

int main() {
    SETUP("1e:34:a1:44:2c:1c", connecting_cb, msg_cb, close_cb);
    for (int i = 0; i < 100; i++) {
        sleep(2);
    }
    pear_fog_connect_release();
    return 0;
}
```
### Client

```C
#include "pr_fog_connect.h"

void connecting_cb(void* arg) {
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    char* msg = g_strdup("jys\r\n");
    pr_send_peer(ud->pr_connect, msg, strlen(msg));
    free(msg);
}

void msg_cb(void* arg) {
    printf("msg cb\n");
    pr_usr_data_t* ud = (pr_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s\n", msg);
        free(msg);
    }
}

void close_cb(void* pr_connect, void* arg) {
    pear_usr_data_free(arg);
}

int main() {
    SETUP("1e:34:a1:44:2c:2c", connecting_cb, msg_cb, close_cb);
    pear_connect_peer("1e:34:a1:44:2c:1c");
    for (int i=0;i<100;i++) {
        sleep(2);
    }
    pear_fog_connect_release();
    return 0;
}
```


### 第三方协议无缝连接
![third part connect](doc/images/third_part_connect.png)

### [更多例子](https://github.com/PearInc/FogConnect/tree/master/examples)

## 文档
- [FogConnet协议栈](doc/PTP-Connect协议栈.md)


## 开发者简介
- 吴必磊(w@pear.hk)
    - 知名数据恢复软件Any Data Recovery Pro作者、反汇编与破解专家（同时也是不知名的健身教练）7年以上产品开发经验的首席技术负责人。

- 陈荡漾(66@pear.hk)

