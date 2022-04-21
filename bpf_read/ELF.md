# ELF

[eBPF 程序构成与通信原理解读](http://tinylab.org/ebpf-part1/)

[eBPF 程序装载、翻译与运行过程详解](https://tinylab.org/ebpf-part2/)

[libelf 开源库用法详解](http://tinylab.org/libelf/)

llvm

llvm-objdump



readelf -h tracex4_kern.o
readelf -S tracex4_kern.o
llvm-objdump -h tracex4_kern.o
llvm-objdump -d -r -print-imm-hex tracex4_kern.o
llvm-objdump --section=maps  -s tracex4_kern.o
sudo strace -v -f -s 128 -o tracex4.txt ./tracex4





[eBPF 内核实现](https://houmin.cc/posts/ca9e2050/)

[Linux bpf 1.1、BPF内核实现](https://blog.csdn.net/pwl999/article/details/82884882)

