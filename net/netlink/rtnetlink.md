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
		.input		= rtnetlink_rcv,// 收到用户态NETLINK_ROUTE协议的数据后，触发这个
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

static struct pernet_operations rtnetlink_net_ops = {// netlink_proto_init中注册
	.init = rtnetlink_net_init,
	.exit = rtnetlink_net_exit,
};

// netlink_proto_init中调用
void __init rtnetlink_init(void)
{
	register_netdevice_notifier(&rtnetlink_dev_notifier);

	rtnl_register(PF_UNSPEC, RTM_GETLINK, rtnl_getlink, rtnl_dump_ifinfo, 0);
	rtnl_register(PF_UNSPEC, RTM_SETLINK, rtnl_setlink, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_NEWLINK, rtnl_newlink, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_DELLINK, rtnl_dellink, NULL, 0);


static int __init netlink_proto_init(void)//net/netlink/af_netlink.c
{
    
    sock_register(&netlink_family_ops);
	register_pernet_subsys(&netlink_net_ops);//@@@
	register_pernet_subsys(&netlink_tap_net_ops);
	/* The netlink device handler may be needed early. */
	rtnetlink_init();//@@@
}
core_initcall(netlink_proto_init);
```



