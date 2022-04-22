

## __netif_receive_skb_core

[网络收包流程-收包函数__netif_receive_skb的核心函数__netif_receive_skb_core（三）](https://blog.csdn.net/hzj_001/article/details/104327771)

了解此函数，首先需要知道ptype_base和ptype_all变量，这二个都是list_head变量，list_head 链表上挂了很多packet_type数据结构，此结构体是对应于具体协议的实例，二个结变量的定义如下：

```c
// include/linux/netdevice.h
/*
 *	The list of packet types we will receive (as opposed to discard)
 *	and the routines to invoke.
 *
 *	Why 16. Because with 16 the only overlap we get on a hash of the
 *	low nibble of the protocol value is RARP/SNAP/X.25.
 *
 *		0800	IP
 *		0001	802.3
 *		0002	AX.25
 *		0004	802.2
 *		8035	RARP
 *		0005	SNAP
 *		0805	X.25
 *		0806	ARP
 *		8137	IPX
 *		0009	Localtalk
 *		86DD	IPv6
 */
#define PTYPE_HASH_SIZE	(16)
#define PTYPE_HASH_MASK	(PTYPE_HASH_SIZE - 1)

extern struct list_head ptype_all __read_mostly;
extern struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly;

// "net/core/dev.c"
struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly; //PTYPE_HASH_SIZE路的hash链表
struct list_head ptype_all __read_mostly;    /* Taps 双向链表 */

struct packet_type {
    __be16          type;   /*  type指定了协议的标识符，处理程序func会使用该标识符 ，保存了三层协议类型，ETH_P_IP、ETH_P_ARP等等 */
    struct net_device   *dev;   /* NULL指针表示该处理程序对系统中所有网络设备都有效      */
/* func是该结构的主要成员。它是一个指向网络层函数的指针，如果分组的类型适当，将其传递给该函数。其中可能的处理程序就是ip_rcv */
    int         (*func) (struct sk_buff *,
                     struct net_device *,
                     struct packet_type *,
                     struct net_device *);
    bool            (*id_match)(struct packet_type *ptype,
                        struct sock *sk);
    void            *af_packet_priv;
    struct list_head    list;
};

// net/ipv4/af_inet.c
// dev_add_pack(&ip_packet_type);
static struct packet_type ip_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_IP),//#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
	.func = ip_rcv,
	.list_func = ip_list_rcv,
};

static int __init inet_init(void)
{
    rc = proto_register(&tcp_prot, 1);
	rc = proto_register(&udp_prot, 1);
	rc = proto_register(&raw_prot, 1);
	rc = proto_register(&ping_prot, 1);

	if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0)//Add all the base protocols.
	if (inet_add_protocol(&udp_protocol, IPPROTO_UDP) < 0)
	if (inet_add_protocol(&tcp_protocol, IPPROTO_TCP) < 0)

	(void)sock_register(&inet_family_ops);//Tell SOCKET that we are alive...

	arp_init();//Set the ARP module up
	ip_init();//Set the IP module up
	tcp_init();/* Setup TCP slab cache for open requests. */
	udp_init();/* Setup UDP memory threshold */
	udplite4_register();/* Add UDP-Lite (RFC 3828) */
	raw_init();
	ping_init();
	icmp_init()
    
	ipv4_proc_init();
	ipfrag_init();

	dev_add_pack(&ip_packet_type);
    
}

// net/core/dev.c
/*
 *	Add a protocol ID to the list. Now that the input handler is
 *	smarter we can dispense with all the messy stuff that used to be
 *	here.
 *
 *	BEWARE!!! Protocol handlers, mangling input packets,
 *	MUST BE last in hash buckets and checking protocol handlers
 *	MUST start from promiscuous ptype_all chain in net_bh.
 *	It is true now, do not change it.
 *	Explanation follows: if protocol handler, mangling packet, will
 *	be the first on list, it is not able to sense, that packet
 *	is cloned and should be copied-on-write, so that it will
 *	change it and subsequent readers will get broken packet.
 *							--ANK (980803)
 */
static inline struct list_head *ptype_head(const struct packet_type *pt)
{
    //type为ETH_P_ALL时，则挂在ptype_all上面
    //	#define ETH_P_ALL	0x0003		/* Every packet (be careful!!!) */
	if (pt->type == htons(ETH_P_ALL))
		return pt->dev ? &pt->dev->ptype_all : &ptype_all;
	else// //否则，挂在ptype_base[type&15]上面，即对应协议的list_head上面。
		return pt->dev ? &pt->dev->ptype_specific :
				 &ptype_base[ntohs(pt->type) & PTYPE_HASH_MASK];
}

/**
 *	dev_add_pack - add packet handler
 *	@pt: packet type declaration
 *
 *	Add a protocol handler to the networking stack. The passed &packet_type
 *	is linked into kernel lists and may not be freed until it has been
 *	removed from the kernel lists.
 *
 *	This call does not sleep therefore it can not
 *	guarantee all CPU's that are in middle of receiving packets
 *	will see the new packet type (until the next received packet).
 注册packet_type的api主要在相应的协议初始化时，通过dev_add_pack函数实现，把packet_type结构挂在对应协议的的list_head上面，移除则采用dev_remove_pack。
 */

void dev_add_pack(struct packet_type *pt)
{
	struct list_head *head = ptype_head(pt);//通过函数ptype_head获取对应协议的list_head

	spin_lock(&ptype_lock);
	list_add_rcu(&pt->list, head);
	spin_unlock(&ptype_lock);
}
```



```c
static int __netif_receive_skb_core(struct sk_buff *skb, bool pfmemalloc) // 将skb传递到上层 
{
	struct packet_type *ptype, *pt_prev;
	rx_handler_func_t *rx_handler;
	struct net_device *orig_dev;
	struct net_device *null_or_dev;
	bool deliver_exact = false;//默认不精确传递
	int ret = NET_RX_DROP;//默认收报失败
	__be16 type;
 
	net_timestamp_check(!netdev_tstamp_prequeue, skb);//记录收包时间，netdev_tstamp_prequeue为0，表示可能有包延迟 
 
	trace_netif_receive_skb(skb);
 
	orig_dev = skb->dev;//记录收包设备 
 
	skb_reset_network_header(skb);//重置network header，此时skb指向IP头（没有vlan的情况下）
	if (!skb_transport_header_was_set(skb))
		skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
  
    // 留下一个节点，最后一次向上层传递时，不需要再inc引用，回调中会free，这样相当于少调用了一次free
	pt_prev = NULL;
 
another_round:
	skb->skb_iif = skb->dev->ifindex;//设置接收设备索引号 
 
	__this_cpu_inc(softnet_data.processed);//处理包数统计 
 
	if (skb->protocol == cpu_to_be16(ETH_P_8021Q) ||
	    skb->protocol == cpu_to_be16(ETH_P_8021AD)) {//vxlan报文处理，剥除vxlan头
		skb = skb_vlan_untag(skb);//剥除vxlan头
		if (unlikely(!skb))
			goto out;
	}
 
#ifdef CONFIG_NET_CLS_ACT
	if (skb->tc_verd & TC_NCLS) {
		skb->tc_verd = CLR_TC_NCLS(skb->tc_verd);
		goto ncls;
	}
#endif
 
	if (pfmemalloc)//此类报文不允许ptype_all处理，即tcpdump也抓不到
		goto skip_taps;
    
    //先处理 ptype_all 上所有的 packet_type->func()           
    //所有包都会调func，对性能影响严重！所有的钩子是随模块加载挂上的。
	list_for_each_entry_rcu(ptype, &ptype_all, list) {//遍历ptye_all链表
		if (!ptype->dev || ptype->dev == skb->dev) {//上面的paket_type.type 为 ETH_P_ALL，典型场景就是tcpdump抓包所使用的协议
			if (pt_prev)//pt_prev提高效率
				ret = deliver_skb(skb, pt_prev, orig_dev);//此函数最终调用paket_type.func()
			pt_prev = ptype;
		}	
	}
    // 这里还有 skb->dev->ptype_all的遍历
 
skip_taps:
#ifdef CONFIG_NET_CLS_ACT
	if (static_key_false(&ingress_needed)) {
		skb = handle_ing(skb, &pt_prev, &ret, orig_dev);
		if (!skb)
			goto out;
	}
 
	skb->tc_verd = 0;
ncls:
#endif
	if (pfmemalloc && !skb_pfmemalloc_protocol(skb))//不支持使用pfmemalloc 
		goto drop;
 
	if (skb_vlan_tag_present(skb)) {// 如果是vlan包 
		if (pt_prev) {/* 处理pt_prev */
			ret = deliver_skb(skb, pt_prev, orig_dev);
			pt_prev = NULL;
		}
		if (vlan_do_receive(&skb))/* 根据实际的vlan设备调整信息，再走一遍 */
			goto another_round;
		else if (unlikely(!skb))
			goto out;
	}
/*如果一个dev被添加到一个bridge（做为bridge的一个接口)，这个接口设备的rx_handler将被设置为br_handle_frame函数，这是在br_add_if函数中设置的，而br_add_if (net/bridge/br_if.c)是在向网桥设备上添加接口时设置的。进入br_handle_frame也就进入了bridge的逻辑代码。*/
	rx_handler = rcu_dereference(skb->dev->rx_handler);/* 如果有注册handler，那么调用，比如网桥模块 */
	if (rx_handler) {
		if (pt_prev) {
			ret = deliver_skb(skb, pt_prev, orig_dev);
			pt_prev = NULL;
		}
		switch (rx_handler(&skb)) {
		case RX_HANDLER_CONSUMED:/* 已处理，无需进一步处理 */
			ret = NET_RX_SUCCESS;
			goto out;
		case RX_HANDLER_ANOTHER:/* 修改了skb->dev，在处理一次 */
			goto another_round;
		case RX_HANDLER_EXACT:/* 精确传递到ptype->dev == skb->dev */
			deliver_exact = true;
		case RX_HANDLER_PASS:
			break;
		default:
			BUG();
		}
	}
 
	if (unlikely(skb_vlan_tag_present(skb))) {/* 还有vlan标记，说明找不到vlanid对应的设备 */
		if (skb_vlan_tag_get_id(skb))/* 存在vlanid，则判定是到其他设备的包 */
			skb->pkt_type = PACKET_OTHERHOST;
		/* Note: we might in the future use prio bits
		 * and set skb->priority like in vlan_do_receive()
		 * For the time being, just ignore Priority Code Point
		 */
		skb->vlan_tci = 0;
	}
 
	/* deliver only exact match when indicated */
	null_or_dev = deliver_exact ? skb->dev : NULL;//指定精确传递的话，就精确传递,否则向未指定设备的指定协议全局发送一份
 
	type = skb->protocol;/* 设置三层协议，下面提交都是按照三层协议提交的 */
	list_for_each_entry_rcu(ptype,&ptype_base[ntohs(type) & PTYPE_HASH_MASK], list) {
		if (ptype->type == type &&
		    (ptype->dev == null_or_dev || ptype->dev == skb->dev ||
		     ptype->dev == orig_dev)) {
			if (pt_prev)
				ret = deliver_skb(skb, pt_prev, orig_dev);//上层传递，其中之一会触发ip_packet_type的回调函数，ip_rcv
			pt_prev = ptype;
		}
	}
	if (pt_prev) {
		if (unlikely(skb_orphan_frags(skb, GFP_ATOMIC)))
			goto drop;
		else
                //使用pt_prev这里就不需要deliver_skb来inc应用数了,  func执行内部会free，减少了一次skb_free
 
        
			ret = pt_prev->func(skb, skb->dev, pt_prev, orig_dev);/* 传递到上层*/
	} else {
drop:
		if (!deliver_exact)
			atomic_long_inc(&skb->dev->rx_dropped);//网卡丢包计数
		else
			atomic_long_inc(&skb->dev->rx_nohandler);
		kfree_skb(skb);
		/* Jamal, now you will not able to escape explaining
		 * me how you were going to use this. :-)
		 */
		ret = NET_RX_DROP;
	}
out:
	return ret;
}

static inline int deliver_skb(struct sk_buff *skb,
			      struct packet_type *pt_prev,
			      struct net_device *orig_dev)
{
	if (unlikely(skb_orphan_frags_rx(skb, GFP_ATOMIC)))
		return -ENOMEM;
	refcount_inc(&skb->users);
	return pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
}
```

# [TCP/IP 协议栈在 Linux 内核中的运行时序分析](https://www.cnblogs.com/luoyang-/p/14349884.html)

# [Linux 内核网络栈分析: 接收数据](https://lizhaolong.blog.csdn.net/article/details/123517286)



[tcp/ip发送接收总体框架](https://blog.csdn.net/hzj_001/article/details/81542483)

[linux网卡报文接收流程](https://blog.csdn.net/hhhhhyyyyy8/article/details/102492680)

[原创                   Linux内核分析 - 网络[二\]：网卡驱动接收报文                       ](https://blog.csdn.net/qy532846454/article/details/6288261)

[原创                   Linux内核分析 - 网络[三\]：从netif_receive_skb()说起                       ](https://blog.csdn.net/qy532846454/article/details/6339789)



[原创                   Linux-4.20.8内核桥收包源码解析（三）----------网卡驱动收包                       ](https://blog.csdn.net/Sophisticated_/article/details/87295513)

[原创                   Linux-4.20.8内核桥收包源码解析（四）----------netif_receive_skb                       ](https://blog.csdn.net/Sophisticated_/article/details/87805798)

[原创                   Linux-4.20.8内核桥收包源码解析（五）----------桥处理流程br_handle_frame                       ](https://blog.csdn.net/Sophisticated_/article/details/87878460)

[原创                   Linux-4.20.8内核桥收包源码解析（六）----------决策函数br_handle_frame_finish                       ](https://blog.csdn.net/Sophisticated_/article/details/87922712)

[原创                   Linux-4.20.8内核桥收包源码解析（七）----------本地（br_pass_frame_up）or 转发（br_forward）                       ](https://blog.csdn.net/Sophisticated_/article/details/87923362)



[原创                   网络收包流程-报文从网卡驱动到网络层（或者网桥)的流程（非NAPI、NAPI）(一)                       ](https://blog.csdn.net/hzj_001/article/details/100085112)

[原创                   网络收包流程-软中断中process_backlog和poll方式处理流程（二）                       ](https://blog.csdn.net/hzj_001/article/details/100708621)

[原创                   网络收包流程-收包函数__netif_receive_skb的核心函数__netif_receive_skb_core（三）                       ](https://blog.csdn.net/hzj_001/article/details/104327771)

[原创                   网路收包流程-网桥的处理流程（br网桥）（四）                       ](https://blog.csdn.net/hzj_001/article/details/104328321)

[原创                   网路收报流程-网桥的处理流程（br网桥）（四）                       ](https://blog.csdn.net/hzj_001/article/details/104327900)

[原创                   网络收包流程-网络层处理流程ip_rcv（五）                       ](https://blog.csdn.net/hzj_001/article/details/104950605)



[设备收发包之netif_receive_skb](https://www.cnblogs.com/wanpengcoder/p/7577088.html)



[原创                   网络丢包排查思路                       ](https://blog.csdn.net/hzj_001/article/details/104950713)

[话说网卡收包内存的块](https://www.cnblogs.com/codestack/p/14278373.html)

[ip_rcv 中使用skb_share_check](https://www.cnblogs.com/codestack/p/13441135.html)
