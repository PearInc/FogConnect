## 雾连接传输组件（FogConnet）

FogConnet 是用于P2P网络中连接，调度，传输等功能于一体的组件。

### 架构图
![fog_connectstack](./doc/images/fogconnectstack.png)

### 特性
- 支持多种传输控制协议(QUIC,RTC,KCP,uTP,SCTP等)。
- 探测NAT类型，并收集和维护用于P2P连接的IP：PORT列表。
- 支持双向“打洞”和高级端口预测。
- NAT类型最优匹配组合策略。
- 连接控制与物理距离最近原则。
- 具有中继功能。
- 所有网络信号采用事件机制处理。
- 对资源消耗极少（一般运行状态下内存占用3～5M,峰值不超过50M）。
- API简单，易懂，支持多种方式接入。


## 快速开始
FogConnect depends on following packages:
- [openssl](https://www.cnblogs.com/emanlee/p/6100019.html)
- [libwebsockets.so.10](https://libwebsockets.org/)
- [libjansson.so.4](https://github.com/akheron/jansson)
- [libusrsctp.so.1](https://github.com/sctplab/usrsctp)
- [libevent-2.0.so.5](https://github.com/libevent/libevent)
- [libglib-2.0.so.0](https://github.com/GNOME/glib)


## Supported Environment
- Ubuntu/Linuxmint


### Ubuntu/LinuxMint

#### Prepare deps
Install common deps:

``` shell
sudo apt-get install git g++ make 
```

Install dependencies:

``` shell
sudo apt-get install openssl libssl-dev libwebsockets-dev libjansson-dev libevent-dev libglib2.0-dev 
```

Install libusrsctp:
``` shell
git clone https://github.com/sctplab/usrsctp.git
cd usrsctp/
cmake .
make
sudo cp usrsctplib/libusrsctp.so* /usr/lib/x86_64-linux-gnu/
sudo cp usrsctplib/libusrsctp.a /usr/lib/x86_64-linux-gnu/
```
#### Compile FogConnect
##### Prepare the FogConnect env
``` shell
git clone git@github.com:PearInc/FogConnect.git
cd FogConnect
sudo cp include/fogconnect.h /usr/include/
sudo cp x86/linux/64/libfog* /usr/lib/x86_64-linux-gnu/
```

##### Build and run examples
``` shell
make
./x86/linux/64/test_server && ./x86/linux/64/test_client
```
This example default use the QUIC protocol

## 性能测试

![benchmark](doc/images/P2P建立连接时间.png) \
[具体数据](doc/benchmark.md)


### 系统环境
Ubuntu 16.04.3 LTS \
Intel(R) Core(TM) i5-7500 CPU @ 2.40 GHz \
Mem: 7840 MB
## Examples

### Echo server
```C
#include "pr_fog_connect.h"

void on_connect(void* arg)
{
    printf("conn_cb\n");
}

void on_message(void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s from the peer\n", msg);
        int len = strlen(msg)+2;
        char* return_msg = (char*)malloc(len+1);
        sprintf(return_msg, "%s\r\n", msg);
        pr_send_peer(ud->pr_connect, return_msg, len);
        printf("sending the msg %s back to the peer\n", msg);
        free(msg);
        free(return_msg);
    }
}

void on_close(void* arg)
{
}

int main()
{
    pear_set_up("1e:34:a1:44:2c:1c", on_connect, on_message, on_close);

    for (int i = 0; i < 100; i++) {
        sleep(2);
    }
    pear_connect_release();

    return 0;
}

```
### Client

```C
#include "pr_fog_connect.h"

void on_connect(void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    char* msg = strdup("hello\r\n");
    pr_send_peer(ud->pr_connect, msg, strlen(msg));
    printf("sending the msg %s to peer\n", msg);
    free(msg);
}

void on_message(void* arg)
{
    pear_usr_data_t* ud = (pear_usr_data_t*)arg;
    size_t len = 0;
    char* msg = evbuffer_readln(ud->buff, &len, EVBUFFER_EOL_CRLF);
    if (msg != NULL) {
        printf("get the msg %s from peer\n", msg);
        free(msg);
    }
}

void on_close(void* arg)
{
    // call this function when the connection is closed
}

int main()
{
    pear_set_up("1e:34:a1:44:2c:2c", on_connect, on_message, on_close);
    pear_connect_peer("1e:34:a1:44:2c:1c");

    for (int i=0;i<100;i++) {
        sleep(2);
    }
    pear_connect_release();

    return 0;
}

```


### 第三方协议无缝连接
![third part connect](doc/images/third_part_connect.png)

### [更多例子](https://github.com/PearInc/FogConnect/tree/master/examples)

## 开发者简介
- 吴必磊(w@pear.hk)
    - 知名数据恢复软件Any Data Recovery Pro作者、反汇编与破解专家（同时也是不知名的健身教练）.

- 陈荡漾(66@pear.hk)
    
