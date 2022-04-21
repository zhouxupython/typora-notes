# bpftool

[上手 bpftool](https://blog.csdn.net/Longyu_wlz/article/details/109931993)

[bpftool安装教程](https://blog.csdn.net/weixin_44260459/article/details/123036982)	apt

https://github.com/cloudflare/bpftools



## bpftool 用法简介

http://tinylab.org/ebpf-part1/

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
```



bpftool 在内核的 `tools/bpf/bpftool/` 目录下，使用 make 编译就可使用，查看当前运行的 bpf 程序，如下所示，可以看到当前运行的是 `kprobe event` 还有 `map id`：

```shell
wu@ubuntu:~/linux/samples/bpf$ sudo bpftool prog show
[sudo] password for wu:
... ...
205: kprobe  tag a6cfc4a29f52a193  gpl
	loaded_at 2021-01-19T11:51:26+0000  uid 0
	xlated 72B  jited 62B  memlock 4096B  map_ids 72
206: kprobe  tag d16c41919f3b767a  gpl
	loaded_at 2021-01-19T11:51:26+0000  uid 0
	xlated 192B  jited 119B  memlock 4096B	map_ids 72
```

执行`bpftool prog dump xlated id xxx` 导出ebpf程序的字节码

执行`bpftool prog dump jited id xxx` 导出机器码

------

查看 map 的id，可以看到当前使用的是 hash map：

```shell
wu@ubuntu:~/linux$ sudo bpftool map show
72: hash  name my_map  flags 0x0
	key 8B	value 16B  max_entries 1000000	memlock 88788992B
```

这里看到的也是value是16B



查看 map 的所有的内容，并查看对应 key 的value：

```shell
wu@ubuntu:~/linux$ sudo bpftool map dump id 72
key: 80 35 43 e5 26 95 ff ff  value: b9 f3 f3 9e 87 6b 00 00  9a 9a c7 86 ff ff ff ff
key: 80 9c 5f f6 25 95 ff ff  value: 98 85 ac c7 8c 6b 00 00  9a 9a c7 86 ff ff ff ff
key: 00 4c 9b e4 25 95 ff ff  value: e6 8d c2 b7 7e 6b 00 00  9a 9a c7 86 ff ff ff ff
key: 80 21 15 f8 26 95 ff ff  value: 60 df 01 fc 5c 6b 00 00  9a 9a c7 86 ff ff ff ff
key: 80 f5 46 5f 26 95 ff ff  value: 5e e1 73 7b 8d 6b 00 00  9a 9a c7 86 ff ff ff ff
... ...


wu@ubuntu:~/linux$ sudo bpftool map lookup id 72 key 0x80 0x35 0x43 0xe5 0x26 0x95 0xff 0xff
key: 80 35 43 e5 26 95 ff ff  value: b9 f3 f3 9e 87 6b 00 00  9a 9a c7 86 ff ff ff ff

```

------

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

