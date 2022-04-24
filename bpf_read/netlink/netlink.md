# netlink

## 资料

[linux的netlink接口详解(上)](https://blog.csdn.net/banruoju/article/details/69387232)

[linux的netlink接口详解（中）](https://blog.csdn.net/banruoju/article/details/73635994)

[linux的netlink接口详解（下）](https://blog.csdn.net/banruoju/article/details/77833538)

[Netlink 内核实现分析 1](https://www.cnblogs.com/codestack/p/10849427.html)

[Netlink 内核实现分析 2](https://www.cnblogs.com/codestack/p/10849706.html)

[Netlink 内核实现分析 3](https://www.cnblogs.com/codestack/p/10850184.html)

[Netlink 内核实现分析 4](https://www.cnblogs.com/codestack/p/10850608.html)



## NETLINK_ROUTE回调函数的初始化

[Netlink实现Linux内核与用户空间通信](http://www.cpplive.com/html/1362.html)

[Linux-内核通信之netlink机制-详解](https://blog.csdn.net/sty23122555/article/details/51581979)

```c
netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)		// include/linux/netlink.h
    __netlink_kernel_create			net/netlink/af_netlink.c
    
    
/* optional Netlink kernel configuration parameters */
struct netlink_kernel_cfg {
	unsigned int	groups;
	unsigned int	flags;
	void		(*input)(struct sk_buff *skb);
	struct mutex	*cb_mutex;
	int		(*bind)(struct net *net, int group);
	void		(*unbind)(struct net *net, int group);
	bool		(*compare)(struct net *net, struct sock *sk);
};

//net/core/rtnetlink.c
static int __net_init rtnetlink_net_init(struct net *net)
{
	struct sock *sk;
	struct netlink_kernel_cfg cfg = {
		.groups		= RTNLGRP_MAX,
		.input		= rtnetlink_rcv,// 收到用户态NETLINK_ROUTE协议的数据后，触发这个	// step-5
		.cb_mutex	= &rtnl_mutex,
		.flags		= NL_CFG_F_NONROOT_RECV,
		.bind		= rtnetlink_bind,
	};

	sk = netlink_kernel_create(net, NETLINK_ROUTE, &cfg);
	if (!sk)
		return -ENOMEM;
	net->rtnl = sk;
	return 0;
}

static void __net_exit rtnetlink_net_exit(struct net *net)
{
	netlink_kernel_release(net->rtnl);
	net->rtnl = NULL;
}

static struct pernet_operations rtnetlink_net_ops = {// rtnetlink_init中注册  @@@-2
	.init = rtnetlink_net_init,			// step-4
	.exit = rtnetlink_net_exit,
};

// netlink_proto_init中调用
void __init rtnetlink_init(void)//@@@-1
{
    register_pernet_subsys(&rtnetlink_net_ops);//@@@-2		// step-3
	register_netdevice_notifier(&rtnetlink_dev_notifier);

	rtnl_register(PF_UNSPEC, RTM_GETLINK, rtnl_getlink, rtnl_dump_ifinfo, 0);
	rtnl_register(PF_UNSPEC, RTM_SETLINK, rtnl_setlink, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_NEWLINK, rtnl_newlink, NULL, 0);//
	rtnl_register(PF_UNSPEC, RTM_DELLINK, rtnl_dellink, NULL, 0);


static int __init netlink_proto_init(void)//net/netlink/af_netlink.c
{
    
    sock_register(&netlink_family_ops);
	register_pernet_subsys(&netlink_net_ops);//@@@-3
	register_pernet_subsys(&netlink_tap_net_ops);
	/* The netlink device handler may be needed early. */
	rtnetlink_init();//@@@-1	// step-2
}
core_initcall(netlink_proto_init);// step-1
    
//------------
static struct pernet_operations __net_initdata netlink_net_ops = {//@@@-3
	.init = netlink_net_init,
	.exit = netlink_net_exit,
};

static int __net_init netlink_net_init(struct net *net)
{
#ifdef CONFIG_PROC_FS
	if (!proc_create_net("netlink", 0, net->proc_net, &netlink_seq_ops,
			sizeof(struct nl_seq_iter)))
		return -ENOMEM;
#endif
	return 0;
}
    
static void __net_exit netlink_net_exit(struct net *net)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry("netlink", net->proc_net);
#endif
}
```



## 用户态socket

```c
socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE)
```

第一个参数必须是 `AF_NETLINK`或 `PF_NETLINK`，在 Linux 中，它们俩实际为一个东西，它表示要使用**netlink**，

第二个参数必须是`SOCK_RAW`或`SOCK_DGRAM`，

第三个参数指定netlink协议类型：

```c
#define NETLINK_ROUTE 0 //路由守护进程  @@@
#define NETLINK_W1 1 //1-wire 子系统  
#define NETLINK_USERSOCK 2 //用户态套结字协议  
#define NETLINK_FIREWALL 3 //防火墙  
#define NETLINK_INET_DIAG 4 //套结字监视  
#define NETLINK_NFLOG 5 //网络数据过滤日志  
#define NETLINK_XFRM 6 //ipsec 安全策略  
#define NETLINK_SELINUX 7 //SELinux 事件通知  
#define NETLINK_ISCSI 8 //iSCSI网络存储子系统  
#define NETLINK_AUDIT 9 //进程审计  
#define NETLINK_FIB_LOOKUP 10 //转发信息表查询  
#define NETLINK_CONNECTOR 11 //netlink连接器  
#define NETLINK_NETFILTER 12 //网络数据过滤系统  
#define NETLINK_IP6_FW 13 //IPv6 防火墙  
#define NETLINK_DNRTMSG 14 //DECnet路由信息  
#define NETLINK_KOBJECT_UEVENT 15 //内核事件向用户态通知  
#define NETLINK_GENERIC 16 //通用netlink（用户自定义功能）  
```



### NETLINK_ROUTE

==流量过滤==：RTM_NEWTFILTER, RTM_DELTFILTER, RTM_GETTFILTER

```c
NETLINK_ROUTE提供路由和连接信息。这些信息主要被用户空间的路由守护进程使用。对于这个 协议，Linux声明了大量的子消息：

链路层：RTM_NEWLINK, RTM_DELLINK, RTM_GETLINK, RTM_SETLINK
地址设定：RTM_NEWADDR, RTM_DELADDR, RTM_GETADDR
路由表：RTM_NEWROUTE, RTM_DELROUTE, RTM_GETROUTE
邻居缓存（Neighbor Cache）：RTM_NEWNEIGH, RTM_DELNEIGH, RTM_GETNEIGH
路由规则：RTM_NEWRULE, RTM_DELRULE, RTM_GETRULE
Queuing Discipline Settings: RTM_NEWQDISC, RTM_DELQDISC, RTM_GETQDISC
Traffic Classes used with Queues: RTM_NEWTCLASS, RTM_DELTCLASS, RTM_GETTCLASS
流量过滤：RTM_NEWTFILTER, RTM_DELTFILTER, RTM_GETTFILTER
其它：RTM_NEWACTION, RTM_DELACTION, RTM_GETACTION, RTM_NEWPREFIX, RTM_GETPREFIX, RTM_GETMULTICAST, RTM_GETANYCAST, RTM_NEWNEIGHTBL,RTM_GETNEIGHTBL, RTM_SETNEIGHTBL
```



### 举例

```c
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <sys/socket.h>  
#include <linux/netlink.h>  
#define UEVENT_BUFFER_SIZE 2048  
int main(void)  
{  
    struct sockaddr_nl client;  
    struct timeval tv;  
    int ntSocket, rcvlen, ret;  
    fd_set fds;  
    int buffersize = 1024;  
    ntSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);  // #define AF_NETLINK	PF_NETLINK
    memset(&client, 0, sizeof(client));  
    client.nl_family = AF_NETLINK;  
    client.nl_pid = getpid();  
    client.nl_groups = 1; /* receive broadcast message*/  
    setsockopt(ntSocket, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));  
    bind(ntSocket, (struct sockaddr*)&client, sizeof(client));  
    while (1) {  
        char buf[UEVENT_BUFFER_SIZE] = { 0 };  
        FD_ZERO(&fds);  
        FD_SET(ntSocket, &fds);  
        tv.tv_sec = 0;  
        tv.tv_usec = 100 * 1000;  
        ret = select(ntSocket + 1, &fds, NULL, NULL, &tv);  
        if(!(ret > 0 && FD_ISSET(ntSocket, &fds)))  
            continue;  
        /* receive data */  
        rcvlen = recv(ntSocket, &buf, sizeof(buf), 0);  
        if (rcvlen > 0) {  
            printf("%s\n", buf);  
            // You can do something here to make the program more perfect!!!  
        }  
    }  
    close(ntSocket);  
    return 0;  
}  
```

运行程序，测试U盘插入/拔除，输出如下：

[view plain](http://www.cpplive.com/html/1362.html#)[copy to clipboard](http://www.cpplive.com/html/1362.html#)[print](http://www.cpplive.com/html/1362.html#)[?](http://www.cpplive.com/html/1362.html#)

1. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1 
2. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0 
3. add@/module/usb_storage 
4. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6 
5. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/scsi_host/host6 
6. add@/bus/usb/drivers/usb-storage 
7. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0 
8. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0 
9. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_disk/6:0:0:0 
10. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_device/6:0:0:0 
11. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_generic/sg2 
12. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/bsg/6:0:0:0 
13. change@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0 
14. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/block/sdb 
15. add@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/block/sdb/sdb4 
16. add@/devices/**virtual**/bdi/8:16 
17. add@/module/fat 
18. add@/kernel/slab/fat_cache 
19. add@/kernel/slab/fat_inode_cache 
20. add@/module/vfat 
21. add@/module/nls_cp437 
22. add@/module/nls_iso8859_1 
23. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/bsg/6:0:0:0 
24. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_generic/sg2 
25. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_device/6:0:0:0 
26. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/scsi_disk/6:0:0:0 
27. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/block/sdb/sdb4 
28. remove@/devices/**virtual**/bdi/8:16 
29. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0/block/sdb 
30. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/target6:0:0/6:0:0:0 
31. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6/scsi_host/host6 
32. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0/host6 
33. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1/2-1:1.0 
34. remove@/devices/pci0000:00/0000:00:1d.7/usb2/2-1 
35. remove@/host6/target6:0:0 



### iproute2的使用

socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, protocol);

```c
// iproute2
// 实现命令  tc filter add dev fwbr0tofwns1 ingress bpf da obj cls_test.o sec classifier/hello
// tc/tc.c
int main(int argc, char **argv)
	do_cmd
    
static int do_cmd(int argc, char **argv)
{
	if (matches(*argv, "qdisc") == 0)
		return do_qdisc(argc-1, argv+1);
	if (matches(*argv, "class") == 0)
		return do_class(argc-1, argv+1);
	if (matches(*argv, "filter") == 0)
		return do_filter(argc-1, argv+1);
    
    
do_filter
    tc_filter_modify		// "add"/"change"/"replace"/"delete"		tc filter add/replace/del
    	parse_fopt			// tc/f_bpf.c: bpf_parse_opt
    	rtnl_talk
    		__rtnl_talk
    			__rtnl_talk_iov
    				sendmsg
    
struct rtnl_handle rth;
rtnl_open(&rth, 0)	// lib/libnetlink.c
    rtnl_open_byproto(rth, subscriptions, NETLINK_ROUTE)
    	socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, protocol);
    	setsockopt(rth->fd, SOL_SOCKET, SO_SNDBUF,
		setsockopt(rth->fd, SOL_SOCKET, SO_RCVBUF,
		setsockopt(rth->fd, SOL_NETLINK, NETLINK_EXT_ACK,
        bind(rth->fd, (struct sockaddr *)&rth->local,
        getsockname(rth->fd, (struct sockaddr *)&rth->local,
```

