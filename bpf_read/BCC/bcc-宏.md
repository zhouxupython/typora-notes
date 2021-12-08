### BPF_PERF_OUTPUT

```c
BPF_PERF_OUTPUT(open_events);

open_events.perf_submit(ctx, &evt, sizeof(evt));

event = b["open_events"].event(data)
    
b["open_events"].open_perf_buffer(print_event)
```

<font title="gray">K</font>：

`event_data_t`：自定义结构体，用于K-U通信，且不存在参数数量和数据大小等限制`BPF_PERF_OUTPUT`(==open_events==)用于追踪函数之间的==隔离==，以及向用户空间==发布==event_data_t

`open_events.perf_submit`用于将 event_data_t 数据==发送==至用户空间



<font title="gray">U</font>：

b["==open_events=="].`open_perf_buffer`(<font style="background-color:#8bc34a">print_event</font>)：将用户空间接收的"==open_events=="数据与数据的处理函数(print_event)==关联==起来

`b.perf_buffer_poll`：==轮询==，有数据就使用处理函数(<font style="background-color:#8bc34a">print_event</font>)进行处理

b["==open_events=="].`event`(data):从data中还原出钩子函数中定义的结构体event_data_t



### 钩子命名约定



还有一种简便的使用方式，声明函数的时候使用特定的前缀和函数名，此种约定就可以省略 `b.attach_kprobe` 显示的使用，例如：

```python
prog = """
int syscall__open(struct pt_regs *ctx, const char __user *filename, int flags) {
	// ...
}
"""

// 上述按照特定格式约定了，此处的 attach_kprobe 就不再需要调用
// b.attach_kprobe(event=b.get_syscall_fnname("open"), fn_name="trace_syscall_open")
```

函数名的组成为 ”类型“ + 内核函数的方式，`syscall`，表示类型是 `syscall`，跟踪的函数是 `open`，需要注意的是 `syscall` 与 `open` 之间为**两个连续的下划线**。相对应的类型还有 `kprobe/kretprobe` 等。详情参见[这里](https://github.com/iovisor/bcc/blob/master/docs/reference_guide.md#8-system-call-tracepoints)。



### BPF_HASH

```c
// 定义HASH_MAP，使用 BCC 宏定义，key 为 u64 类型，value 为 struct val_t 结构；
BPF_HASH(infotmp, u64, struct val_t);

infotmp.update(&id, &val);  // 保存中间结果至 hash_map 中,以 id 为 key，将 val 对象结果保存至 infotmp 中；

// 用于读取在map中保存的信息，如果未查询到则直接返回，
// 需要注意的是 lookup 函数的入参和出参都是指针类型，使用前需要判断；
valp = infotmp.lookup(&id); // 从 hash_map 中获取到  sys_open 函数保存的中间数据
if (valp == 0) {
    // missed entry
    return 0;
}

infotmp.delete(&id);  // 删除这个k-v pair
```





### PT_REGS_RC

```python
b.attach_kretprobe(event=b.get_syscall_fnname("open"), fn_name="trace_syscall_open_return")
```

```c
// 使用宏 PT_REGS_RC 从 ctx 字段中读取本次函数跟踪的返回值；
int trace_syscall_open_return(struct pt_regs *ctx)
{
    evt.ret = PT_REGS_RC(ctx); // 读取结果值
    
    ......
    return 0;
}
```

```shell
zhouxu@zhouxu:~$ sudo cat /sys/kernel/debug/tracing/events/syscalls/sys_exit_open/format
name: sys_exit_open
ID: 624
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int __syscall_nr;	offset:8;	size:4;	signed:1;
	field:long ret;	offset:16;	size:8;	signed:1;

print fmt: "0x%lx", REC->ret

```



### TRACEPOINT_PROBE

```python
TRACEPOINT_PROBE(syscalls,sys_enter_open)
```

代替：

```python
int trace_syscall_open(struct pt_regs *ctx, const char __user *filename, int flags)

b.attach_kprobe(event=b.get_syscall_fnname("open"), fn_name="trace_syscall_open")
```



### PT_REGS_IP



### BPF_STACK_TRACE
