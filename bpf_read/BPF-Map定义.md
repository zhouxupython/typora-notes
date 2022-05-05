# map定义

https://blogs.oracle.com/linux/post/bpf-in-depth-communicating-with-userspace

## Array

### BPF_MAP_TYPE_ARRAY

```c
// 定义ARRAY_MAP，使用 BCC 宏定义，不能用map观点，就是个数组，每个元素是u64类型，数组大小256
BPF_ARRAY(count_map, u64, 256);

int count_packets(struct __sk_buff *skb)
{
    int index = load_byte(skb, ETH_HLEN + offsetof(struct iphdr, protocol));
    
    //这个是类似于arr[index]，但是返回的不是对应的数据，而是&arr[index]
    u64 *val = count_map.lookup(&index);
    if(val)//如果index超出数组大小，那么返回空指针
        count_map.increment(index);
    return 0;
}
```

U中使用：

```python
# bpf["count_map"]获取到这个array
# bpf["count_map"][socket.IPPROTO_TCP]类似于arr[index]，U空间不再是&arr[index]，而是真正这个元素的值
# .value表示ctypes.c_ulong 类型转化为int
TCP_cnt = bpf["count_map"][socket.IPPROTO_TCP].value
```



如果是array类型的映射，那么可以使用==__sync_fetch_and_add==对数组的元素值进行原子计算

```c
//samples/bpf/sockex1_kern.c
struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, u32);
	__type(value, long);
	__uint(max_entries, 256);
} my_map SEC(".maps");

value = bpf_map_lookup_elem(&my_map, &index);
	if (value)
		__sync_fetch_and_add(value, skb->len);//对应的数组元素值增加skb->len
```



c代码对应的定义方式

k含义

v含义

------

### BPF_MAP_TYPE_PERCPU_ARRAY

samples/bpf/xdp1_kern.c

samples/bpf/xdp1_user.c

typora-notes/bpf_read/samples/xdp1.md

```c
struct {
	__uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
	__type(key, u32);
	__type(value, long);//根据key搜索到的是一个数组，大小是cpu个数，数组的每个元素，就是value
	__uint(max_entries, 256);// key：0～255，总共256个
} rxcnt SEC(".maps");

unsigned int nr_cpus = bpf_num_possible_cpus();
__u64 values[nr_cpus], prev[UINT8_MAX] = { 0 };
while (bpf_map_get_next_key(map_fd, &key, &key) != -1) {
    __u32 key = UINT32_MAX;
    __u64 sum = 0;

    assert(bpf_map_lookup_elem(map_fd, &key, values) == 0);
    for (i = 0; i < nr_cpus; i++)
        sum += values[i];
    if (sum > prev[key])
        printf("proto %u: %10llu pkt/s\n",
               key, (sum - prev[key]) / interval);
    prev[key] = sum;//每种协议的丢包总数
}
```

使用 bpf_map_lookup_elem(map_fd, &key, values) 获取的 values 是个数组，大小就是运行环境的cpu个数，数组的元素类型就是map定义时的value

例子中，key是==协议类型==，values是个数组，每个元素是各个cpu的丢包数，也就是value。

所以协议对应的values中每个元素的和，就是这种协议类型在所有cpu上的丢包总和。

------

### BPF_MAP_TYPE_PROG_ARRAY

An array of BPF programs used as a jump table by bpf_tail_call(). See samples/bpf/sockex3_kern.c for an example.

------

### BPF_MAP_TYPE_PERF_EVENT_ARRAY

有时候我们期望 eBPF 程序能够通知用户态程序数据准备好了，array、hash 类型的 eBPF map 不满足此类使用场景，                                                                  这时候就轮到 `BPF_MAP_TYPE_PERF_EVENT_ARRAY` 了。与普通 hash、array 类型有些不同，它==没有== `bpf_map_lookup_elem()` 方法，使用的是 `bpf_perf_event_output()` 向用户态传递数据。它的 `value_size` 只能是 `sizeof(u32)`，代表的是 perf_event 的文件描述符；`max_entries` 则是 perf_event 的文件描述符数量。

k含义：u32，可以为cpu编号

v含义：u32，作为perf event buffer从内核往用户空间传递数据时，是 SYS_PERF_EVENT_OPEN系统调用的返回值，表示一个fd。

​             参见bpf_read/PerfBuffer_RingBuffer.md

MaxEntries：cpu个数

```go
func CreatePerfMap(mapPath string, maxEntries uint32) (int, error) {
	var key, val uint32
	cfg := MapConfig{
		MapType:    BPF_MAP_TYPE_PERF_EVENT_ARRAY,
		KeySize:    unsafe.Sizeof(key), //  
		ValSize:    unsafe.Sizeof(val), //  只能是 sizeof(u32)，代表的是 perf_event 的文件描述符
		MaxEntries: maxEntries,
	}

	fd, err := CreatePinnedMapFd(&cfg, mapPath)
	if err != nil {
		return -1, err
	}
	return fd, nil
}

rf.perfFd, err = unix.PerfEventOpen(&attr, -1, rf.cpuIdx, -1, unix.PERF_FLAG_FD_CLOEXEC)

MapUpdate(p.BpfMapFd, unsafe.Pointer(&cpuIdx), unsafe.Pointer(&rf.perfFd), 0)
```

举例：

```c
struct msg {
	__s32 seq;
	__u64 cts;
	__u8 comm[MAX_LENGTH];
};

struct bpf_map_def SEC("maps") map = {
	.type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 0,
};

SEC("kprobe/vfs_read")
int hello(struct pt_regs *ctx) {
	unsigned long cts = bpf_ktime_get_ns();
	struct msg val = {0};
	static __u32 seq = 0;

	val.seq = seq = (seq + 1) % 4294967295U;
	val.cts = bpf_ktime_get_ns();
	bpf_get_current_comm(val.comm, sizeof(val.comm));

	bpf_perf_event_output(ctx, &map, 0, &val, sizeof(val));

	return 0;
}
```

> Note:
>
> 1. 这里的 `seq` 代表的是消息序列号
> 2. 若用户态不向内核态传递消息，PERF_EVENT_ARRAY map 中的 `max_entries` 没有意义。该 map 向用户态传递的数据暂存在 perf ring buffer 中，而由 `max_entries` 指定的 map 存储空间存放的是 perf_event 文件描述符，若用户态程序不向 map 传递 perf_event 的文件描述符，其值可以为 0。用户态程序使用 `bpf(BPF_MAP_UPDATE_ELEM)` 将由 `sys_perf_event_open()` 取得的文件描述符传递给 eBPF 程序，eBPF 程序再使用 `bpf_perf_event_{read, read_value}()` 得到该文件描述符。
> 3. 于此有关的用法见 linux kernel 下的 sample/bpf/tracex6_{user, kern.c}[[kern.c](https://github.com/torvalds/linux/blob/v5.10/samples/bpf/tracex6_kern.c)]、[[user.c](https://github.com/torvalds/linux/blob/v5.10/samples/bpf/tracex6_user.c)]）。

[[libbpf](https://github.com/torvalds/linux/tree/v5.10/tools/lib/bpf)] 提供了 PERF_EVENT_ARRAY map 在用户态开箱即用的 API，它使用了 epoll 进行封装，仅需调用 `perf_buffer__new()`、`perf_buffer__poll()` 即可使用：

```c
static void print_bpf_output(void *ctx, int cpu, void *data, __u32 size) {
	struct msg *msg = data;

	fprintf(stdout, "%.4f: @seq=%d @comm=%s\n",
		 (float)msg->cts/1000000000ul, msg->seq, msg->comm);
}

int main(int argc, char *argv[]) {
	struct perf_buffer_opts pb_opts = {};
	struct perf_buffer *pb;
	...

	pb_opts.sample_cb = print_bpf_output;
	pb = perf_buffer__new(map_fd, 8, &pb_opts);

	while (true) {
		perf_buffer__poll(pb, 1000);
		if (stop)
			break;
	}
	...
}
```

linux-5.14.14/samples/bpf/trace_output_kern.c





------

### BPF_MAP_TYPE_RINGBUF

https://github.com/torvalds/linux/blob/v5.10/Documentation/bpf/ringbuf.rst

------

### BPF_MAP_TYPE_CGROUP_ARRAY

Array map used to store cgroup fds in user-space for later use in BPF  programs which call bpf_skb_under_cgroup() to check if skb is associated with the cgroup in the cgroup array at the specified index.

------

### BPF_MAP_TYPE_ARRAY_OF_MAPS

Allows map-in-map definition where the values are the fds for the inner  maps. Only two levels of map are supported, i.e. a map containing maps,  not a map containing maps containing maps. 

BPF_MAP_TYPE_PROG_ARRAY does  not support map-in-map functionality as it would make tail call  verification harder. See https://www.mail-archive.com/netdev@vger.kernel.org/msg159387.html. for more.

------

## Hash

### BPF_MAP_TYPE_HASH



```c
// 定义HASH_MAP，使用 BCC 宏定义，key 为 u64 类型，value 为 struct val_t 结构；
BPF_HASH(infotmp, u64, struct val_t);
```

![](sock_example_bcc版本实现.jpg)

typora-notes/bpf_read/BCC/bcc-宏.md

### 

```c
// 定义HASH_MAP，使用 BCC 宏定义，就是map，key 为 u64 类型，value 为 struct val_t 结构；
BPF_HASH(infotmp, u64, struct val_t);

infotmp.update(&id, &val);  // 保存中间结果至 hash_map 中,以 id 为 key，将 val 对象结果保存至 infotmp 中；

// 用于读取在map中保存的信息，如果未查询到则直接返回，
// 需要注意的是 lookup 函数的入参和出参都是指针类型，使用前需要判断；
valp = infotmp.lookup(&id); // infotmp[id]， 从 hash_map 中获取到  sys_open 函数保存的中间数据
if (valp == 0) {//没有找到
    // missed entry
    return 0;
}

infotmp.delete(&id);  // 删除这个k-v pair
```



c代码对应的定义方式

k含义

v含义

------

### BPF_MAP_TYPE_PERCPU_HASH

------

### BPF_MAP_TYPE_LRU_HASH

Each hash maintains an LRU (least recently used) list for each bucket to inform delete when the hash bucket fills up.

------

### BPF_MAP_TYPE_LRU_PERCPU_HASH

------

### BPF_MAP_TYPE_HASH_OF_MAPS

Similar to ARRAY_OF_MAPS for for hash. See https://www.mail-archive.com/netdev@vger.kernel.org/msg159383.html for more.



------

## Others

### BPF_MAP_TYPE_STACK_TRACE

defined in kernel/bpf/stackmap.c. Kernel programs can store stacks via  the bpf_get_stackid() helper. The idea is we store stacks based on an  identifier which appears to correspond to a 32-bit hash of the  instruction pointer addresses that comprise the stack for the current  context. The common use case is to get stack id in kernel, and use it as key to update another map. So for example we could profile specific  stack traces by counting their occurence, or associate a specific stack  trace with the current pid as key. See samples/bpf/offwaketime_kern.c  for an example of the latter. In user-space we can look up the symbols  associated with the stackmap to unwind the stack (see  samples/bpf/offwaketime_user.c).



samples/bpf/trace_event_kern.c

------

### BPF_MAP_TYPE_LPM_TRIE

Map supporting efficient longest-prefix matching. Useful for storage/retrieval of IP routes for example.

------

### BPF_MAP_TYPE_SOCKMAP

sockmaps are used primarily for socket redirection, where sockets added to a socket map and referenced by a key which dictates redirection when bpf_sockmap_redirect() is called.

------

### BPF_MAP_TYPE_DEVMAP

does a similar job to sockmap, with netdevices for XDP and bpf_redirect().

------

### BPF_MAP_TYPE_CPUMAP

`XDP_REDIRECT` 与 `XDP_TX` 类似，但是通过另一个网卡将包发出去。另外， `XDP_REDIRECT` 还可以将包重定向到一个 BPF cpumap，即，当前执行 XDP 程序的 CPU 可以将这个包交给某个远端 CPU，由后者将这个包送到更上层的内核栈，当前 CPU 则继续在这个网卡执行接收和处理包的任务。这**和 `XDP_PASS` 类似，但当前 CPU 不用去做将包送到内核协议栈的准备工作（分配 `skb`，初始化等等），这部分开销还是很大的**。

`XDP_REDIRECT` 返回码还可以和 BPF cpumap 一起使用，对那些目标是本机协议栈、 将由 non-XDP 的远端（remote）CPU 处理的包进行负载均衡。

------

