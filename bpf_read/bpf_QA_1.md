# ***整理好之后，可以将内容挪到其他地方去，这里留一个链接***

------

bpf()执行后，是将字节码挂载，还是JIT后再挂载？也就是事件触发后，执行的是字节码，还是需要先JIT转换成机器码，然后再执行？挂载的时候，为何不能让JIT转换成机器码呢？

------

BPF MAPS的作用是什么？各种maps的使用场景是什么？

eBPF使用的主要的数据结构是eBPF map，这是一个通用的数据结构，```用于在内核或内核和用户空间传递数据```。map的结构是什么样子的，**key**是什么？

使用bpf()系统调用创建和操作map数据结构。成功创建map后，将返回与该map关联的文件描述符。每个map由四个值定义:类型（map_type）、元素的最大个数（max_entries）、值大小(value_size，以字节为单位)和键大小(key_size，以字节为单位)。**这几个值在哪儿进行描述？**

**linux-5.14.14/include/uapi/linux/bpf.h**

```c
union bpf_attr {
       struct { /* anonymous struct used by BPF_MAP_CREATE command */
              __u32  map_type;      /* one of enum bpf_map_type */
              __u32  key_size;      /* size of key in bytes  这个是创建时指定的key长度*/
              __u32  value_size;    /* size of value in bytes */
              __u32  max_entries;   /* max number of entries in a map */
              __u32  map_flags;     /* BPF_MAP_CREATE related
                                                     * flags defined above.
                                                     */
              __u32  inner_map_fd;  /* fd pointing to the inner map */
               __u32  numa_node;  /* numa node (effective only if
                                                      * BPF_F_NUMA_NODE is set).
                                                      */
              char  map_name[BPF_OBJ_NAME_LEN];
       };

       struct { /* anonymous struct used by BPF_MAP_*_ELEM commands */
              __u32         map_fd;
              __aligned_u64  key;  /* 这个就是key*/
              union {
                     __aligned_u64 value;
                     __aligned_u64 next_key;
              };
              __u64         flags;
       };

       struct { /* anonymous struct used by BPF_PROG_LOAD command */
              __u32         prog_type;     /* one of enum bpf_prog_type */
              __u32         insn_cnt;
              __aligned_u64  insns;
              __aligned_u64  license;
              __u32         kern_version;  /* not used */
              __u32         prog_flags;
              char          prog_name[BPF_OBJ_NAME_LEN];
              __u32         prog_ifindex;  /* ifindex of netdev to prep for */


       };
```



==int bpf(int cmd, union bpf_attr *attr, unsigned int size);==

==cmd:==

BPF_MAP_CREATE，

BPF_MAP_*_***ELEM，

BPF_PROG_LOAD，



==BPF_MAP_CREATE==:map类型，每种类型都提供不同的行为和一些权衡：==enum bpf_map_type {==

- `BPF_MAP_TYPE_HASH`: 一种哈希表

- `BPF_MAP_TYPE_ARRAY`: 一种为快速查找速度而优化的数组类型map键值对，通常用于计数器

- `BPF_MAP_TYPE_PROG_ARRAY`: 与eBPF程序相对应的一种文件描述符数组;用于实现跳转表和处理特定（网络）包协议的子程序

- `BPF_MAP_TYPE_PERCPU_ARRAY`: 一种基于每个cpu的数组，用于实现展现延迟的直方图

- `BPF_MAP_TYPE_PERF_EVENT_ARRAY`: 存储指向`perf_event`数据结构的指针，用于读取和存储perf事件计数器

- `BPF_MAP_TYPE_CGROUP_ARRAY`: 存储指向控制组的指针

- `BPF_MAP_TYPE_PERCPU_HASH`: 一种基于每个CPU的哈希表

- `BPF_MAP_TYPE_LRU_HASH`: 一种只保留最近使用项的哈希表

- `BPF_MAP_TYPE_LRU_PERCPU_HASH`: 一种基于每个CPU的哈希表，只保留最近使用项

- `BPF_MAP_TYPE_LPM_TRIE`: 一个匹配最长前缀的字典树数据结构，适用于将IP地址匹配到一个范围

- `BPF_MAP_TYPE_STACK_TRACE`: 存储堆栈跟踪信息

- `BPF_MAP_TYPE_ARRAY_OF_MAPS`: 一种map-in-map数据结构

- `BPF_MAP_TYPE_HASH_OF_MAPS`: 一种map-in-map数据结构

- `BPF_MAP_TYPE_DEVICE_MAP`: 用于存储和查找网络设备的引用

- `BPF_MAP_TYPE_SOCKET_MAP`: 存储和查找套接字，并允许使用BPF帮助函数进行套接字重定向

    

==BPF_PROG_LOAD==:目前内核支持的eBPF程序类型列表如下所示：==enum bpf_prog_type {==

- `BPF_PROG_TYPE_SOCKET_FILTER`: 一种网络数据包过滤器
- `BPF_PROG_TYPE_KPROBE`: 确定kprobe是否应该触发
- `BPF_PROG_TYPE_SCHED_CLS`: 一种网络流量控制分类器
- `BPF_PROG_TYPE_SCHED_ACT`: 一种网络流量控制动作
- `BPF_PROG_TYPE_TRACEPOINT`: 确定 tracepoint是否应该触发
- `BPF_PROG_TYPE_XDP`: 从设备驱动程序接收路径运行的网络数据包过滤器
- `BPF_PROG_TYPE_PERF_EVENT`: 确定是否应该触发perf事件处理程序
- `BPF_PROG_TYPE_CGROUP_SKB`: 一种用于控制组的网络数据包过滤器
- `BPF_PROG_TYPE_CGROUP_SOCK`: 一种由于控制组的网络包筛选器，它被允许修改套接字选项
- `BPF_PROG_TYPE_LWT_*`: 用于轻量级隧道的网络数据包过滤器
- `BPF_PROG_TYPE_SOCK_OPS`: 一个用于设置套接字参数的程序
- `BPF_PROG_TYPE_SK_SKB`: 一个用于套接字之间转发数据包的网络包过滤器
- `BPF_PROG_CGROUP_DEVICE`: 确定是否允许设备操作



![img](https://davidlovezoe.club/wordpress/wp-content/uploads/2020/04/ebpfworkflow.png)

eBPF流程图中有个错误吧。
perf event不是以BPF map方式发送给用户态的，是走perf缓冲区，单项从内核态发送到用户态的。???

------

通过对程序的***控制流程图***（CFG）进行深度优先搜索来检查确保eBPF程序终止，并且不包含任何可能导致内核锁定的循环

------

BCC

kerneltravel视频中，那个胖子使用的几个bcc功能，看看是作了什么？分析以下代码和作用，可以作为契口

------

bpf(BPF_MAP_CREATE...)时，返回的是一个**fd**，那么这个fd是如何与创建的map关联的？load的时候呢？
create和load的时候，返回的肯定都是fd，int型的，kernel内部维护与bpf-map或者bpf-prog结构的关联。



------

bpf相关的api，哪些是==用户层面==使用的，哪些是==内核层面==的钩子函数使用的？



------

字段的含义

zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp object tc-xdp-drop-tcp.o section xdp verbose

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 18 (0 over limit)
 - License:      GPL

Verifier analysis:

processed 25 insns (limit 1000000) max_states_per_insn 0 total_states 2 peak_states 2 mark_read 1



x@zx:~$ ip a
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever
zx@zx:~$ ip a
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp/id:107 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever


【Q】xdp/id:107  含义是什么

------

为何

bpf_read/experiments/exp1.md:反复开启关闭xdp访问外网

和 ==https://davidlovezoe.club/wordpress/archives/952 2. 在Nginx容器内部curl外部网站==实验结果不一样

------

为何

 https://davidlovezoe.club/wordpress/archives/952 的xdp使用后，ip a 命令显示的是 “xdpgeneric”

自己测试的是 xdp/id:107 这样的，有何不同？

------

tcpdump抓包的钩子点，与xdp、tc的钩子点比较

谁较早被执行？

------

既然上面这样，那么如何通过代码证明xdp过滤包的效率更高？自己分析这部分的性能，就用bpf提供的工具

------

ebpf的map类型，内部是什么区别，优劣区分？比如如下两种

BPF_MAP_TYPE_HASH,

BPF_MAP_TYPE_ARRAY,

均有key、value

```c
//samples/bpf/sockex1_kern.c
struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, u32);
	__type(value, long);
	__uint(max_entries, 256);
} my_map SEC(".maps");
```

------

```c
//samples/bpf/tracex4_kern.c
struct pair {
	u64 val;
	u64 ip;
};

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, long);
	__type(value, struct pair);
	__uint(max_entries, 1000000);
} my_map SEC(".maps");

展开：
struct {
	int (*type)[BPF_MAP_TYPE_HASH]
	typeof(long) *key
	typeof(struct pair) *value		/*指针啊*/
	int (*max_entries)[1000000]
} my_map SEC(".maps");
```



sudo strace -v -f -s 128 -o tracex4.txt ./tracex4：

bpf(BPF_MAP_CREATE, {map_type=BPF_MAP_TYPE_HASH, key_size=8, value_size=`16`, 



sudo bpftool map show 看到的也是
	key 8B	value `16B`  max_entries 1000000	memlock 88788992B

<font>[Q]</font>为何这里的value是16B，不是指针吗？应该是8B才对阿？



换个map的定义看看，bcc定义方式：

```c
// include/linux/sched.h
/* Task command name length: */
#define TASK_COMM_LEN			16

struct val_t {
    u64 id;
    char comm[TASK_COMM_LEN];
    const char *fname;
};

BPF_HASH(infotmp, u64, struct val_t);
```



sudo strace -v -f -s 128 -o my_open_snoop_4.txt  /usr/bin/python    ./my_open_snoop_4.py

bpf(BPF_MAP_CREATE, {map_type=BPF_MAP_TYPE_HASH, key_size=8, value_size=`32`, .......

sizeof(struct val_t ) = 8 + 16 + 8 = 32

是结构体的大小



c定义方式

```c
#define __uint(name, val) int (*name)[val]
#define __type(name, val) typeof(val) *name
#define __array(name, val) typeof(val) *name[]

struct pair {
	u64 val;
	u64 ip;
};

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, long);
	__type(value, struct pair);
	__uint(max_entries, 1000000);
} my_map SEC(".maps");

展开：
struct {
	int (*type)[BPF_MAP_TYPE_HASH]
	typeof(long) *key
	typeof(struct pair) *value		虽然是指针
	int (*max_entries)[1000000]
} my_map SEC(".maps");
```

虽然使用SEC定义的时候，是使用的指针，但是map创建的时候，是字段的大小，

sizeof（struct pair）= 16

------





1. **尾调用**（tail call）：高效地调用其他 BPF 程序
2. **安全加固原语**（security hardening primitives）
3. 用于 pin/unpin 对象（例如 map、程序）的**伪文件系统**（`bpffs`），实现持久存储
4. 支持 BPF **offload**（例如 offload 到网卡）的基础设施

BPF_OBJ_GET 如何指定想要获取的obj，因为肯定不止一个obj被pin到**BPF 文件系统**



区别是什么？

- **如果指定的是 `PIN_GLOBAL_NS`，那 map 会被放到 `/sys/fs/bpf/tc/globals/`**。 `globals` 是一个跨对象文件的全局命名空间。
- 如果指定的是 `PIN_OBJECT_NS`，tc 将会为对象文件创建一个它的本地目录（local to the object file）。例如，只要指定了 `PIN_OBJECT_NS`，不同的 C 文件都可以像上 面一样定义各自的 `acc_map`。在这种情况下，这个 map 会在不同 BPF 程序之间共享。
- `PIN_NONE` 表示 map 不会作为节点（node）钉（pin）到 BPF 文件系统，因此当 tc 退 出时这个 map 就无法从用户空间访问了。同时，这还意味着独立的 tc 命令会创建出独 立的 map 实例，因此后执行的 tc 命令无法用这个 map 名字找到之前被钉住的 map。 在路径 `/sys/fs/bpf/tc/globals/acc_map` 中，map 名是 `acc_map`。



```shell
$ mount | grep bpf
sysfs on /sys/fs/bpf type sysfs (rw,nosuid,nodev,noexec,relatime,seclabel)
bpf on /sys/fs/bpf type bpf (rw,relatime,mode=0700)

$ tree /sys/fs/bpf/
/sys/fs/bpf/
+-- ip -> /sys/fs/bpf/tc/
+-- tc
|   +-- globals
|       +-- acc_map
+-- xdp -> /sys/fs/bpf/tc/
```





------

xdp返回值：XDP_REDIRECT 如何指定一个重定向的cpu？

bpf cpumap ？ 这是什么意思？来个例子
