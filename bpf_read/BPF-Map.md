# ebpf-maps

## API对比

|         | kernel                                                       | user                                                         |      |
| ------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ---- |
| 创建map | 1. SEC("maps") <br/>2. sys_bpf(BPF_MAP_CREATE, ...？？？？？ | int fd = bpf(BPF_MAP_CREATE, ...                             |      |
| 查找    | bpf_map_lookup_elem(&map, key)<br/>//返回查找结果的指针，为空表示不存在 | bpf_map_lookup_elem(map_fd, &k, &v);<br/>//返回值0表示查找成功 |      |
| 插入    | bpf_map_update_elem(&map, key, &val, BPF_NOEXIST)            |                                                              |      |
| 遍历    |                                                              | bpf_map_get_next_key(map_fd, &k1, &k2)<br/>//返回为-1表示遍历结束 |      |
|         |                                                              |                                                              |      |
|         |                                                              |                                                              |      |
|         |                                                              |                                                              |      |
|         |                                                              |                                                              |      |
|         |                                                              | bpf_prog_load                                                |      |



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

