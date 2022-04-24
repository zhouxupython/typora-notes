# ebpf-maps

## c api对比

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



------

## 创建map

### bpf(BPF_MAP_CREATE

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

### SEC("maps")

```c
// 简化版创建map
struct bpf_map_def SEC("maps") my_bpf_map = {
  .type       = BPF_MAP_TYPE_HASH, 
  .key_size   = sizeof(int),
  .value_size   = sizeof(int),
  .max_entries = 100,
  .map_flags   = BPF_F_NO_PREALLOC,
};
```

简化版看起来就是一个BPF Map声明，它是如何做到声明即创建的呢？

关键点就是`SEC("maps")`，学名**ELF惯例格式（ELF convention）**，它的工作原理是这样的：

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

```c
// 感觉这三行还是在用户空间执行的阿？ syscall才能到内核空间？？？？？？？？？？？？？？？？？？？？？？？？
int bpf_create_map_node()
sys_bpf(BPF_MAP_CREATE, &attr, sizeof(attr));
syscall(__NR_bpf, cmd, attr, size);
```



因此在bpf sample文件夹下的程序可以直接使用这两个变量，作为对于BPF程序和BPF Map的引用。

- `prog_fd`是一个数组，在加载内核空间BPF程序时，一旦fd生成后，就[添加到这个数组中](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L111)去；

- `map_fd`也是一个数组，在运行上文提到的[`load_maps()`](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L212)函数时，一旦完成创建BPF Map系统调用生成fd后，同样会[添加到这个数组中](https://elixir.bootlin.com/linux/v4.15/source/samples/bpf/bpf_load.c#L242)去。 

    

------

## map遍历

**模板**

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

**举例**

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

## map-api in kernel

### 从用户态到内核态的系统调用

  ```c
//linux-5.14.14/tools/lib/bpf/bpf.h
//linux-5.14.14/tools/lib/bpf/bpf.c
int bpf_map_lookup_elem(int fd, const void *key, void *value)
{
	union bpf_attr attr;
	int ret;

	memset(&attr, 0, sizeof(attr));
	attr.map_fd = fd;
	attr.key = ptr_to_u64(key);
	attr.value = ptr_to_u64(value);

	ret = sys_bpf(BPF_MAP_LOOKUP_ELEM, &attr, sizeof(attr));
	return libbpf_err_errno(ret);
}

//linux-5.14.14/tools/lib/bpf/bpf.c
static inline int sys_bpf(enum bpf_cmd cmd, union bpf_attr *attr,
			  unsigned int size)
{
	return syscall(__NR_bpf, cmd, attr, size);
}

//------------------------------ K ------------------------------
//linux-5.14.14/kernel/bpf/syscall.c
//@@@@@@@@@@@@@
SYSCALL_DEFINE3(bpf, int, cmd, union bpf_attr __user *, uattr, unsigned int, size)
{
	return __sys_bpf(cmd, USER_BPFPTR(uattr), size);
}

//linux-5.14.14/kernel/bpf/syscall.c
static int __sys_bpf(int cmd, bpfptr_t uattr, unsigned int size)
{
	union bpf_attr attr;// 用户空间传来的uattr，映射到内核空间的attr
	int err;

	if (copy_from_bpfptr(&attr, uattr, size) != 0)
		return -EFAULT;

	switch (cmd) {

	case BPF_MAP_LOOKUP_ELEM:
		err = map_lookup_elem(&attr);
		break;

}

//linux-5.14.14/kernel/bpf/syscall.c
static int map_lookup_elem(union bpf_attr *attr)
{

	int ufd = attr->map_fd;

	f = fdget(ufd);
	map = __bpf_map_get(f);

	value_size = bpf_map_value_size(map);

	err = -ENOMEM;
	value = kmalloc(value_size, GFP_USER | __GFP_NOWARN);
	if (!value)
		goto free_key;

	err = bpf_map_copy_value(map, key, value, attr->flags);// 查找到的结果会拷贝给value

	err = -EFAULT;
	if (copy_to_user(uvalue, value, value_size) != 0)// 再将结果返回给用户空间，完成整个系统调用
		goto free_value;

}

static int bpf_map_copy_value(struct bpf_map *map, void *key, void *value,
			      __u64 flags)
{
	void *ptr;
	int err;

    if (map->ops->map_lookup_elem_sys_only)
        ptr = map->ops->map_lookup_elem_sys_only(map, key);
    else
        ptr = map->ops->map_lookup_elem(map, key);
	
    copy_map_value(map, value, ptr);// 查找到的结果会拷贝给value

	return err;
}

// include/linux/bpf.h
/* copy everything but bpf_spin_lock */
static inline void copy_map_value(struct bpf_map *map, void *dst, void *src)
{
	if (unlikely(map_value_has_spin_lock(map))) {
		u32 off = map->spin_lock_off;

		memcpy(dst, src, off);
		memcpy(dst + off + sizeof(struct bpf_spin_lock),
		       src + off + sizeof(struct bpf_spin_lock),
		       map->value_size - off - sizeof(struct bpf_spin_lock));
	} else {
		memcpy(dst, src, map->value_size);
	}
}
  ```

`map->ops->map_lookup_elem(map, key);`

各种 bpf_map_ops都会实现map_lookup_elem，当前map类型决定使用哪种实现所以要看各种map自己的实现，比如 `kernel/bpf/`路径下的各种map

array map

```c
// linux-5.14.14/kernel/bpf/arraymap.c
const struct bpf_map_ops array_map_ops = {
	.map_meta_equal = array_map_meta_equal,
	.map_alloc_check = array_map_alloc_check,
	.map_alloc = array_map_alloc,
	.map_free = array_map_free,
	.map_get_next_key = array_map_get_next_key,
	.map_lookup_elem = array_map_lookup_elem,// @@@
	.map_update_elem = array_map_update_elem,
	.map_delete_elem = array_map_delete_elem,
	.map_gen_lookup = array_map_gen_lookup,
	.map_direct_value_addr = array_map_direct_value_addr,
	.map_direct_value_meta = array_map_direct_value_meta,
	.map_mmap = array_map_mmap,
	.map_seq_show_elem = array_map_seq_show_elem,
	.map_check_btf = array_map_check_btf,
	.map_lookup_batch = generic_map_lookup_batch,
	.map_update_batch = generic_map_update_batch,
	.map_set_for_each_callback_args = map_set_for_each_callback_args,
	.map_for_each_callback = bpf_for_each_array_elem,
	.map_btf_name = "bpf_array",
	.map_btf_id = &array_map_btf_id,
	.iter_seq_info = &iter_seq_info,
};

static void *array_map_lookup_elem(struct bpf_map *map, void *key)
{
	struct bpf_array *array = container_of(map, struct bpf_array, map);
	u32 index = *(u32 *)key;

	if (unlikely(index >= array->map.max_entries))
		return NULL;

	return array->value + array->elem_size * (index & array->index_mask);
}
```

percpu_array

```c
// linux-5.14.14/kernel/bpf/arraymap.c
const struct bpf_map_ops percpu_array_map_ops = {
	.map_meta_equal = bpf_map_meta_equal,
	.map_alloc_check = array_map_alloc_check,
	.map_alloc = array_map_alloc,
	.map_free = array_map_free,
	.map_get_next_key = array_map_get_next_key,
	.map_lookup_elem = percpu_array_map_lookup_elem,// @@@
	.map_update_elem = array_map_update_elem,
	.map_delete_elem = array_map_delete_elem,
	.map_seq_show_elem = percpu_array_map_seq_show_elem,
	.map_check_btf = array_map_check_btf,
	.map_lookup_batch = generic_map_lookup_batch,
	.map_update_batch = generic_map_update_batch,
	.map_set_for_each_callback_args = map_set_for_each_callback_args,
	.map_for_each_callback = bpf_for_each_array_elem,
	.map_btf_name = "bpf_array",
	.map_btf_id = &percpu_array_map_btf_id,
	.iter_seq_info = &iter_seq_info,
};

/* Called from eBPF program */
static void *percpu_array_map_lookup_elem(struct bpf_map *map, void *key)
{
	struct bpf_array *array = container_of(map, struct bpf_array, map);
	u32 index = *(u32 *)key;

	if (unlikely(index >= array->map.max_entries))
		return NULL;

	return this_cpu_ptr(array->pptrs[index & array->index_mask]);
}

```

hash map

```c
// linux-5.14.14/kernel/bpf/hashtab.c
const struct bpf_map_ops htab_map_ops = {
	.map_meta_equal = bpf_map_meta_equal,
	.map_alloc_check = htab_map_alloc_check,
	.map_alloc = htab_map_alloc,
	.map_free = htab_map_free,
	.map_get_next_key = htab_map_get_next_key,
	.map_lookup_elem = htab_map_lookup_elem, // @@@
	.map_lookup_and_delete_elem = htab_map_lookup_and_delete_elem,
	.map_update_elem = htab_map_update_elem,
	.map_delete_elem = htab_map_delete_elem,
	.map_gen_lookup = htab_map_gen_lookup,
	.map_seq_show_elem = htab_map_seq_show_elem,
	.map_set_for_each_callback_args = map_set_for_each_callback_args,
	.map_for_each_callback = bpf_for_each_hash_elem,
	BATCH_OPS(htab),
	.map_btf_name = "bpf_htab",
	.map_btf_id = &htab_map_btf_id,
	.iter_seq_info = &iter_seq_info,
};

static void *htab_map_lookup_elem(struct bpf_map *map, void *key)
{
	struct htab_elem *l = __htab_map_lookup_elem(map, key);

	if (l)
		return l->key + round_up(map->key_size, 8);

	return NULL;
}

/* Called from syscall or from eBPF program directly, so
 * arguments have to match bpf_map_lookup_elem() exactly.
 * The return value is adjusted by BPF instructions
 * in htab_map_gen_lookup().
 */
static void *__htab_map_lookup_elem(struct bpf_map *map, void *key)
{
	struct bpf_htab *htab = container_of(map, struct bpf_htab, map);
	struct hlist_nulls_head *head;
	struct htab_elem *l;
	u32 hash, key_size;

	WARN_ON_ONCE(!rcu_read_lock_held() && !rcu_read_lock_trace_held() &&
		     !rcu_read_lock_bh_held());

	key_size = map->key_size;

	hash = htab_map_hash(key, key_size, htab->hashrnd);

	head = select_bucket(htab, hash);

	l = lookup_nulls_elem_raw(head, hash, key, key_size, htab->n_buckets);

	return l;
}
```

等等

------

### 内核态内部使用的helper函数

```c
// kernel/bpf/helpers.c
// 内核态，返回查找结果的指针，为空表示不存在
void *bpf_map_lookup_elem(struct bpf_map *map, void *key)
    
// 用户态，bpf_map_lookup_elem(map_fd, &k, &v);  返回值0表示查找成功

/* If kernel subsystem is allowing eBPF programs to call this function,
 * inside its own verifier_ops->get_func_proto() callback it should return
 * bpf_map_lookup_elem_proto, so that verifier can properly check the arguments
 *
 * Different map implementations will rely on rcu in map methods
 * lookup/update/delete, therefore eBPF programs must run under rcu lock
 * if program is allowed to access maps, so check rcu_read_lock_held in
 * all three functions.
 */
BPF_CALL_2(bpf_map_lookup_elem, struct bpf_map *, map, void *, key)
{
	WARN_ON_ONCE(!rcu_read_lock_held() && !rcu_read_lock_bh_held());
	return (unsigned long) map->ops->map_lookup_elem(map, key);
}

const struct bpf_func_proto bpf_map_lookup_elem_proto = {
	.func		= bpf_map_lookup_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_PTR_TO_MAP_VALUE_OR_NULL,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_KEY,
};

const struct bpf_func_proto *
bpf_base_func_proto(enum bpf_func_id func_id)
{
	switch (func_id) {
	case BPF_FUNC_map_lookup_elem:
		return &bpf_map_lookup_elem_proto;// @@@
	case BPF_FUNC_map_update_elem:
		return &bpf_map_update_elem_proto;

//bpf_map_update_elem
BPF_CALL_4(bpf_map_update_elem, struct bpf_map *, map, void *, key,
	   void *, value, u64, flags)
{
	WARN_ON_ONCE(!rcu_read_lock_held() && !rcu_read_lock_bh_held());
	return map->ops->map_update_elem(map, key, value, flags);
}

const struct bpf_func_proto bpf_map_update_elem_proto = {
	.func		= bpf_map_update_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_KEY,
	.arg3_type	= ARG_PTR_TO_MAP_VALUE,
	.arg4_type	= ARG_ANYTHING,
};
            
// kernel/bpf/syscall.c
const struct bpf_func_proto * __weak
tracing_prog_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	return bpf_base_func_proto(func_id);
}

```

