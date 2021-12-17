# ELF

[eBPF 程序构成与通信原理解读](http://tinylab.org/ebpf-part1/)

[libelf 开源库用法详解](http://tinylab.org/libelf/)

llvm

llvm-objdump



readelf -h tracex4_kern.o
readelf -S tracex4_kern.o
llvm-objdump -h tracex4_kern.o
llvm-objdump -d -r -print-imm-hex tracex4_kern.o
llvm-objdump --section=maps  -s tracex4_kern.o
sudo strace -v -f -s 128 -o tracex4.txt ./tracex4
