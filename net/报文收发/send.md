[Linux*内核网络*udp数据包发送（一）                                ](https://blog.51cto.com/u_15155099/2767255)

[Linux*内核网络*udp数据包发送（二）——UDP协议层分析                                ](https://blog.51cto.com/u_15155099/2767256)

[Linux*内核网络*UDP数据包发送（三）——IP协议层分析                                ](https://blog.51cto.com/u_15155099/2767257)

[Linux*内核网络*UDP数据包发送（四）——Linux netdevice 子系统                                ](https://blog.51cto.com/u_15155099/2767259)

[Linux*内核网络*UDP数据包发送（五）——排队规则                                ](https://blog.51cto.com/u_15155099/2767262)



[[译] Linux 网络栈监控和调优：发送数据（2017）](http://arthurchiao.art/blog/tuning-stack-tx-zh/)

[TCP/IP 协议栈在 Linux 内核中的运行时序分析](https://www.cnblogs.com/luoyang-/p/14349884.html)



[从udp_sendmsg到ip_output发包过程](https://blog.csdn.net/hhhhhyyyyy8/article/details/106589844)

```c
udp_sendmsg->udp_send_skb->ip_send_skb->ip_local_out->ip_local_out_sk->__ip_local_out->__ip_local_out_sk->dst_output_sk->ip_output
```



[原创                   Linux内核数据包的发送传输](https://blog.csdn.net/asiainfolf/article/details/10252857)

[原创                   Linux内核二层数据包接收流程 ](https://blog.csdn.net/asiainfolf/article/details/10286593)

[原创                   Linux内核对三层协议的管理 ](https://blog.csdn.net/asiainfolf/article/details/10296673)

[原创                   Linux 二层协议架构组织](https://blog.csdn.net/asiainfolf/article/details/10475583)

[原创                   Linux内核IP层的报文处理流程--从网卡接收的报文处理流程 ](https://blog.csdn.net/asiainfolf/article/details/10789811)





[TCP->IP输出 之 ip_queue_xmit、ip_build_and_send_pkt、ip_send_unicast_reply](https://www.cnblogs.com/wanpengcoder/p/11755349.html)

ip_queue_xmit是ip层提供给tcp层发送回调，大多数tcp发送都会使用这个回调，tcp层使用tcp_transmit_skb封装了tcp头之后，调用该函数，该函数提供了路由查找校验、封装ip头和ip选项的功能，封装完成之后调用ip_local_out发送数据包；

ip_build_and_send_pkt函数是服务器端在给客户端回复syn+ack时调用的，该函数在构造ip头之后，调用ip_local_out发送数据包；

ip_send_unicast_reply函数目前只用于发送ACK和RST，该函数根据对端发过来的skb构造ip头，然后调用ip_append_data向发送队列中附加/新增数据，最后调用ip_push_pending_frames发送数据包；



[IP 层收发报文简要剖析4--ip 报文发送](https://www.cnblogs.com/codestack/p/9195001.html)

[IP 层收发报文简要剖析5--ip报文发送2](https://www.cnblogs.com/codestack/p/9201813.html)

[IP 层收发报文简要剖析6--ip报文输出3 ip_push_pending_frames](https://www.cnblogs.com/codestack/p/9265886.html)

[dst_output发包](https://www.cnblogs.com/codestack/p/9292122.html)
