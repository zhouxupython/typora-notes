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



### 1

https://stackoverflow.com/questions/62159622/what-is-wrong-with-the-bpf-csum-diff-function-call

​            

You are loading your program as a `socket_filter`, which does not have *direct packet access*. Please refer to [this question](https://stackoverflow.com/questions/61702223/bpf-verifier-rejects-code-invalid-bpf-context-access) and its answer.           

I'm not sure I understood  your question. Why not attach your program as a TC filter instead of a  socket filter? It would probably be more flexible and possibly better  suited to what you are trying to achieve?





https://stackoverflow.com/questions/61702223/bpf-verifier-rejects-code-invalid-bpf-context-access



### 这三个函数功能
