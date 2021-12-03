# 1

https://github.com/iovisor/bcc/blob/master/docs/tutorial.md

https://github.com/iovisor/bcc/blob/master/docs/tutorial_bcc_python_developer.md

















[【BPF入门系列-7】使用 ebpf 实时持续跟踪进程文件记录](https://www.ebpf.top/post/ebpf_trace_file_open/)

open函数原型是：

```c
int open(const char *pathname, int flags);
```

在open函数上添加钩子`trace_syscall_open`，该钩子的入参，除了常规的`struct pt_regs *ctx`，

还需要有open函数的两个入参，即``打开的文件名``和`flag`

```python
#!/usr/bin/python
from bcc import BPF

prog = """
int trace_syscall_open(struct pt_regs *ctx, const char __user *filename, int flags) {
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    u32 uid = bpf_get_current_uid_gid();

    bpf_trace_printk("%d [%s]\\n", pid, filename);
    return 0;
}
"""

b = BPF(text=prog)
b.attach_kprobe(event=b.get_syscall_fnname("open"), fn_name="trace_syscall_open")
try:
    b.trace_print()
except KeyboardInterrupt:
    exit()
```



再比如，这里有一个 [名为 disksnoop 的 BPF 程序](https://github.com/iovisor/bcc/blob/0c8c179fc1283600887efa46fe428022efc4151b/examples/tracing/disksnoop.py)

```python
BPF_HASH(start, struct request *);
void trace_start(struct pt_regs *ctx, struct request *req) {
    // stash start timestamp by request ptr
    u64 ts = bpf_ktime_get_ns();
    start.update(&req, &ts);
}
...
b.attach_kprobe(event="blk_start_request", fn_name="trace_start")
b.attach_kprobe(event="blk_mq_start_request", fn_name="trace_start")
```

挂载到内核函数blk_mq_start_request，该函数原型是：

```c
//include/linux/blk-mq.h:522:
void blk_mq_start_request(struct request *rq);
```

钩子函数的入参除了常规的`struct pt_regs *ctx`，还需要有`blk_mq_start_request`的入参`struct request *rq`























