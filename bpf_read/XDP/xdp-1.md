# 1

https://www.iovisor.org/technology/xdp

[使用XDP防御DDoS攻击](https://blog.csdn.net/dog250/article/details/77993218)





https://blog.apnic.net/2020/09/02/journeying-into-xdp-part-0/

https://gitlab.epfl.ch/debeule/bpf/-/blob/master/LOG.md	



【Q】

If my NIC driver don’t support XDP, how do I test it

How to show if a NIC driver support xdpoffload

How to show if a XDP program has loaded on a interface

How to translate the bpf code to c format

https://liuhangbin.netlify.app/post/ebpf-and-xdp/



## [eBPF XDP 子系统](https://houmin.cc/posts/b7703758/)

### XDP 输入参数

作为一个**在驱动中运行 BPF 的框架**，XDP 还保证了**包是线性放置并且可以匹配到单个 DMA 页面**，这个页面对 BPF 程序来说是可读和可写的。BPF 允许以 `direct packet access` 的方式访问包中的数据，这意味着**程序直接将数据的指针放到了寄存器中，然后将内容加载到寄存器，相应地再将内容从寄存器写到包中**。数据包在 XDP 中的表示形式是 `xdp_buff`，这也是传递给 BPF 程序的结构体（BPF 上下文）：

<font style="background-color:#ff5555">！！！理解samples等中，对 `bpf_xdp_adjust_head()`、`bpf_xdp_adjust_meta()`的使用</font>

需要彻底理解这个结构中的指针

```c
// include/net/xdp.h
struct xdp_buff {
    void *data;      // 指向 page 中包数据的起始位置
    void *data_end;  // 指向 page 中包数据的结尾位置
    void *data_meta; // 开始时指向与 data 相同的位置
    void *data_hard_start;  // 指向页面中最大可能的 headroom 开始位置
    struct xdp_rxq_info *rxq;
    struct xdp_txq_info *txq;
    u32 frame_sz; /* frame size to deduce data_hard_end/reserved tailroom*/
};

//rxq 字段指向某些额外的、和每个接收队列相关的元数据：这些元数据是在缓冲区设置时确定的（并不是在 XDP 运行时）。BPF 程序可以从 netdevice 自身获取 queue_index 以及其他信息，例如 ifindex。
struct xdp_rxq_info {
	struct net_device *dev;
	u32 queue_index;
	u32 reg_state;
	struct xdp_mem_info mem;
} ____cacheline_aligned; /* perf critical, avoid false-sharing */

struct xdp_txq_info {
	struct net_device *dev;
};
```

XDP 还提供了额外的 256 字 节 headroom 给 BPF 程序，

- 利用 `bpf_xdp_adjust_head()`辅助函数实现自定义封装头
    - 当对包进行封装（加 header）时，`data` 会逐渐向 `data_hard_start` 靠近
    - 该辅助函数还支持解封装（去 header）
- 通过 `bpf_xdp_adjust_meta()`在包前面添加自定义元数据
    - `bpf_xdp_adjust_meta()` 能够将 `data_mata` 朝着 `data_hard_start` 移动，这样可以给自定义元数据提供空间，这个空间对内核网络栈是不可见的，但对 tc BPF 程序可见，因为 tc 需要将它从 XDP 转移到 `skb`。
    - `bpf_xdp_adjust_meta()` 也可以将 `data_meta` 移动到离 `data_hard_start` 比较远的位 置，这样就可以达到删除或缩小这个自定义空间的目的。
    - `data_meta` 还可以单纯用于在尾调用时传递状态，和 tc BPF 程序中用 `skb->cb[]` 控制块（control block）类似。

这样，我们就可以得到这样的结论，对于 `struct xdp_buff` 中数据包的指针，有： `data_hard_start` <= `data_meta` <= `data` < `data_end`.

`data_meta`与`data`指针之间的区域，填入的是meta部分

XDP 还能够在包的前面 push 元数据（非包内容的数据）。这些元数据对常规的内核栈是不可见的（invisible），但能被 GRO 聚合（匹配元数据），稍后可以和 tc ingress BPF 程序一起处理，tc BPF 中携带了 `skb` 的某些上下文，例如，设置了某些 skb 字段。

#### 调整headroom

##### bpf_xdp_adjust_head

签名： 		int bpf_xdp_adjust_head(struct xdp_buff *xdp_md, int delta)

将 d->data 移动 delta字节，delta可以是负数。

该函数准备用于push/pop headers的封包。

调用此助手函数会导致封包缓冲区改变，因此在加载期间校验器对指针的校验将失效，必须重新校验。

##### bpf_xdp_adjust_meta

签名： 		int bpf_xdp_adjust_meta(struct xdp_buff *xdp_md, int delta)

将 xdp_md->data_meta所指向的地址调整 delta字节。该操作改变了存储在xdp_md->data中的地址信息。



还有一个skb的

bpf_skb_adjust_room

签名： 		int bpf_skb_adjust_room(struct sk_buff *skb, u32 len_diff, u32 mode, u64 flags)

增加/缩小skb关联的封包的数据的room，增量为len_diff。mode可以是：

1. BPF_ADJ_ROOM_NET，在网络层调整room，即在L3头上增加/移除room space

flags必须置零。

调用此助手函数会导致封包缓冲区改变，因此在加载期间校验器对指针的校验将失效，必须重新校验。

------

### XDP 输出参数

XDP BPF 程序执行结束后会返回一个判决结果，告诉驱动接下来如何处理这个包。在系统头文件 `linux/bpf.h` 中列出了所有的判决类型。

```c
enum xdp_action {
    XDP_ABORTED = 0,
    XDP_DROP,
    XDP_PASS,
    XDP_TX,
    XDP_REDIRECT,
};
```

- `XDP_DROP` 表示立即在驱动层将包丢弃。这样可以节省很多资源，对于 DDoS mitigation 或通用目的防火墙程序来说这尤其有用。
- `XDP_PASS` 表示允许将这个包送到内核网络栈。同时，当前正在处理这个包的 CPU 会 **分配一个 `skb`**，做一些初始化，然后将其**送到 GRO 引擎**。这和没有 XDP 时默认的包处理行为是一样的。
- `XDP_TX` 是 BPF  程序的一个高效选项，能够在收到包的网卡上直接将包再发送出去。对于实现防火墙+负载均衡的程序来说这非常有用，因为这些部署了 BPF  的节点可以作为一个hairpin（发卡模式，从同一个设备进去再出来）模式的负载均衡器集群，将收到的包在XDP BPF程序中重写（目的地址等）之后直接发送回去。
- `XDP_REDIRECT` 与 `XDP_TX` 类似，但是通过另一个网卡将包发出去。另外， `XDP_REDIRECT` 还可以将包重定向到一个 BPF cpumap，即，当前执行 XDP 程序的 CPU 可以将这个包交给某个远端 CPU，由后者将这个包送到更上层的内核栈，当前 CPU 则继续在这个网卡执行接收和处理包的任务。这**和 `XDP_PASS` 类似，但当前 CPU 不用去做将包送到内核协议栈的准备工作（分配 `skb`，初始化等等），这部分开销还是很大的**。
- `XDP_ABORTED` 表示程序产生异常，其行为和 `XDP_DROP`，但 `XDP_ABORTED` 会经过 `trace_xdp_exception` tracepoint，因此可以通过 tracing 工具来监控这种非正常行为。

------

### XDP 使用案例

**DDoS 防御、防火墙**

**包转发和负载均衡**：通过 `XDP_TX` 或 `XDP_REDIRECT` 动作实现

**栈前过滤与处理**

**流抽样和监控**：这个类似于pcap cube

Facebook 的 SHIV 和 Droplet 基础设施：

- [演讲 Slides](https://www.netdevconf.org/2.1/slides/apr6/zhou-netdev-xdp-2017.pdf)
- [演讲视频](https://youtu.be/YEU2ClcGqts)

 Cloudflare：

- [Slides](https://www.netdevconf.org/2.1/slides/apr6/bertin_Netdev-XDP.pdf)
- [Video](https://youtu.be/7OuOukmuivg)

------

### XDP 工作模式

可以使用ethtool查看经XDP处理的报文统计：

```shell
# ethtool -S eth0
NIC statistics:
  rx_queue_0_packets: 547115
  rx_queue_0_bytes: 719558449
  rx_queue_0_drops: 0
  rx_queue_0_xdp_packets: 0
  rx_queue_0_xdp_tx: 0
  rx_queue_0_xdp_redirects: 0
  rx_queue_0_xdp_drops: 0
  rx_queue_0_kicks: 20
  tx_queue_0_packets: 134668
  tx_queue_0_bytes: 30534028
  tx_queue_0_xdp_tx: 0
  tx_queue_0_xdp_tx_drops: 0
  tx_queue_0_kicks: 127973
```



#### Native XDP

默认模式，当讨论 XDP 时通常隐含的都是指这种模式。在这种模式中，BPF 程序直接在驱动的接收路径上运行，理论上这是软件层最早可以处理包的位置。这是常规的 XDP 模式，**需要驱动实现对 XDP 的支持**，目前 Linux 内核中主流的 10G/40G 网卡都已经支持。

内核代码网卡驱动部分，搜索“XDP_SETUP_PROG”，就是支持这种模式的

#### Offloaded XDP

在这种模式中，XDP BPF 程序直接 offload 到网卡，而不是在主机的 CPU 上执行。 因此，本来就已经很低的 per-packet 开销完全从主机下放到网卡，能够比运行在 native XDP 模式取得更高的性能。这种 offload 通常由**智能网卡**（例如支持 Netronome’s nfp 驱动的网卡）实现，这些网卡有多线程、多核流处理器（flow processors），一个位于内核中的 JIT 编译器（ in-kernel JIT compiler）将 BPF 翻译成网卡的原生指令。

虽然在这种模式中某些 BPF map 类型 和 BPF 辅助函数是不能用的。BPF 校验器检测到这种情况时会直接报错，告诉用户哪些东西是不支持的。除了这些不支持的 BPF 特性之外，其他方面与 native XDP 都是一样的。

内核代码网卡驱动部分，搜索“XDP_SETUP_PROG_HW”，就是支持这种模式的

#### Generic XDP

对于还没有实现 native 或 offloaded XDP 的驱动，内核提供了一个 generic XDP 选项，这种模式不需要任何驱动改动。

generic XDP hook 位于内核协议栈的主接收路径上，接受的是 `skb` 格式的包，但由于 **这些 hook 位于 ingress 路径的很后面**，因此与 native XDP 相比性能有明显下降。因此，`xdpgeneric` 大部分情况下只能用于试验目的，很少用于生产环境。

#### 命令加载

然后通过如下命令加载：

```shell
$ ip link set dev em1 xdp obj prog.o sec xdp
```

默认情况下，如果 XDP 程序已经 attach 到网络接口，那再次加载会报错，这样设计是为了防止程序被无意中覆盖。要强制替换当前正在运行的 XDP 程序，必须指定 `-force` 参数：

```shell
$ ip -force link set dev em1 xdp obj prog.o
```

今天，大部分支持 XDP 的驱动都支持**在不会引起流量中断的前提下原子地替换运行中的程序**。出于性能考虑，支持 XDP 的驱动只允许 attach 一个程序 ，不支持程序链（a chain of programs）。如果有必要的话，可以通过尾调用来对程序进行拆分，以达到与程序链类似的效果。

如果一个接口上有 XDP 程序 attach，`ip link` 命令会显示一个 **`xdp` 标记**。因 此可以用 `ip link | grep xdp` 查看所有有 XDP 程序运行的接口。

==`ip -d link`== 可以查 看进一步信息；另外，`bpftool` 指定 BPF 程序 ID 可以获取 attached 程序的信息，其中程序 ID 可以通过 `ip link` 看到。

要从接口**删除 XDP 程序**，执行下面的命令：

```shell
$ ip link set dev em1 xdp off
```

要将驱动的工作模式从 non-XDP 切换到 native XDP  ，或者相反，通常情况下驱动都需要重新配置它的接收（和发送）环形缓冲区，以保证接收的数据包在单个页面内是线性排列的， 这样 BPF  程序才可以读取或写入。一旦完成这项配置后，大部分驱动只需要执行一次原子的程序替换，将新的 BPF 程序加载到设备中。

执行 `ip link set dev em1 xdp obj [...]` 命令时，**内核会先尝试以 native XDP 模 式加载程序，如果驱动不支持再自动回退到 generic XDP 模式**。如果显式指定了 `xdpdrv` 而不是 `xdp`，那驱动不支持 native XDP 时加载就会直接失败，而不再尝试 generic XDP 模式。



一个例子：以 native XDP 模式强制加载一个 BPF/XDP 程序，打印链路详情，最后再卸载程序：

```shell
$ ip -force link set dev em1 xdpdrv obj prog.o
$ ip link show
[...]
6: em1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc mq state UP mode DORMANT group default qlen 1000
    link/ether be:08:4d:b6:85:65 brd ff:ff:ff:ff:ff:ff
    prog/xdp id 1 tag 57cd311f2e27366b
[...]
$ ip link set dev em1 xdpdrv off
```

还是这个例子，但强制以 generic XDP 模式加载（即使驱动支持 native XDP），另外用 bpftool 打印 attached 的这个 dummy 程序内具体的 BPF 指令：

```shell
$ ip -force link set dev em1 xdpgeneric obj prog.o
$ ip link show
[...]
6: em1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdpgeneric qdisc mq state UP mode DORMANT group default qlen 1000
    link/ether be:08:4d:b6:85:65 brd ff:ff:ff:ff:ff:ff
    prog/xdp id 4 tag 57cd311f2e27366b                <-- BPF program ID 4
[...]
$ bpftool prog dump xlated id 4                       <-- Dump of instructions running on em1
0: (b7) r0 = 1
1: (95) exit
$ ip link set dev em1 xdpgeneric off
```

最后卸载 XDP，用 bpftool 打印程序信息，查看其中的一些元数据：

```shell
$ ip -force link set dev em1 xdpoffload obj prog.o
$ ip link show
[...]
6: em1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdpoffload qdisc mq state UP mode DORMANT group default qlen 1000
    link/ether be:08:4d:b6:85:65 brd ff:ff:ff:ff:ff:ff
    prog/xdp id 8 tag 57cd311f2e27366b
[...]

$ bpftool prog show id 8
8: xdp  tag 57cd311f2e27366b dev em1                  <-- Also indicates a BPF program offloaded to em1
    loaded_at Apr 11/20:38  uid 0
    xlated 16B  not jited  memlock 4096B

$ ip link set dev em1 xdpoffload off
```

注意，每个程序只能选择用一种 XDP 模式加载，无法同时使用多种模式，例如 `xdpdrv` 和 `xdpgeneric`。

**无法原子地在不同 XDP 模式之间切换**，例如从 generic 模式切换到 native 模式。但 重复设置为同一种模式是可以的：

```shell
$ ip -force link set dev em1 xdpgeneric obj prog.o
$ ip -force link set dev em1 xdpoffload obj prog.o
RTNETLINK answers: File exists

$ ip -force link set dev em1 xdpdrv obj prog.o
RTNETLINK answers: File exists

$ ip -force link set dev em1 xdpgeneric obj prog.o    <-- Succeeds due to xdpgeneric
```

在不同模式之间切换时，需要先退出当前的操作模式，然后才能进入新模式：

```shell
$ ip -force link set dev em1 xdpgeneric obj prog.o
$ ip -force link set dev em1 xdpgeneric off
$ ip -force link set dev em1 xdpoffload obj prog.o

$ ip l
[...]
6: em1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdpoffload qdisc mq state UP mode DORMANT group default qlen 1000
    link/ether be:08:4d:b6:85:65 brd ff:ff:ff:ff:ff:ff
    prog/xdp id 17 tag 57cd311f2e27366b
[...]

$ ip -force link set dev em1 xdpoffload off
```

## 参考资料
