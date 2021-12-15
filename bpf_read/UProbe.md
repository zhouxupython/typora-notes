# uprobe&uret_probe

[【译】eBPF 概述：第 5 部分：跟踪用户进程](https://www.ebpf.top/post/ebpf-overview-part-5/)

[libstapsdt](https://github.com/sthima/libstapsdt)

ebpf-experiments/1_bcc/2_libstapsdt

自己画得一个程序图



USDT_libstapsdt_demo.py

```python
from bcc import BPF, USDT

u = USDT(pid=int(sys.argv[1]))#pid是server.py执行时的进程号
u.enable_probe(probe="probe_i_j", fn_name="trace_demo_i_j")#！！！USDT probe，探针file_transfer 关联一个钩子函数，跟踪点作为钩子函数的入参
b = BPF(text=bpf, usdt_contexts=[u])

b["events"].open_perf_buffer(print_event)
```





bcc/tools/gethostlatency.py

```python
#name="c"表示libc库
b = BPF(text=bpf_text)
b.attach_uprobe(name="c", sym="getaddrinfo", fn_name="do_entry", pid=args.pid)

b.attach_uretprobe(name="c", sym="getaddrinfo", fn_name="do_return", pid=args.pid)

b["events"].open_perf_buffer(print_event)
```

