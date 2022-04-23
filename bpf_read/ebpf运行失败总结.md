# ebpf运行失败总结

## csum相关

当前有：

csum_diff：28

l3_csum_replace：10

l4_csum_replace：11



### 这几个helper函数不是所有的prog都可以使用

https://stackoverflow.com/questions/62171477/ebpf-program-loading-error-unknown-func-bpf-l4-csum-replace11

bpf_l4_csum_replace：tc可以；socket_filter、XDP不能使用，出错信息如下：

```shell
221: (85) call bpf_l4_csum_replace#11
unknown func bpf_l4_csum_replace#11  
```

出错信息显示了数字11，就是这个helper对应的枚举值，说明内核可以识别这个函数。

*解释如下：*

A similar message could mean that your kernel does not know the BPF helper you are trying to use, because e.g. **your kernel is too old** or **the helper has been compiled out** based on the kernel configuration options. But in those cases, you would not see the name of the function in the verifier logs.

What is probably happening here is that your kernel does support the BPF helper, but **the type of the BPF program** you are trying to load **is not compatible with that helper**. For example, if your program is of type `socket_filter`, you cannot use this helper (see function [`sk_filter_func_proto()`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/net/core/filter.c?h=v5.7#n6127) used for [the check](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/kernel/bpf/verifier.c?h=v5.7#n4464)). If your program was a TC classifier instead, you would be able to use it.



### socket filter programs **are not allowed to do direct packet access**

https://stackoverflow.com/questions/62159622/what-is-wrong-with-the-bpf-csum-diff-function-call

You are loading your program as a `socket_filter`, which does not have *direct packet access*. Please refer to [this question](https://stackoverflow.com/questions/61702223/bpf-verifier-rejects-code-invalid-bpf-context-access) and its answer.           

I'm not sure I understood  your question. Why not attach your program as a TC filter instead of a  socket filter? It would probably be more flexible and possibly better  suited to what you are trying to achieve?

https://stackoverflow.com/questions/61702223/bpf-verifier-rejects-code-invalid-bpf-context-access

```c
#include <linux/bpf.h>
#include <linux/if_ether.h>

#define SEC(NAME) __attribute__((section(NAME), used))

SEC("socket_filter")
int myprog(struct __sk_buff *skb) {
        void *data = (void *)(long)skb->data;
        void *data_end = (void *)(long)skb->data_end;
        struct ethhdr *eth = data;

        if ((void*)eth + sizeof(*eth) > data_end)
                return 0;
        return 1;
}
```

```shell
clang -I./ -I/usr/include/x86_64-linux-gnu/asm \
        -I/usr/include/x86_64-linux-gnu/ -O2 -target bpf -c test.c  -o test.elf
        
invalid bpf_context access off=80 size=4
0000000000000000 packet_counter:
   0:       61 12 50 00 00 00 00 00 r2 = *(u32 *)(r1 + 80)
   1:       61 11 4c 00 00 00 00 00 r1 = *(u32 *)(r1 + 76)
   2:       07 01 00 00 0e 00 00 00 r1 += 14
   3:       b7 00 00 00 01 00 00 00 r0 = 1
   4:       3d 12 01 00 00 00 00 00 if r2 >= r1 goto +1 <LBB0_2>
   5:       b7 00 00 00 00 00 00 00 r0 = 0
```

*解释如下：*

This is because your BPF program is a **“socket filter”**, and that such programs **are not allowed to do direct packet access** (see [`sk_filter_is_valid_access()`](https://elixir.bootlin.com/linux/v5.6/source/net/core/filter.c#L6451), where we return `false` on trying to read `skb->data` or `skb->data_end` for example). I do not know the specific reason why it is not  available, although I suspect this would be a security precaution as  socket filter programs may be available to unprivileged users.

Your program loads just fine as a TC classifier, for example (`bpftool prog load foo.o /sys/fs/bpf/foo type classifier` -- By the way thanks for the standalone working reproducer, much appreciated!).

If you want to access data for a socket filter, you can still use the [`bpf_skb_load_bytes()`](https://elixir.bootlin.com/linux/v5.6/source/include/uapi/linux/bpf.h#L1126) (or `bpf_skb_store_bytes()`) helper, which automatically does the check on length. Something like this:

```c
#include <linux/bpf.h>

#define SEC(NAME) __attribute__((section(NAME), used))

static void *(*bpf_skb_load_bytes)(const struct __sk_buff *, __u32,
                                   void *, __u32) =
        (void *) BPF_FUNC_skb_load_bytes;

SEC("socket_filter")
int myprog(struct __sk_buff *skb)
{
        __u32 foo;

        if (bpf_skb_load_bytes(skb, 0, &foo, sizeof(foo)))
                return 0;
        if (foo == 3)
                return 0;
        return 1;
}
```

Regarding your last comment:

> However it only happens if I don't try to check the bounds later.

I suspect clang compiles out the assignments for `data` and `data_end` if you do not use them in your code, so they are no longer present and no longer a problem for the verifier.



### 这三个函数功能

参考typora-notes/net/协议/checksum.md

------



## [eBPF 指令集](https://houmin.cc/posts/5150fab3/)文章最后列举了一些错误



------

## 读取未初始化的变量或者结构体

### 1.初始化结构体部分字段

https://stackoverflow.com/questions/62441361/bpf-how-to-inspect-syscall-arguments

```c
// linux-5.14.14\samples\bpf\trace_output_kern.c
SEC("kprobe/" SYSCALL(sys_write))
int bpf_prog1(struct pt_regs *ctx)
{
	struct S {
		u64 pid;
		u64 cookie;
	} data;

	data.pid = bpf_get_current_pid_tgid();// 这两行初始化data的字段，缺一不可
	data.cookie = 0x12345678;

	bpf_perf_event_output(ctx, &my_map, 0, &data, sizeof(data));// 不进行上面对data的初始化，这里会报错

	return 0;
}
```

假如：

```c
    struct S {
            u64 pid;
            u64 cookie;
            char bleh[128]; //<-- added this 
    } data;

	data.pid = bpf_get_current_pid_tgid();// 并没有具体初始化 bleh
	data.cookie = 0x12345678;

	bpf_perf_event_output(ctx, &my_map, 0, &data, sizeof(data));// 所以会报错


/usr/src/linux-5.4/samples/bpf# ./trace_output
bpf_load_program() err=13
0: (bf) r6 = r1
1: (85) call bpf_get_current_pid_tgid#14
2: (b7) r1 = 305419896
3: (7b) *(u64 *)(r10 -136) = r1
4: (7b) *(u64 *)(r10 -144) = r0
5: (bf) r4 = r10
6: (07) r4 += -144
7: (bf) r1 = r6
8: (18) r2 = 0xffff8975bd44aa00
10: (b7) r3 = 0
11: (b7) r5 = 144
12: (85) call bpf_perf_event_output#25
invalid indirect read from stack off -144+16 size 144
processed 12 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
0: (bf) r6 = r1
1: (85) call bpf_get_current_pid_tgid#14
2: (b7) r1 = 305419896
3: (7b) *(u64 *)(r10 -136) = r1
4: (7b) *(u64 *)(r10 -144) = r0
5: (bf) r4 = r10
6: (07) r4 += -144
7: (bf) r1 = r6
8: (18) r2 = 0xffff8975bd44aa00
10: (b7) r3 = 0
11: (b7) r5 = 144
12: (85) call bpf_perf_event_output#25       // 这里
invalid indirect read from stack off -144+16 size 144
processed 12 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0

 // 修改
    struct S {
        u64 pid;
        u64 cookie;
        char bleh[128];
    } data = {0}; // 修改，初始化
```





------

## libbpf代码中混用bcc方式

### 1. 模仿bcc，将跟踪函数的入参，写入ebpf prog的上下文

https://stackoverflow.com/questions/62441361/bpf-how-to-inspect-syscall-arguments

```c
//试图跟踪exec，获取其入参filename的值

/* Signature of sys_execve: 
asmlinkage long sys_execve(const char __user *filename,
                			const char __user *const __user *argv,
                			const char __user *const __user *envp);
*/

SEC("kprobe/__x64_sys_execve")
int bpf_prog1(struct pt_regs *ctx, const char *filename)
{
        struct S {
                u64 pid;
                u64 cookie;
                char bleh[128];
        } data;

        data.pid = bpf_get_current_pid_tgid();
        data.cookie = 0x12345678;
        //bpf_get_current_comm(&data.bleh, 128);
        bpf_probe_read(&data.bleh, 128, (void *)filename);

        bpf_perf_event_output(ctx, &my_map, 0, &data, sizeof(data));

        return 0;
}


/usr/src/linux-5.4/samples/bpf# ./borky
bpf_load_program() err=13
0: (bf) r6 = r2
R2 !read_ok
processed 1 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
0: (bf) r6 = r2
R2 !read_ok
processed 1 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
```

You cannot pass the syscall arguments to your function `bpf_prog1()` as you do, it only takes the `struct pt_regs *ctx`. 

You likely saw this syntax in bcc, but [bcc rewrites it under the hood](https://github.com/iovisor/bcc/blob/v0.14.0/src/cc/frontends/clang/b_frontend_action.cc#L692). Prefer the `PT_REGS_PARM*(ctx)` macros to access them ([tracex1_kern.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/samples/bpf/tracex1_kern.c?h=v5.7#n33), [definition](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/lib/bpf/bpf_tracing.h?h=v5.7#n50)).

```c
//试图跟踪exec，获取其入参filename的值

/* Signature of sys_execve: 
asmlinkage long sys_execve(const char __user *filename,
                			const char __user *const __user *argv,
                			const char __user *const __user *envp);
*/

SEC("kprobe/__x64_sys_execve")
int bpf_prog1(struct pt_regs *ctx)
{
        struct S {
                u64 pid;
                u64 cookie;
                char bleh[128];
        } data;

        data.pid = bpf_get_current_pid_tgid();
        data.cookie = 0x12345678;
        //bpf_get_current_comm(&data.bleh, 128);
        bpf_probe_read(&data.bleh, 128, (void *)PT_REGS_PARM1(ctx));// 使用这个宏，去获取第一个参数

        bpf_perf_event_output(ctx, &my_map, 0, &data, sizeof(data));

        return 0;
}
```

