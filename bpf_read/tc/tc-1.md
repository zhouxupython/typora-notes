# tc-1

[你的第一个TC BPF 程序](https://davidlovezoe.club/wordpress/archives/952)

![img](DraggedImage.png)

TC有4大组件：

- **Queuing disciplines**，简称为**qdisc**，直译是「队列规则」，它的本质是一个带有算法的队列，默认的算法是**FIFO**，形成了一个最简单的流量调度器。
- **Class**，直译是「种类」，它的本质是为上面的qdisc进行分类。因为现实情况下会有很多qdisc存在，每种qdisc有它特殊的职责，根据职责的不同，可以对qdisc进行分类。
- **Filters**，直译是「过滤器」，它是用来过滤传入的网络包，使它们进入到对应class的qdisc中去。
- **Policers**，直译是「规则器」，它其实是filter的跟班，通常会紧跟着filter出现，定义命中filter后网络包的后继操作，如丢弃、延迟或限速。



【Q】那么TC是怎么和BPF联系在一起的呢？

从内核4.1版本起，引入了一个特殊的**qdisc**，叫做**clsact**，它为TC提供了一个可以加载BPF程序的入口，使TC和XDP一样，成为一个可以加载BPF程序的网络钩子。

【Q】clsact这个对应的上述四个组件都是什么？



【Q】这几个的n-m对应关系是怎样的？



bpf程序attach到tc上，就是多了一个qdisc这样的概念，bpf程序就是一种算法，决定了数据包的处理方式

用来加载BPF程序是个特殊的**qdisc** 叫**clsact**



```shell
# 最开始的状态
> tc qdisc show dev veth09e1d2e
qdisc noqueue 0: root refcnt 2

# 创建clsact
> tc qdisc add dev veth09e1d2e clsact

# 再次查看，观察有什么不同
> tc qdisc show dev veth09e1d2e
qdisc noqueue 0: root refcnt 2
qdisc clsact ffff: parent ffff:fff1

# 加载TC BPF程序到容器的veth网卡上
> tc filter add dev veth09e1d2e egress bpf da obj tc-xdp-drop-tcp.o sec tc

# 再次查看，观察有什么不同
> tc qdisc show dev veth09e1d2e
qdisc noqueue 0: root refcnt 2
qdisc clsact ffff: parent ffff:fff1

> tc filter show dev veth09e1d2e egress
filter protocol all pref 49152 bpf chain 0
filter protocol all pref 49152 bpf chain 0 handle 0x1 tc-xdp-drop-tcp.o:[tc] direct-action not_in_hw id 24 tag 9c60324798bac8be jited
```

参数**da**，它的全称是「direct action」告诉TC请使用BPF程序提供的返回值，无需再手动指定action了

不使用bpf程序时，需要手动指定action

```shell
# 一个没有使用bpf的tc filter
tc filter add dev eth0 protocol ip parent 1:0 prio 1 u32 match ip src 1.2.3.4 action drop
```

