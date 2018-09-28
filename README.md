## Pear传输组件（PTP-FogConnet）

PTP-FogConnet是用于P2P网络中连接，调度，传输等功能于一体的组件。

### 组件功能
- 支持多种传输控制协议(QUIC,RTC,KCP,uTP,SCTP等等)。
- 更多功能详情请点[这里](https://km.webrtc.win/index.php?share/file&user=102&sid=NkyPTXUR)。


### 环境搭建
#### 依赖库及工具
- [openssl](https://www.cnblogs.com/emanlee/p/6100019.html) (本版1.1.0或以上 openssl升级后编译时出现问题看[这里](https://ask.helplib.com/python-2.7/post_12311885))。
- [libwebsockets.so.10](https://libwebsockets.org/)
- [libjansson.so.4](https://github.com/akheron/jansson)
- [libusrsctp.so.1](https://github.com/sctplab/usrsctp)
- [libevent-2.0.so.5](https://github.com/libevent/libevent)
- [libglib-2.0.so.0](https://github.com/GNOME/glib ) 或直接安装sudo apt-get install libglib2.0-dev


### 编译与测试
- 首先下载[PTP_FogConnect.zip](https://km.webrtc.win/index.php?share/file&user=102&sid=RTeDzJWE)
- 解压后将看到mips, x86, include3个文件夹和一个readme.md文件.
- 在解压的目录下执行make命令即可，并分部生成test_server,test_client两个可执行文件，在不同平台目录下。
- 将test_server放在相关平台上先执行无需参数，然后在启动test_client也无需参数。
- 执行结果是test_client返送一个 "I ma fog data!", test_server将答复一个"I know!"(需要更高级的功能可以根据自己的情况添加和修改)。
- DEMO中默认为QUIC协议 (可以自行修改)。

### PTP-FogConnet协议文档
https://km.webrtc.win/index.php?share/file&user=102&sid=hmXV7SpC


