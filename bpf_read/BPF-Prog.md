# ebpf prog

## 收集

[BPF程序类型及其原理](https://blog.csdn.net/weixin_41036447/article/details/107817340)

[一文搞懂所有bpf程序分类](https://blog.csdn.net/weixin_41036447/article/details/106473865)

https://blogs.oracle.com/linux/post/bpf-a-tour-of-program-types

每个都需要搞清楚：

（1）用途

（2）SEC()、ebpf prog入口参数

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

（5）实现分析



==是否可以归类，或者进行比较==



按照https://blogs.oracle.com/linux/post/bpf-a-tour-of-program-types分类

## 1. socket-related program types

### BPF_PROG_TYPE_SOCKET_FILTER 

BPF_PROG_TYPE_SOCKET_FILTER是第一个添加到Linux内核的程序类型。将BPF程序附加到原始套接字时，可以访问该套接字处理的所有数据包。Socket Filter Programs==不允许==您修改这些数据包的内容或更改这些数据包的目的地；它们只允许您出于可观察的目的访问这些数据包。程序接收的元数据包含与网络堆栈相关的信息，例如用于传递数据包的协议类型。我们将在第6章详细介绍套接字过滤和其他网络程序。

（1）用途

BPF_PROG_TYPE_SOCKET_FILTER，从宏字面意思比较容易想到实现的是[socket](https://so.csdn.net/so/search?q=socket&spm=1001.2101.3001.7020) filter功能，它区别于sockops和tracepoint等功能，需要额外借助setsockopt能力将功能函数和socket绑定，功能才能真正生效。

（2）SEC()、ebpf prog入口参数

在内核态功能函数中定义==SEC("socketxxxx")==，则会被解析为BPF_PROG_TYPE_SOCKET_FILTER类型功能。

比如内核中实现的三个example程序：
samples/bpf/sockex1_kern.c --->SEC("socket1")
samples/bpf/sockex2_kern.c --->SEC("socket2")
samples/bpf/sockex3_kern.c --->SEC("socket3")

BPF prog 入口参数（R1)：struct __sk_buff

（3）是否可以修改数据包	

不允许

（4）在哪儿有例子（项目、bcc、libbpf、samples）

[linux网络和BPF](https://da1234cao.blog.csdn.net/article/details/115900299)、samples/bpf/sock_example.c、samples/bpf/sockex1-2-3_user.c

```c
setsockopt(sock, SOL_SOCKET, SO_ATTACH_BPF, prog_fd, sizeof(prog_fd[0]))
```

（5）实现分析

[BPF_PROG_TYPE_SOCKET_FILTER 功能实现](https://blog.csdn.net/already_skb/article/details/123027350)

------

### BPF_PROG_TYPE_SK_SKB

程序允许您访问套接字映射和套接字重定向。

------

### BPF_PROG_TYPE_SOCK_OPS

[[译] 利用 ebpf sockmap/redirection 提升 socket 性能（2020）](http://arthurchiao.art/blog/socket-acceleration-with-ebpf-zh/)

[ebpf sockops 功能分析](https://blog.csdn.net/already_skb/article/details/122988446)

[ebpf sockops 代码解读](https://blog.csdn.net/already_skb/article/details/123021389)



------

### BPF_PROG_TYPE_SK_MSG

这些类型的程序允许您控制是否应该传递发送到套接字的消息。

[[译] 利用 ebpf sockmap/redirection 提升 socket 性能（2020）](http://arthurchiao.art/blog/socket-acceleration-with-ebpf-zh/)

------

##  2. tc (traffic control) subsystem programs

------



------

## 3. xdp : the Xpress Data Path.

### BPF_PROG_TYPE_XDP

（1）用途

XDP程序允许您编写在网络数据包到达内核时很早就执行的代码。由于内核本身没有太多时间来处理信息，因此它只公开来自数据包的有限信息集。因为数据包是在早期执行的，所以您对如何处理该数据包有更高级别的控制。XDP程序定义了几个可以控制的操作，这些操作允许您决定如何处理数据包。您可以从XDP程序返回XDP_PASS ，这意味着数据包应该传递到kernel中的下一个子系统。您还可以返回XDP_DROP，这意味着内核应该完全忽略这个数据包，而不做任何其他处理。您还可以返回 XDP_TX，这意味着数据包应该转发回最初接收到数据包的网络接口卡（NIC）。

（2）ebpf prog函数原型、参数含义



**struct** **xdp_md**

（3）是否可以修改数据包	

允许，XDP 层运行的 BPF 程序能够任意修改（mangle）数据包，即使是 BPF 辅助函数都能增加或减少包的 headroom，这样就可以在将包再次发送出去之前，对包进行任何的封装/解封装。

利用 `XDP_TX` 能够实现 hairpinned（发卡）模式的负载均衡器，这种均衡器能够在接收到包的网卡（修改包之后）再次将包发送出去，而 `XDP_REDIRECT` 动作能够将包转发到另一个 网卡然后发送出去。

（4）在哪儿有例子（项目、bcc、libbpf、samples）

typora-notes/bpf_read/samples/xdp1.md



（5）实现分析

------

## 4. kprobes, tracepoints and perf events

### BPF_PROG_TYPE_KPROBE

（1）用途

正如您将在第4章中看到的，我们在其中讨论跟踪，kprobes是可以动态附加到内核中某些调用点的函数。BPF kprobe程序类型允许您将BPF程序用作kprobe处理程序。它们用BPF_PROG_TYPE_KPROBE类型定义的。BPF VM确保kprobe程序始终安全运行，这是传统kprobe模块的一个优势。您仍然需要记住，kprobe在内核中不是稳定的入口点，因此您需要确保kprobe BPF程序与您使用的特定内核版本兼容

（2）SEC()、ebpf prog入口参数

SEC("kprobe/probe的内核函数")

**struct** **pt_regs**

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

linux-5.14.14/samples/bpf/racex6_kern.c

（5）实现分析

------

### BPF_PROG_TYPE_TRACEPOINT

（1）用途

这种类型的程序允许您将BPF程序附加到内核提供的跟踪点处理程序。跟踪点程序是用类型BPF_PROG_TYPE_TRACEPOINT定义的。正如您将在第4章中看到的，跟踪点是内核代码库中的静态标记，允许您为跟踪和调试目的注入任意代码。它们不如kprobes灵活，因为它们需要由内核预先定义，但是在内核中引入它们之后，它们保证是稳定的。当您要调试系统时，这将为您提供更高级别的可预测性。系统中的所有跟踪点都在/sys/kernel/debug/trac-ing/events目录中定义。在那里，您将找到每个子系统，其中包含任何跟踪点，并且您可以将BPF程序附加到这些子系统。

（2）SEC()、ebpf prog入口参数



这类  BPF 的参数比较特殊，根据  tracepoint 位置的不同而不同。

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

（5）实现分析

------

### BPF_PROG_TYPE_PERF_EVENT

这些类型的BPF程序允许您将BPF代码附加到Perf事件。Perf是内核中的一个内部探查器，它为硬件和软件发出性能数据事件。你可以用它来监视很多事情，从你的计算机的CPU到你系统上运行的任何软件。当您将BPF程序附加到Perf事件时，每次Perf生成数据供您分析时，您的代码都将被执行。

（1）用途

（2）SEC()、ebpf prog入口参数

SEC("perf_event")

**struct bpf_perf_event_data**

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

typora-notes/bpf_read/samples/sampleip.md

typora-notes/bpf_read/samples/trace_event.md

（5）实现分析





------

## 5. cgroups-related program types

### BPF_PROG_TYPE_CGROUP_SKB

（1）用途

这些类型的程序允许您将BPF逻辑附加到控制组（cgroups）。它们允许cgroup在它们包含的进程内控制网络流量。使用这些程序，您可以在将网络数据包传递到cgroup中的进程之前决定如何处理它。内核试图传递给同一cgroup中的任何进程的任何数据包都将通过这些过滤器之一。同时，您可以决定当cgroup中的进程通过该接口发送网络数据包时要做什么。

用于在  network cgroup 中运行的  BPF 代码。*功能上和  Socket_Filter 近似*。具体用法可以参考范例 linux-5.14.14\samples\bpf\test_cgrp2_attach.c

这些类型的程序允许您在运行时修改套接字连接选项，而数据包在内核的网络堆栈中经过几个阶段。

（2）SEC()、ebpf prog入口参数



**struct __sk_buff**

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

 linux-5.14.14\samples\bpf\test_cgrp2_attach.c

（5）实现分析

------

### BPF_PROG_TYPE_CGROUP_SOCK

（1）用途

这些类型的程序允许您在cgroup中的任何进程打开网络套接字时执行代码。这种行为类似于连接到cgroup套接字缓冲区的程序，但是它们不允许您在数据包通过网络时访问它们，而是允许您控制进程打开新套接字时发生的事情。它们是用BPF_PROG_TYPE_CGROUP_SOCK类型定义的。这有助于对可以打开套接字的程序组提供安全性和访问控制，而不必单独限制每个进程的功能。

另一个用于在  network cgroup  中运行的 BPF  代码，范例 linux-5.14.14\samples\bpf\test_cgrp2_sock2.c  中就展示了一个利用 BPF  来控制 host  和 netns  间通信的例子。

（2）SEC()、ebpf prog入口参数



**struct bpf_sock**

（3）是否可以修改数据包

（4）在哪儿有例子（项目、bcc、libbpf、samples）

linux-5.14.14\samples\bpf\test_cgrp2_sock2.c

（5）实现分析





------



------

### BPF_PROG_TYPE_CGROUP_DEVICE

这种类型的程序允许您决定是否可以对给定设备执行cgroup中的操作。

------



------



------

BPF_PROG_TYPE_RAW_TRACEPOINT、BPF_PROG_TYPE_CGROUP_SOCK_ADDR、BPF_PROG_TYPE_SK_REUSEPORT、BPF_PROG_TYPE_FLOW_DISSECTOR、BPF_PROG_TYPE_SCHED_CLS and BPF_PROG_TYPE_SCHED_ACT、BPF_PROG_TYPE_LWT_IN, BPF_PROG_TYPE_LWT_OUT, BPF_PROG_TYPE_LWT_XMIT and BPF_PROG_TYPE_LWT_SEG6LOCAL 、BPF_PROG_TYPE_LIRC_MODE2：略

------

## 6. Lightweight tunnel program types