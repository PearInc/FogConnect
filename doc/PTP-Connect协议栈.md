## PTP-Connect协议栈


> PTP-FogConnet协议栈如下：
![PTP-FogConnetStack.png](https://km.webrtc.win/index.php?user/publicLink&fid=e84buJRaKZm6fKwa3pvK089ue2T211VaYyr7Qxx6_fvCVA2tGNl4pzLEZG8w5rNK__tiQPbxWI-MmOZ2Taem8EcaePbUreOrk3H-lAwUUw39TRPfSyNxUzKFaTp-o7U94ojYEd1j2jW8EK9XM3YO6eCZoreO7y2S4WDfeDM0iN0a&file_name=/PTP-FogConnetStack.png)
> 其中通过proxy层的PTP-FogConnet协议栈图：
![PTP-FogConnetStack-Proxy.png](https://km.webrtc.win/index.php?user/publicLink&fid=4ef4aveWZm8-KoXMd6yJrUzp2XBtDelQqwL6L0oWD8QAMtpvfpq5LJPpDKEX4QsEFPgMI5U74ZYZg661LuDGFzjia8hm6UoHBUniUFjdBhl1S5lNkbfLhx3oXE9e6sOgLPr-Yn3f6h4QW04iVwip1qcGRSTT37wHrBYJcmpH9WjE2YD_P1D2&file_name=/PTP-FogConnetStack-Proxy.png)

## 接入PTP-FogConnet后的流程图如下：
> ![flowchart.png](https://km.webrtc.win/index.php?user/publicLink&fid=05a7FHWVRvXYLubQORlXdCIg9kvEDcttT_DB46bd_n0vj6F7H3E2Xh3MOloluAivIWxVouLMNclPSuCOPRHFR8GtWUvv2vOGwa1foBlFE8wU8Yk5coh0ofhjEERU1vN-5tYPe-vue5_RuD_IEZufL79KzYMqd70X&file_name=/flowchart.png)


## 以下为AIP使用说明:
> 根据上面的流程图，可以知道接入时就是重写几个回调函数，实际上是3个回调2个操作1个保存,具体回调函数和操作说明如下：
> 第一个回调：节点主动连接或被连接,当连接上时应用层来决定怎么处理这个连接动作(图中回调函数名为pr_connect_callback)。
![pr_connect_callback.png](https://km.webrtc.win/index.php?user/publicLink&fid=b978Ky_ICm-bUmc80lIB2DY1kecSPi93O1gih22gRfGf2f9IfFoUnR1xJ_q3_UfK3d3tahnsdPupBSzuOuRdwqYZfCl1hFd7a8RchsHPCSxOukYHaWooBkSuhngJFVEAdpRWAbCvmSRwvgcpQSL7VlX7jFGPs7eKUiOingjFr7kRqw&file_name=/pr_connect_callback.png)
> 第二个回调：在建立了P2P连接后，在pr_connect_callback被调用时来注册这个连接获得数据后，应用层对数据的处理（图中回调函数名为pr_recv_data）
![pr_recv_data.png](https://km.webrtc.win/index.php?explorer/fileProxy&accessToken=b54e75LAfoUwpRXpH33YZFniJvjFjTvmE3e_3MTQ0BhE2_W0h6Ay6uyvIeqRrtTZEH1Gf9jwSA&path=%2Fdev%2F%E5%BC%80%E5%8F%91%E7%BB%8F%E9%AA%8C%E6%80%BB%E7%BB%93%2F%E7%BB%88%E7%AB%AF%2FLiveoVideoStack%2Fpr_recv_data.png)
> 第三个回调：在建立了P2P连接后，在pr_connect_callback被调用时来注册这个连接断开和超时后，应用层对这个事件的处理（图中回调函数名为pr_recv_data）
![pr_close_connect.png](https://km.webrtc.win/index.php?explorer/fileProxy&accessToken=b54e75LAfoUwpRXpH33YZFniJvjFjTvmE3e_3MTQ0BhE2_W0h6Ay6uyvIeqRrtTZEH1Gf9jwSA&path=%2Fdev%2F%E5%BC%80%E5%8F%91%E7%BB%8F%E9%AA%8C%E6%80%BB%E7%BB%93%2F%E7%BB%88%E7%AB%AF%2FLiveoVideoStack%2Fpr_close_connect.png)
> 操作一：应用层私有数据与连接对象绑定过程（被动连接的情况下）
![bing.png](https://km.webrtc.win/index.php?user/publicLink&fid=911ab0WWSEZKG2otCS_xeu07rP5RImtWgPP6JBfVUE91XPKssgPj2_xE3JU0OqbI7ouv0dT4Zl1PuQ4HTJYhSM7SkPVTQ7s9vPWrdKPGM4k0tfFCa1I9yyW9lpzXX1v2BvKV04ghkfKTG8uclCKH20p1dw&file_name=/bing.png)
> 操作二：主要是双方约定最终这个连接使用应层的那个服务。
![convention.png](https://km.webrtc.win/index.php?user/publicLink&fid=05c1Bzcw6p3z6K3NBhfYC_JZge70ikekvl6r8ivQqcVs5r7u5OYpKnSZgmz-GcdUyLNBCSK1fdAXc8yyg8YgAoOvWt7ZBBOxyODSbU_I-yvjxEAwCPVzSSO4HbMN9CNJsiOqpf-015wd-KTDa8sv9EXXqs5Pgpuiaw&file_name=/convention.png)
>保存：应用层保存连接对象（void* pr_connect)
![save.png](https://km.webrtc.win/index.php?user/publicLink&fid=f0839u-GF8K9SjWA7PTnkOl21V8d9hbU3v2oSnXxJf52BnDWYCR3SsXfjIZxqZRoh2ZGIirSIx2dca9RLFIiCAekU9LC6lDTSZc7QPSOCSLZ7tWgBtHuPK5z67CpoW3TFiN5NsWkCUKpQAz524anISnbHw&file_name=/save.png)

## 相关DEMO源码：
> 每个不同平台下的pr_server.c pr_client.c
> intclude目录下的fogconnect.h