[eBPF: 从 BPF to BPF Calls 到 Tail Calls](https://lizhaolong.blog.csdn.net/article/details/123474244)

以及附注的参考资料

https://forsworns.github.io/zh/blogs/20210329/#tail-calls

https://forsworns.github.io/zh/blogs/20210329/#bpf-to-bpf-calls

```c
int bpf_tail_call(void *ctx, struct bpf_map *prog_array_map, u32 index)
```

这是一个特殊的助手函数，用于触发尾调用 —— 跳转到另外一个eBPF程序。新程序将使用一样的栈帧，但是被调用者不能访问调用者在栈上存储的值，以及寄存器。

使用场景包括：

1. 突破eBPF程序长度限制
2. 在不同条件下进行跳转（到子程序）

出于安全原因，可以连续执行的尾调用次数是受限制的。限制定义在内核宏MAX_TAIL_CALL_CNT中，默认32，无法被用户空间访问

当调用发生后，程序尝试跳转到prog_array_map（BPF_MAP_TYPE_PROG_ARRAY类型的Map）的index索引处的eBPF程序，并且将当前ctx传递给它。

如果调用成功，则当前程序被替换掉，不存在函数调用返回。如果调用失败，则不产生任何作用，当前程序继续运行后续指令。失败的原因包括：

1. 指定的index不存在eBPF程序
2. 当前尾调用链的长度超过限制