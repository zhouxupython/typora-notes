[原创                   Linux内核分析 - 网络[十六\]：TCP三次握手                       ](https://blog.csdn.net/qy532846454/article/details/7882819)

[TCP-IP详解：TCP的头部与选项](https://blog.csdn.net/wdscq1234/article/details/52423272)

[TCP-IP详解：TCP半打开连接及同时打开同时关闭](https://blog.csdn.net/wdscq1234/article/details/52422657)

[TCP状态转换图总结](https://zhuanlan.zhihu.com/p/78540103)

[浅析TCP协议---转载](https://www.cnblogs.com/codestack/p/13563432.html)



[TCP->IP输出 之 ip_queue_xmit、ip_build_and_send_pkt、ip_send_unicast_reply](https://www.cnblogs.com/wanpengcoder/p/11755349.html)

ip_queue_xmit是ip层提供给tcp层发送回调，大多数tcp发送都会使用这个回调，tcp层使用tcp_transmit_skb封装了tcp头之后，调用该函数，该函数提供了路由查找校验、封装ip头和ip选项的功能，封装完成之后调用ip_local_out发送数据包；

ip_build_and_send_pkt函数是服务器端在给客户端回复==syn+ack==时调用的，该函数在构造ip头之后，调用ip_local_out发送数据包；

ip_send_unicast_reply函数目前只用于发送==ACK==和==RST==，该函数根据对端发过来的skb构造ip头，然后调用ip_append_data向发送队列中附加/新增数据，最后调用

ip_push_pending_frames发送数据包；

```c

```





[原创                   tcp传输控制协议-报文格式                       ](https://blog.csdn.net/hzj_001/article/details/81542704)

[原创                   TCP连接的建立与终止                       ](https://blog.csdn.net/hzj_001/article/details/81542647)



[TCP/IP源码分析](https://blog.csdn.net/hzj_001/article/details/100063579) [合集]

[tcp/ip发送接收总体框架](https://blog.csdn.net/hzj_001/article/details/81542483)



[TCP->IP输出 之 ip_queue_xmit、ip_build_and_send_pkt、ip_send_unicast_reply](https://www.cnblogs.com/wanpengcoder/p/11755349.html)

[TCP输出 之 tcp_transmit_skb](https://www.cnblogs.com/wanpengcoder/p/11755347.html)

[TCP输出 之 tcp_write_xmit](https://www.cnblogs.com/wanpengcoder/p/11752189.html)

。。。。https://www.cnblogs.com/wanpengcoder/category/1691162.html?page=2，3，4 【合集】





[聊一聊 tcp拥塞控制] https://www.cnblogs.com/codestack/category/1494258.html【合集】

[tcp ip 三次握手时数据结构-](https://www.cnblogs.com/codestack/p/14883810.html)

[linux tcp/ip 参数解析](https://www.cnblogs.com/codestack/p/11151950.html)



```
-> igb_msix_ring          中断服务函数（分队列处理）

-> napi_schedule -> napi_schedule_prep        
   检测napi->state, NAPI_STATE_SCHED是否置位允许napi调度
-> __napi_schedule -> ____napi_schedule     
   关闭硬件中断，并将该napi->poll_list添加到全局轮询队列poll_list
-> __raise_softirq_irqoff(NET_RX_SOFTIRQ)     产生napi软件中断


-> net_rx_action                             软中断服务函数
   只要全局poll_list队列不为空，则一直轮询处理
   当轮询完成预设目标任务budget，或者2秒轮询超时后强制退出则结束napi轮询，重新使能中断
-> n->poll -> igb_poll 回调网卡轮询处理接口


-> igb_clean_rx_irq
    -> igb_alloc_rx_buffers    
       判断回收rx_buffer超过IGB_RX_BUFFER_WRITE(16),一次性补充16个buffer
    -> igb_fetch_rx_buffer      申请skb并将rx_buffer数据page挂接到skb
    -> napi_gro_receive        
       判断网卡是否支持GRO(Generic Segmentation Offload)
       相对应的有TSO(TCP Segmentation Offload)
    -> napi_skb_finish -> netif_receive_skb
       使能CONFIG_RPS(Receive Packet Steering)时
    -> get_rps_cpu              依据skb->hash获取后续传输层协议栈处理target CPU
    -> enqueue_to_backlog -> __skb_queue_tail    
       将各个队列的skb入列到对应处理target CPU的input_pkt_queue
       
       
-> ____napi_schedule 
-> __raise_softirq_irqoff(NET_RX_SOFTIRQ)     产生sd->backlog软件中断
-> net_rx_action                软中断服务函数
-> n->poll -> process_backlog    回调backlog的轮询处理函数
    -> __skb_dequeue            从队列中出列待处理skb
    -> __netif_receive_skb -> __netif_receive_skb_core     
       开始处理网络层skb    
       根据注册skb->protocol搜索pt_prev /* Protocol hook */
    -> deliver_skb -> pt_prev->func
    
-> ip_rcv               IP层数据处理，net/ipv4/af_inet.c注册
   若注册pf_ring的packet_type，则调用pfring rcv
   prot_hook.func = packet_rcv;
   prot_hook.type = htons(ETH_P_ALL);
-> ip_rcv_finish -> dst_input /* Input packet from network to transport */
-> skb_dst(skb)->input(skb) 
-> ip_local_deliver -> ip_defrag /* Reassemble IP fragments. */
-> ip_local_deliver_finish 
   根据注册inet_protos搜索ipprot
   tcp_protocol, udp_protocol, icmp_protocol, igmp_protocol等
   在net/ipv4/af_inet.c，inet_init注册，以tcp为例
-> tcp/udp层 -> ipprot->handler


-> tcp_v4_rcv(udp_rcv) -> tcp_v4_do_rcv 
-> tcp_rcv_state_process 
-> case TCP_ESTABLISHED: tcp_data_queue 
-> tcp_queue_rcv 
-> __skb_queue_tail /* queue a buffer at the list tail */


-> napi_complete            结束轮询
-> igb_ring_irq_enable        重新使能硬件中断


应用层send -> copy_from_user 
-> 内核层skb -> 协议栈 
-> 分队列
-> ndo_start_xmit -> igb_xmit_frame
-> igb_xmit_frame_ring 
    -> igb_tx_queue_mapping     根据skb->queue_mapping获取对应发送队列tx_ring
    -> igb_xmit_frame_ring      skb填充tx_ring->tx_buffer_info
    -> igb_tso                  TSO(TCP Segmentation Offload)
    -> igb_tx_map                
    将skb流式映射dma_map_single填充描述符tx_desc
    并在最后一帧的描述符tx_desc中将EOP(End of Packet)置位
    

    
-> igb_msix_ring                            中断服务函数（分队列处理）
-> napi_schedule -> napi_schedule_prep        
   检测napi->state, NAPI_STATE_SCHED是否置位允许napi调度
-> __napi_schedule -> ____napi_schedule     
   关闭硬件中断，并将该napi->poll_list添加到全局轮询队列poll_list
-> __raise_softirq_irqoff(NET_RX_SOFTIRQ)   产生napi软件中断
-> net_rx_action                            软中断服务函数
   只要全局poll_list队列不为空，则一直轮询处理
   当轮询完成预设目标任务budget，或者2秒轮询超时后强制退出则结束napi轮询，重新使能中断
-> n->poll -> igb_poll                      回调网卡轮询处理接口       
    -> igb_clean_tx_irq                     释放skb，dma_unmap_single
```

