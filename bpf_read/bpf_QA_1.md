

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

------

bpf(BPF_MAP_CREATE...)时，返回的是一个**fd**，那么这个fd是如何与创建的map关联的？load的时候呢？
create和load的时候，返回的肯定都是fd，int型的，kernel内部维护与bpf-map或者bpf-prog结构的关联。



------



