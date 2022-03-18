[Linux*内核网络*udp数据包发送（一）                                ](https://blog.51cto.com/u_15155099/2767255)

[Linux*内核网络*udp数据包发送（二）——UDP协议层分析                                ](https://blog.51cto.com/u_15155099/2767256)

[Linux*内核网络*UDP数据包发送（三）——IP协议层分析                                ](https://blog.51cto.com/u_15155099/2767257)

[Linux*内核网络*UDP数据包发送（四）——Linux netdevice 子系统                                ](https://blog.51cto.com/u_15155099/2767259)

[Linux*内核网络*UDP数据包发送（五）——排队规则                                ](https://blog.51cto.com/u_15155099/2767262)



[从udp_sendmsg到ip_output发包过程](https://blog.csdn.net/hhhhhyyyyy8/article/details/106589844)

```c
udp_sendmsg->udp_send_skb->ip_send_skb->ip_local_out->ip_local_out_sk->__ip_local_out->__ip_local_out_sk->dst_output_sk->ip_output
```



[原创                   Linux内核数据包的发送传输](https://blog.csdn.net/asiainfolf/article/details/10252857)

[原创                   Linux内核二层数据包接收流程 ](https://blog.csdn.net/asiainfolf/article/details/10286593)

[原创                   Linux内核对三层协议的管理 ](https://blog.csdn.net/asiainfolf/article/details/10296673)

[原创                   Linux 二层协议架构组织](https://blog.csdn.net/asiainfolf/article/details/10475583)

[原创                   Linux内核IP层的报文处理流程--从网卡接收的报文处理流程 ](https://blog.csdn.net/asiainfolf/article/details/10789811)

