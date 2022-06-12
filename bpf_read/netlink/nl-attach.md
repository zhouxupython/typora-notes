# 使用netlink挂载xdp、tc ebpf prog



## XDP

https://blog.csdn.net/already_skb/article/details/123024306     ebpf xdp 挂载点分析
https://blog.csdn.net/already_skb/article/details/123074373     xdp 程序如何挂载
https://blog.csdn.net/already_skb/article/details/123091334     Generic XDP Hook
https://blog.csdn.net/already_skb/article/details/123073814     Native xdp hook 点

[重看ebpf -代码载入执行点-hook](https://www.cnblogs.com/codestack/p/14733074.html)

https://switch-router.gitee.io/blog/bpf-3/

一般由`bpf_set_link_xdp_fd`发起

```c
//用户态：
//linux-5.14.14/tools/lib/bpf/netlink.c
bpf_set_link_xdp_fd
    __bpf_set_link_xdp_fd_replace
        libbpf_netlink_send_recv	// 会发送 RTM_NEWLINK 类型报文给内核态
    

//内核态：	会进行回调函数的注册
//linux-5.14.14/net/netlink/af_netlink.c
netlink_proto_init
    rtnetlink_init
core_initcall(netlink_proto_init);

//linux-5.14.14/net/core/rtnetlink.c
rtnetlink_init
	rtnl_register(PF_UNSPEC, RTM_SETLINK, rtnl_setlink, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_NEWLINK, rtnl_newlink, NULL, 0);// 内核态会使用该函数响应 RTM_NEWLINK 类型报文
	rtnl_register(PF_UNSPEC, RTM_DELLINK, rtnl_dellink, NULL, 0);

// 开始处理attach请求
rtnl_newlink
    do_setlink
        if (tb[IFLA_XDP])
            if (xdp[IFLA_XDP_FD])
                dev_change_xdp_fd
                    dev_xdp_attach
                        bpf_op = dev_xdp_bpf_op(dev, mode); //根据mode获取挂载函数，generic 对应generic_xdp_install函数，其他两种是 ndo_bpf， 是驱动厂家提供的函数，通过注册生效
                        dev_xdp_install(dev, mode, bpf_op, extack, flags, new_prog);//挂载，传入获取的挂载函数
                            bpf_op(dev, &xdp)   //调用上面获取的bpf_op开始挂载
```

bpf_op有三种情况：
`generic mode`: bpf_op是内核提供的模拟 generic_xdp_install  ;
`native mode`: 也就是`driver mode`，xdp prog加载到网卡驱动中，不是内核协议栈，此时 ndo_bpf 比如 i40e_xdp ，不支持offloaded
`offloaded mode`: xdp prog加载到网卡硬件中，此时 ndo_bpf 比如 nfp_net_xdp ，native mode和offloaded mode都支持

```c
//linux-5.14.14/drivers/net/ethernet/intel/i40e/i40e_main.c
/**
 * i40e_xdp - implements ndo_bpf for i40e
 * @dev: netdevice
 * @xdp: XDP command
 **/
static int i40e_xdp(struct net_device *dev,
		    struct netdev_bpf *xdp)
{
	struct i40e_netdev_priv *np = netdev_priv(dev);
	struct i40e_vsi *vsi = np->vsi;

	if (vsi->type != I40E_VSI_MAIN)
		return -EINVAL;

	switch (xdp->command) {
	case XDP_SETUP_PROG:
		return i40e_xdp_setup(vsi, xdp->prog, xdp->extack);
	case XDP_SETUP_XSK_POOL:
		return i40e_xsk_pool_setup(vsi, xdp->xsk.pool,
					   xdp->xsk.queue_id);
	default:
		return -EINVAL;
	}
}
```



```c
//linux-5.14.14/drivers/net/ethernet/netronome/nfp/nfp_net_common.c
static int nfp_net_xdp(struct net_device *netdev, struct netdev_bpf *xdp)
{
	struct nfp_net *nn = netdev_priv(netdev);

	switch (xdp->command) {
	case XDP_SETUP_PROG:
		return nfp_net_xdp_setup_drv(nn, xdp);
	case XDP_SETUP_PROG_HW:
		return nfp_net_xdp_setup_hw(nn, xdp);
	default:
		return nfp_app_bpf(nn->app, nn, xdp);
	}
}
```



原文链接：https://blog.csdn.net/already_skb/article/details/123024306     ebpf xdp 挂载点分析
 xdp 函数的Hook点

很显然xdp也是利用了trace类似的机制，通过埋点挂载回调函数实现，所以xdp一定有埋点函数，而且如果要实现双向的数据报文处理，必须有一个收包路径的关键埋点 和 一个发包路径的关键埋点，而且这两个埋点比如在网卡驱动程序中(事实上xdp只在收包路径上 有埋点)。

ixgbe网卡为例，收包路径埋点在ixgbe_clean_rx_irq函数中，埋点函数是ixgbe_run_xdp，函数调用路径为：

ixgbe_run_xdp <--- ixgbe_clean_rx_irq <--- ixgbe_poll(网卡poll)



https://blog.csdn.net/already_skb/article/details/123091334     Generic XDP Hook

上一篇文章讲过bpf_prog_run_xdp是XDP 程序的最终调用函数，想要跟踪Generic XDP的代码可以从该函数入手。很显然你不应该从driver/，而是已改在net/目录下检索。

jensonqiu@Bing$ grep -rn "bpf_prog_run_xdp" net
net/core/dev.c:4349:	act = bpf_prog_run_xdp(xdp_prog, xdp);

稍微过滤一下真相就呼之欲出了，函数的调用关系如下：

bpf_prog_run_xdp
netif_receive_generic_xdp
do_xdp_generic
netif_rx_internal/netif_receive_skb_internal/netif_receive_skb_list_internal
netif_receive_skb
ixgb_clean_rx_irq //Send received data up the network stack

从调用关系上很容易推导出 Generic XDP的Hook点是在网卡中断的下半部。也即是在网卡软中断的收包流程中，软中断收到报文后直接传递给网络协议栈。
网卡的两种工作模式

网卡有两种工作模式:

1） 软中断收包模式，该机制的特点是直接在软中断函数将报文传递到网络协议栈。

2）软中断唤醒触发轮询模式(NAPI)，该机制的特点是在软中断函数中唤醒网卡驱动注册的poll函数，通过poll函数轮询收包。

API 是一种新技术，目前不是所有网卡驱动都支持这种能力，你如果想确定网卡是不是支持NAPI，有很多方式，其中一种就是查一下网卡驱动是否有实现xxxx_poll函数，比如ixgbe的驱动实现了ixgbe_poll函数。

这两种模式分别对应了XDP的 Generic XDP 和 Native XDP模式。

两种模式的代码汇总点其实都在软中断函数中，Generic XDP是在网卡不支持Native XDP模式时候，通过在ixgb_clean_rx_irq函数中调用netif_receive_skb实现，如果支持Native XDP模式则是通过调用网卡实现的xxxx_poll函数实现。



------

tc

https://arthurchiao.art/blog/firewalling-with-bpf-xdp/#3-how-does-it-work-in-the-underlying-dig-inside	！！！

[iproute2 - ip 命令源码分析](https://www.cnblogs.com/elewei/p/8093847.html)                

```c
// iproute2用户态，接收命令以及发送给内核态

// Calling stack of parsing and executing the following tc command:
// $ tc filter add dev eth0 ingress bpf da obj drop-arp.o sec ingress

main                                                  // tc/tc.c
  |-do_cmd                                            // tc/tc.c
      |-if match(*argv, "filter")
          return do_filter                            // tc/filter.c
              /
          /--/
         /
do_filter                                             // tc/filter.c
  |-if match(*argv, "add")
      return tc_filter_modify                         // tc/filter.c
            /
        /--/
       /
tc_filter_modify                                      // tc/filter.c
  |-struct {
  |   struct nlmsghdr n;     // netlink message header
  |   struct tcmsg    t;     // tc message header
  |   char            buf[];
  |} req = {
  |   n.nl.msg_type = "add",
  | }
  |
  |-Parse CLI parameters:    "dev eth0 ingress"
  |
  |-q->parse_fopt            "bpf da obj drop-arp.o sec ingress"
  |  |-bpf_parse_opt                                  // tc/f_bpf.c
  |     |-cfg.type = BPF_PROG_TYPE_SCHED_CLS          // tc/f_bpf.c
  |
  |-rtnl_talk                                         // lib/libnetlink.c
     |-__rtnl_talk                                    // lib/libnetlink.c
       |-__rtnl_talk_iov                              // lib/libnetlink.c
         |-sendmsg
```



内核态对于回调函数的注册

```c
//linux-5.14.14/net/sched/cls_api.c
static int __init tc_filter_init(void)
{
	int err;

	tc_filter_wq = alloc_ordered_workqueue("tc_filter_workqueue", 0);
	if (!tc_filter_wq)
		return -ENOMEM;

	err = register_pernet_subsys(&tcf_net_ops);
	if (err)
		goto err_register_pernet_subsys;

	rtnl_register(PF_UNSPEC, RTM_NEWTFILTER, tc_new_tfilter, NULL,
		      RTNL_FLAG_DOIT_UNLOCKED);// 响应函数
	rtnl_register(PF_UNSPEC, RTM_DELTFILTER, tc_del_tfilter, NULL,
		      RTNL_FLAG_DOIT_UNLOCKED);
	rtnl_register(PF_UNSPEC, RTM_GETTFILTER, tc_get_tfilter,
		      tc_dump_tfilter, RTNL_FLAG_DOIT_UNLOCKED);
	rtnl_register(PF_UNSPEC, RTM_NEWCHAIN, tc_ctl_chain, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_DELCHAIN, tc_ctl_chain, NULL, 0);
	rtnl_register(PF_UNSPEC, RTM_GETCHAIN, tc_ctl_chain,
		      tc_dump_chain, 0);

	return 0;

err_register_pernet_subsys:
	destroy_workqueue(tc_filter_wq);
	return err;
}
subsys_initcall(tc_filter_init);


//linux-5.14.14/net/sched/cls_bpf.c
static struct tcf_proto_ops cls_bpf_ops __read_mostly = {
	.kind		=	"bpf",
	.owner		=	THIS_MODULE,
	.classify	=	cls_bpf_classify,
	.init		=	cls_bpf_init,
	.destroy	=	cls_bpf_destroy,
	.get		=	cls_bpf_get,
	.change		=	cls_bpf_change,
	.delete		=	cls_bpf_delete,
	.walk		=	cls_bpf_walk,
	.reoffload	=	cls_bpf_reoffload,
	.dump		=	cls_bpf_dump,
	.bind_class	=	cls_bpf_bind_class,
};

static int __init cls_bpf_init_mod(void)
{
	return register_tcf_proto_ops(&cls_bpf_ops);
}

static void __exit cls_bpf_exit_mod(void)
{
	unregister_tcf_proto_ops(&cls_bpf_ops);
}

module_init(cls_bpf_init_mod);
module_exit(cls_bpf_exit_mod);
```

`tc_new_tfilter`

```c
//linux-5.14.14/net/sched/cls_api.c
tc_new_tfilter
    tp->ops->change		// cls_bpf.c: cls_bpf_change()

//linux-5.14.14/net/sched/cls_bpf.c
cls_bpf_change
    prog = kzalloc(sizeof(*prog), GFP_KERNEL);
    cls_bpf_set_parms(net, tp, prog,
        is_ebpf   // true
        cls_bpf_prog_from_efd
            struct bpf_prog *fp = bpf_prog_get_type_dev(bpf_fd, BPF_PROG_TYPE_SCHED_CLS,

	[cls_bpf_offload]

	[replace]:.....
	[add]: list_add_rcu(&prog->link, &head->plist);	//到此完成命令 "tc filter add dev eth0 ingress bpf da obj drop-arp.o sec ingress"           @@@@@@@@@@@ head->plist


其中，head是：
struct cls_bpf_head *head = rtnl_dereference(tp->root);
那么list_add_rcu就是将当前加载的prog插入队首

```

接收到数据包时，处理链是

```c
__netif_receive_skb_core
	sch_handle_ingress                                              //【tc ingress】
        tcf_classify_ingress                                        //执行tc ingress 钩子函数
			__tcf_classify
				tp->classify										//linux-5.14.14/net/sched/cls_bpf.c:cls_bpf_classify
                    list_for_each_entry_rcu(prog, &head->plist, link)
					    BPF_PROG_RUN(prog->filter, skb)             // @@@@@@@@@@@ &head->plist
```

**如果可以tc offloaded**

```c
[cls_bpf_offload]

    cls_bpf_offload(tp, prog, oldprog, extack);
        cls_bpf_offload_cmd(tp, prog, oldprog, extack)
            tc_setup_cb_replace(block, tp, TC_SETUP_CLSBPF, &cls_bpf,		//replace
                __tc_setup_cb_call


            tc_setup_cb_add(block, tp, TC_SETUP_CLSBPF, &cls_bpf,    		//add
                __tc_setup_cb_call


            tc_setup_cb_destroy(block, tp, TC_SETUP_CLSBPF, &cls_bpf,		//delete
                __tc_setup_cb_call


//linux-5.14.14/net/sched/cls_api.c
__tc_setup_cb_call
    block_cb->cb(type, type_data, block_cb->cb_priv)

//cb原型是：
typedef int flow_setup_cb_t(enum tc_setup_type type, void *type_data, void *cb_priv)

//cb举例子：
//linux-5.14.14/drivers/net/ethernet/netronome/nfp/bpf/main.c
static int nfp_bpf_setup_tc_block_cb(enum tc_setup_type type,
				     void *type_data, void *cb_priv)
{
	struct tc_cls_bpf_offload *cls_bpf = type_data;
	struct nfp_net *nn = cb_priv;
	struct bpf_prog *oldprog;
	struct nfp_bpf_vnic *bv;
	int err;

	err = nfp_net_bpf_offload(nn, cls_bpf->prog, oldprog,
				  cls_bpf->common.extack);
	if (err)
		return err;

	bv->tc_prog = cls_bpf->prog;
	nn->port->tc_offload_cnt = !!bv->tc_prog;
	return 0;
}

//netdevsim这个也不是真正的硬件设备，是内核虚拟的
//linux-5.14.14/drivers/net/netdevsim/bpf.c
int nsim_bpf_setup_tc_block_cb(enum tc_setup_type type,
			       void *type_data, void *cb_priv)
{
	struct tc_cls_bpf_offload *cls_bpf = type_data;
	struct bpf_prog *prog = cls_bpf->prog;
	struct netdevsim *ns = cb_priv;
	struct bpf_prog *oldprog;

	oldprog = cls_bpf->oldprog;

	return nsim_bpf_offload(ns, cls_bpf->prog, oldprog);
}
```



---------------------------------------------------------------------------
