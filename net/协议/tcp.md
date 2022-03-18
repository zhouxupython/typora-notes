[原创                   Linux内核分析 - 网络[十六\]：TCP三次握手                       ](https://blog.csdn.net/qy532846454/article/details/7882819)

[TCP-IP详解：TCP的头部与选项](https://blog.csdn.net/wdscq1234/article/details/52423272)

[TCP-IP详解：TCP半打开连接及同时打开同时关闭](https://blog.csdn.net/wdscq1234/article/details/52422657)

[TCP状态转换图总结](https://zhuanlan.zhihu.com/p/78540103)



[TCP->IP输出 之 ip_queue_xmit、ip_build_and_send_pkt、ip_send_unicast_reply](https://www.cnblogs.com/wanpengcoder/p/11755349.html)

ip_queue_xmit是ip层提供给tcp层发送回调，大多数tcp发送都会使用这个回调，tcp层使用tcp_transmit_skb封装了tcp头之后，调用该函数，该函数提供了路由查找校验、封装ip头和ip选项的功能，封装完成之后调用ip_local_out发送数据包；

ip_build_and_send_pkt函数是服务器端在给客户端回复==syn+ack==时调用的，该函数在构造ip头之后，调用ip_local_out发送数据包；

ip_send_unicast_reply函数目前只用于发送==ACK==和==RST==，该函数根据对端发过来的skb构造ip头，然后调用ip_append_data向发送队列中附加/新增数据，最后调用

ip_push_pending_frames发送数据包；

```c

```

