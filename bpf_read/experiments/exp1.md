# experiment-1

#### [你的第一个XDP BPF 程序](https://davidlovezoe.club/wordpress/archives/937)

```shell
zx@u18-1:~$ clang-13 -O2 -target bpf -c xdp-drop-world.c -o xdp-drop-world.o
In file included from tc-xdp-drop-tcp.c:2:
In file included from /usr/include/linux/bpf.h:11:
/usr/include/linux/types.h:5:10: fatal error: 'asm/types.h' file not found
#include <asm/types.h>
        ^~~~~~~~~~~~~
1 error generated.

```

- 原因分析

    在源代码文件中引用了某些系统目录（一般为`/usr/include/`，是==绝对路径==）下的头文件，而这些头文件没有出现在目标路径下，导致编译失败。

    如上述问题中的asm相关文件，asm全称`Architecture Specific Macros`，直译过来“与机器架构相关的宏文件”，顾名思义它是跟机器架构密切相关的，不同的架构x86、x64、arm实现是不一样的，而操作系统并没有提供`/usr/include/asm/`这样通用的目录，只提供了具体架构相关的目录，如`/usr/include/x86_64-linux-gnu/asm/`，因此无法找到引用。

- 解决方案

    添加软链`/usr/include/asm/`，指向操作系统自带的asm目录：

    ```shell
    cd /usr/include
    ln -s ./x86_64-linux-gnu/asm asm
    ```



```shell
#win10， used to ping the virtual machine in it
C:\Users\zhouxu>ping -n 100 192.168.28.133

#virtual machine, 用于抓包
zx@u18-1:~$ ip addr
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UNKNOWN group default qlen 1000
    link/ether 00:0c:29:c4:d3:78 brd ff:ff:ff:ff:ff:ff
    inet 192.168.28.133/24 brd 192.168.28.255 scope global dynamic noprefixroute ens33
       valid_lft 1702sec preferred_lft 1702sec
    inet6 fe80::c6fd:b0ad:be8:94b4/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever

#ens33这个if是对外的，所以win10 ping的时候，会走这个if
zx@u18-1:~$ sudo tcpdump -i ens33

#virtual machine, 另一个终端，用于开启或者关闭xdp程序
zx@u18-1:~$ sudo ip link set dev ens33 xdp object /home/zx/works/ebpf/src_from_github/linux-bpf-learning/xdp/xdp-drop-world.o section xdp verbose

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 2 (0 over limit)
 - License:      GPL

Verifier analysis:

0: (b7) r0 = 1
1: (95) exit
processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0



```



未开启xdp程序，正常ping

![image-20211122231401912](C:\Users\zhouxu\AppData\Roaming\Typora\typora-user-images\image-20211122231401912.png)

开启xdp程序，开始丢包

![image-20211122231619375](C:\Users\zhouxu\AppData\Roaming\Typora\typora-user-images\image-20211122231619375.png)

关闭xdp程序后，又可以ping通

<img src="C:\Users\zhouxu\AppData\Roaming\Typora\typora-user-images\image-20211122231818401.png" alt="image-20211122231818401" style="zoom: 200%;" />





#### [你的第一个TC BPF 程序](https://davidlovezoe.club/wordpress/archives/952)







#### [调试你的BPF程序](https://davidlovezoe.club/wordpress/archives/963)







[编译运行LINUX内核源码中的BPF示例代码](https://davidlovezoe.club/wordpress/archives/988)

