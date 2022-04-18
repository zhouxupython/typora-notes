



**cilium/ebpf**

## perf buffer

1   写的时候是在ebpf prog程序中

```c
struct map events __section("maps") = {
    .type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,          PerfEventArray
};

__section("xdp") 
int output_single(void *ctx) {
    unsigned char buf[] = {     // buf写入events
        1, 2, 3, 4, 5
                          };

    return perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &buf[0], 5);
}
```

2  用户态接收       每个cpu对应一个 perf event事件fd，以及一个缓冲区

```c
//1   fd是一个perf event事件
fd, err := unix.PerfEventOpen(&attr, -1, cpu, -1, unix.PERF_FLAG_FD_CLOEXEC)


//2   通过mmap，fd与一块内存勾搭
mmap, err := unix.Mmap(fd, 0, perfBufferSize(perCPUBuffer), unix.PROT_READ|unix.PROT_WRITE, unix.MAP_SHARED)


//3  fd通过epoll监控io事件
unix.EpollCtl(p.epollFd, unix.EPOLL_CTL_ADD, fd, &event)

//4  array是PerfEventArray类型的map，fd加入
pr.array.Put(uint32(i), uint32(fd))

events, err := ebpf.NewMap(&ebpf.MapSpec{
	Type: ebpf.PerfEventArray,
})
    
//Put对照下面这个，可以看到 PerfEventArray类型的map， 在这里 key是一个fd的index，value是fd
///home/zhouxu/works/ebpf/bpf-ringbuf-examples/src/perfbuf-output.bpf.c    /* BPF perfbuf map */
struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(int));
	__uint(value_size, sizeof(int));
} pb SEC(".maps");

//5
unix.EpollWait(p.epollFd, events, -1)
```





buf ----perf_event_output ---》 events --- 哪个cpu写的，就找到对应的fd  ---》 往用户态通知有IO事件，那么epoll.wait会返回 ---》 用户态感知到有IO事件，找到对应的fd，就能找到对应的mmap ---》 读取数据



看一下写入端代码，就知道为什么需要将每个cpu对应的fd写入PerfEventArray类型的map了。

（1）perf event map插入的时候，pr.array.Put(uint32(i), uint32(fd))       

在这里array是PerfEventArray类型的map， key是一个fd的index，即cpu的编号，而value是fd，fd是对应一个打开的perf event事件的。

（2）output数据写入的时候
static int (*perf_event_output)(void *, struct bpf_map *, int, void *, unsigned long)
perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, 

events表示PerfEventArray类型的map，BPF_F_CURRENT_CPU表示当前cpu的编号，所以通过编号可以获取对应的fd，也就能找到这个fd对应的mmap，就可以知道往这个地址进行写入output数据就可以了。
这个fd又被epoll监控，那么此时就能往用户态通知有IO事件发生，整个路子就通了。




------------------------------------------------------
------------------------------------------------------
------------------------------------------------------

## ringbuf

1   写的时候是在ebpf prog程序中

```c
events, err := ebpf.NewMap(&ebpf.MapSpec{
    Type:       ebpf.RingBuf,
    MaxEntries: 4096,              // 表示ringbuf大小
})
```

2  用户态接收

```c
//1    fd是 上面这个map的fd     这个fd用于监控，因为ringbuf没有别的fd了
unix.EpollCtl(p.epollFd, unix.EPOLL_CTL_ADD, fd, &event);

//2    mapFD同上
cons, err := unix.Mmap(mapFD, 0, os.Getpagesize(), unix.PROT_READ|unix.PROT_WRITE, unix.MAP_SHARED)
	
prod, err := unix.Mmap(mapFD, (int64)(os.Getpagesize()), os.Getpagesize()+2*size, unix.PROT_READ, unix.MAP_SHARED)
    
//3 
unix.EpollWait(p.epollFd, events, -1)
```

