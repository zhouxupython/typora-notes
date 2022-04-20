# ebpf-maps

## API对比

|         | kernel                                                       | user                                                         |
| ------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 创建map | 1. struct {} `my_map` ***SEC***("maps") <br/>2. sys_bpf(BPF_MAP_CREATE, ...？？？？？ | 1. int `map_fd` = ***bpf***(BPF_MAP_CREATE, ...<br/>2. `map_fd` = ***bpf_create_map***(BPF_MAP_TYPE_x, sizeof(key), sizeof(value), 256, 0); |
| map fd  | K可以直接使用 `my_map`<br/>或者**全局变量**==map_fd==[n].*fd* | struct bpf_object *obj;<br/>int map_fd, prog_fd;<br/>**bpf_prog_load**("xxx_kern.o", BPF_PROG_TYPE_xxx, &*obj, &prog_fd)<br/>`map_fd` = ***bpf_object__find_map_fd_by_name***(obj, "`my_map`"); |
| 查找    | ***bpf_map_lookup_elem***(&map, key)<br/>//返回查找结果的指针，为空表示不存在（kernel/bpf/helpers.c） | ***bpf_map_lookup_elem***(map_fd, &k, &v);<br/>//返回值0表示查找成功 |
| 插入    | ***bpf_map_update_elem***(&map, key, &val, BPF_NOEXIST)<br/>(kernel/bpf/helpers.c) | int ***bpf_map_update_elem***(int fd, const void *key, const void *value, __u64 flags); |
| 遍历    |                                                              | ***bpf_map_get_next_key***(map_fd, &k1, &k2)<br/>//返回为-1表示遍历结束 |
| 删除    | bpf_map_delete_elem                                          |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              | bpf_object * bpf_object__open_file(const char *path, const struct bpf_object_open_opts *opts); |
|         |                                                              | /* Load object into kernel */<br/>int bpf_object__load(struct bpf_object *obj); |
|         |                                                              | /* unload object from kernel */<br/>int bpf_object__unload(struct bpf_object *obj); |
|         |                                                              | bpf_link * bpf_program__attach(struct bpf_program *prog);    |
|         |                                                              | int bpf_link__destroy(struct bpf_link *link);                |
|         |                                                              | void bpf_object__close(struct bpf_object *object);           |
|         |                                                              |                                                              |
|         |                                                              | bpf_object__for_each_program(bpf_program *prog, bpf_object *obj) |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         |                                                              | libbpf_get_error                                             |
|         |                                                              |                                                              |
|         |                                                              |                                                              |
|         | &map, 是**struct** bpf_map_def SEC("maps")  `map` = {}       | 上面都是libbpf的api，基本上最后都会进入bpf系统调用           |



```c
#这个是系统调用，是在用户空间使用的，当cmd是BPF_MAP_CREATE时，返回的是内核创建的ebpf-map在用户空间对应的fd 
#include <linux/bpf.h>
int bpf(int cmd, union bpf_attr *attr, unsigned int size);

union bpf_attr my_map_attr {
  .map_type = BPF_MAP_TYPE_ARRAY,
  .key_size = sizeof(int),
  .value_size = sizeof(int),
  .max_entries = 1024,
  .map_flags = BPF_F_NO_PREALLOC,
};

int fd = bpf(BPF_MAP_CREATE, &my_map_attr, sizeof(my_map_attr));
```



```
#简化版创建map
struct bpf_map_def SEC("maps") my_bpf_map = {
  .type       = BPF_MAP_TYPE_HASH, 
  .key_size   = sizeof(int),
  .value_size   = sizeof(int),
  .max_entries = 100,
  .map_flags   = BPF_F_NO_PREALLOC,
};
```



简化版看起来就是一个BPF Map声明，它是如何做到声明即创建的呢？关键点就是`SEC("maps")`，学名**ELF惯例格式（ELF convention）**，它的工作原理是这样的：

1. 声明ELF Section属性 `SEC("maps")` （之前的[博文](https://davidlovezoe.club/wordpress/archives/937#设计你的第一个XDP程序)里有对Section作用的描述）
2. 内核代码[`bpf_load.c`](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c)respect目标文件中所有Section信息，它会扫描目标文件里定义的Section，其中就有用来创建BPF Map的`SEC("maps")`，
3. 我们可以到[相关代码](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.h#L41)里看到说明：

```c
// https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.h#L41
/* parses elf file compiled by llvm .c->.o
 * . parses 'maps' section and creates maps via BPF syscall // 就是这里
 * . parses 'license' section and passes it to syscall
 * . parses elf relocations for BPF maps and adjusts BPF_LD_IMM64 insns by
 *   storing map_fd into insn->imm and marking such insns as BPF_PSEUDO_MAP_FD
 * . loads eBPF programs via BPF syscall
 *
 * One ELF file can contain multiple BPF programs which will be loaded
 * and their FDs stored stored in prog_fd array
 *
 * returns zero on success
 */
int load_bpf_file(char *path);
```



1. [`bpf_load.c`](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c)扫描到`SEC("maps")`后，对BPF Map相关的操作是由[`load_maps`](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L212)函数完成，其中的[`bpf_create_map_node()`](https://elixir.bootlin.com/linux/v4.15/source/tools/lib/bpf/bpf.c#L62)和[`bpf_create_map_in_map_node()`](https://elixir.bootlin.com/linux/v4.15/source/tools/lib/bpf/bpf.c#L101)就是创建BPF Map的关键函数，它们背后都是调用了定义在内核代码[tools/lib/bpf/bpf.c](https://elixir.bootlin.com/linux/v4.15/source/tools/lib/bpf/bpf.c)中的方法，而[这个方法](https://elixir.bootlin.com/linux/v4.15/source/tools/lib/bpf/bpf.c#L83)就是使用上文提到的`BPF_MAP_CREATE`命令进行的系统调用。
2. 最后在编译程序时，通过添加`bpf_load.o`作为依赖库，并合并为最终的可执行文件中，这样在程序运行起来时，就可以通过声明`SEC("maps")`即可完成创建BPF Map的行为了。

从上面梳理的过程可以看到，这个简化版虽然使用了“语法糖”，但最后还是会去使用bpf()函数完成系统调用。

```
#感觉这三行还是在用户空间执行的阿？ syscall才能到内核空间？？？？？？？？？？？？？？？？？？？？？？？？
int bpf_create_map_node()
sys_bpf(BPF_MAP_CREATE, &attr, sizeof(attr));
syscall(__NR_bpf, cmd, attr, size);
```







因此在bpf sample文件夹下的程序可以直接使用这两个变量，作为对于BPF程序和BPF Map的引用。

- `prog_fd`是一个数组，在加载内核空间BPF程序时，一旦fd生成后，就[添加到这个数组中](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L111)去；

- `map_fd`也是一个数组，在运行上文提到的[`load_maps()`](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L212)函数时，一旦完成创建BPF Map系统调用生成fd后，同样会[添加到这个数组中](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L242)去。 

    

map遍历

```c
int result;
struct map_key next_key;
struct map_key lookup_key = {none-sense, none-sense};
struct map_value value = {};
while (1)//或者遍历多个map
{
    ......
    // retrieve the bpf map of statistics
    /*
    这个内层循环用于遍历一个map
    通过将lookup_key置为不可用，第一次执行while时，会获取map的第一个key，然后存入next_key；
    使用next_key就可以lookup对应的val;
    然后将next_key赋给lookup_key，再次while时，又会获取到的第二个key，存入next_key
    依次执行，直到while处返回-1，表示迭代结束
    */
    while (bpf_map_get_next_key(map_fd, &lookup_key, &next_key) != -1)
    {
        .......
        result = bpf_map_lookup_elem(map_fd, &next_key, &value);
        if (result == 0)
        {
            // success
            ......value
        }
        else
        {
            printf("Failed to read value from the map: %d (%s)\n", result, strerror(errno));
        }

        // prepare to get next key
        lookup_key = next_key;
        ......
    }
    
    // reset the lookup key for a fresh start
    
}
```



```c
int result;
struct pair next_key;
struct pair lookup_key = {0, 0};
struct stats value = {};
while (1)
{
    sleep(2);
    // retrieve the bpf map of statistics
    while (bpf_map_get_next_key(map_fd[0], &lookup_key, &next_key) != -1)
    {
        //printf("The local ip of next key in the map is: '%d'\n", next_key.src_ip);
        //printf("The remote ip of next key in the map is: '%d'\n", next_key.dest_ip);
        struct in_addr local = {next_key.src_ip};
        struct in_addr remote = {next_key.dest_ip};
        printf("The local ip of next key in the map is: '%s'\n", inet_ntoa(local));
        printf("The remote ip of next key in the map is: '%s'\n", inet_ntoa(remote));

        // get the value via the key
        // TODO: change to assert
        // assert(bpf_map_lookup_elem(map_fd[0], &next_key, &value) == 0)
        result = bpf_map_lookup_elem(map_fd[0], &next_key, &value);
        if (result == 0)
        {
            // print the value
            printf("rx_cnt value read from the map: '%llu'\n", value.rx_cnt);
            printf("rx_bytes value read from the map: '%llu'\n", value.rx_bytes);
        }
        else
        {
            printf("Failed to read value from the map: %d (%s)\n", result, strerror(errno));
        }
        lookup_key = next_key;
        printf("\n\n");
    }
    printf("start a new loop...\n");
    // reset the lookup key for a fresh start
    lookup_key.src_ip = 0;
    lookup_key.dest_ip = 0;
}
```



------

## map定义

```c
// 定义HASH_MAP，使用 BCC 宏定义，key 为 u64 类型，value 为 struct val_t 结构；
BPF_HASH(infotmp, u64, struct val_t);
```

![](sock_example_bcc版本实现.jpg)

typora-notes/bpf_read/BCC/bcc-宏.md

### BPF_HASH

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





### BPF_ARRAY

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

### BPF_MAP_TYPE_PERF_EVENT_ARRAY

k含义：u32，可以为cpu个数

v含义：u32，作为perf event buffer从内核往用户空间传递数据时，是 SYS_PERF_EVENT_OPEN系统调用的返回值，表示一个fd。

​             参见bpf_read/PerfBuffer_RingBuffer.md

```go
func CreatePerfMap(mapPath string, maxEntries uint32) (int, error) {
	var key, val uint32
	cfg := MapConfig{
		MapType:    BPF_MAP_TYPE_PERF_EVENT_ARRAY,
		KeySize:    unsafe.Sizeof(key),
		ValSize:    unsafe.Sizeof(val),
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





