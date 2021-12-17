# SEC

samples/bpf/tracex4_kern.c

K

==SEC("kprobe/kmem_cache_free")==

int bpf_prog1(struct pt_regs *ctx)

==SEC("kretprobe/kmem_cache_alloc_node")==

int bpf_prog2(struct pt_regs *ctx)



U

struct bpf_link *links[2];

struct bpf_object *obj;

struct bpf_program *prog;

int j = 0;

obj = bpf_object__open_file("xxx_kern.o", NULL);

bpf_object__load(obj);

bpf_object__for_each_program(prog, obj) {

​        links[j] = bpf_program__attach(prog);

​        j++;

}

因为SEC中指定了attach的类型(kprobe、kretprobe)和挂载点(kmem_cache_free、kmem_cache_alloc_node)，

所以U中没有再指定。

prog依次是bpf_prog1和bpf_prog2，attach后对应挂载钩子函数

------

samples/bpf/sockex1_kern.c

K

==SEC("socket1")==

U

struct bpf_object *obj;

int prog_fd;

bpf_prog_load("xxx_kern.o", BPF_PROG_TYPE_SOCKET_FILTER, &obj, &prog_fd)

int sock = open_raw_sock("lo");

setsockopt(sock, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd, sizeof(prog_fd))

SEC中没有指定attach类型和挂载点，所以使用api挂载时需要指定；因为是socket的钩子函数，用setsockopt挂载。



------

K

==SEC(".maps")==

struct {

​    __uint(type, BPF_MAP_TYPE_ARRAY);

​    __type(key, u32);

​    __type(value, long);

​    __uint(max_entries, 256);

} my_map SEC(".maps");





U

struct bpf_object *obj;

int map_fd, prog_fd;

bpf_prog_load("xxx_kern.o", BPF_PROG_TYPE_SOCKET_FILTER, &obj, &prog_fd)

map_fd = bpf_object__find_map_fd_by_name(obj, "my_map");





struct bpf_object *obj;

int map_fd；

obj = bpf_object__open_file("xxx_kern.o", NULL);

bpf_object__load(obj)

map_fd = bpf_object__find_map_fd_by_name(obj, "my_map");



------





