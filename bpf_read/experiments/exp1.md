

# experiment-1

#### [你的第一个XDP BPF 程序](https://davidlovezoe.club/wordpress/archives/937)

##### 主机和虚拟机

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
    sudo ln -s ./x86_64-linux-gnu/asm asm
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



##### 虚拟机和其上的docker

```shell
zx@zx:/etc/docker$ sudo docker run -d -p 80:80 --name=nginx-xdp nginx:alpine

zx@zx:/etc/docker$ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 08:00:27:b6:50:91 brd ff:ff:ff:ff:ff:ff
    inet 10.0.2.15/24 brd 10.0.2.255 scope global dynamic noprefixroute enp0s3
       valid_lft 65787sec preferred_lft 65787sec
    inet6 fe80::c7cf:3b07:7f39:68c8/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
3: docker0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:cc:40:33:a0 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:ccff:fe40:33a0/64 scope link 
       valid_lft forever preferred_lft forever
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever
       
#一个窗口开启抓包       
zx@zx:~$ sudo tcpdump -i veth0afb74f -vv -nn tcp        

#另一个curl
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ curl localhost       
```



###### 没有启用xdp

没有启用xdp程序时，可以在docker外部通过curl命令获取docker-nginx的服务

![image-20211123094650419](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123094650419.png)

###### 启动xdp

```shell
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ clang -I ./headers/ -O2 -target bpf -c tc-xdp-drop-tcp.c -o tc-xdp-drop-tcp.o

zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ llvm-objdump-13 -S tc-xdp-drop-tcp.o

tc-xdp-drop-tcp.o:	file format elf64-bpf

Disassembly of section xdp:

0000000000000000 <xdp_drop_tcp>:
       0:	61 12 04 00 00 00 00 00	r2 = *(u32 *)(r1 + 4)
       1:	61 11 00 00 00 00 00 00	r1 = *(u32 *)(r1 + 0)
       2:	bf 13 00 00 00 00 00 00	r3 = r1
       3:	07 03 00 00 0e 00 00 00	r3 += 14
       4:	2d 23 0b 00 00 00 00 00	if r3 > r2 goto +11 <LBB0_4>
       5:	71 13 0d 00 00 00 00 00	r3 = *(u8 *)(r1 + 13)
       6:	67 03 00 00 08 00 00 00	r3 <<= 8
       7:	71 14 0c 00 00 00 00 00	r4 = *(u8 *)(r1 + 12)
       8:	4f 43 00 00 00 00 00 00	r3 |= r4
       9:	55 03 06 00 08 00 00 00	if r3 != 8 goto +6 <LBB0_4>
      10:	bf 13 00 00 00 00 00 00	r3 = r1
      11:	07 03 00 00 22 00 00 00	r3 += 34
      12:	2d 23 03 00 00 00 00 00	if r3 > r2 goto +3 <LBB0_4>
      13:	b7 00 00 00 01 00 00 00	r0 = 1
      14:	71 11 17 00 00 00 00 00	r1 = *(u8 *)(r1 + 23)
      15:	15 01 01 00 06 00 00 00	if r1 == 6 goto +1 <LBB0_5>

0000000000000080 <LBB0_4>:
      16:	b7 00 00 00 02 00 00 00	r0 = 2

0000000000000088 <LBB0_5>:
      17:	95 00 00 00 00 00 00 00	exit

Disassembly of section tc:

0000000000000000 <tc_drop_tcp>:
       0:	61 12 50 00 00 00 00 00	r2 = *(u32 *)(r1 + 80)
       1:	61 11 4c 00 00 00 00 00	r1 = *(u32 *)(r1 + 76)
       2:	bf 13 00 00 00 00 00 00	r3 = r1
       3:	07 03 00 00 0e 00 00 00	r3 += 14
       4:	2d 23 0b 00 00 00 00 00	if r3 > r2 goto +11 <LBB1_4>
       5:	71 13 0d 00 00 00 00 00	r3 = *(u8 *)(r1 + 13)
       6:	67 03 00 00 08 00 00 00	r3 <<= 8
       7:	71 14 0c 00 00 00 00 00	r4 = *(u8 *)(r1 + 12)
       8:	4f 43 00 00 00 00 00 00	r3 |= r4
       9:	55 03 06 00 08 00 00 00	if r3 != 8 goto +6 <LBB1_4>
      10:	bf 13 00 00 00 00 00 00	r3 = r1
      11:	07 03 00 00 22 00 00 00	r3 += 34
      12:	2d 23 03 00 00 00 00 00	if r3 > r2 goto +3 <LBB1_4>
      13:	b7 00 00 00 02 00 00 00	r0 = 2
      14:	71 11 17 00 00 00 00 00	r1 = *(u8 *)(r1 + 23)
      15:	15 01 01 00 06 00 00 00	if r1 == 6 goto +1 <LBB1_5>

0000000000000080 <LBB1_4>:
      16:	b7 00 00 00 00 00 00 00	r0 = 0

0000000000000088 <LBB1_5>:
      17:	95 00 00 00 00 00 00 00	exit


#一个窗口开启抓包
zx@zx:~$ sudo tcpdump -i veth0afb74f -vv -nn tcp 

#另一个开启xdp后，curl
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp object tc-xdp-drop-tcp.o section xdp verbose
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ curl localhost




```



![image-20211123103837042](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123103837042.png)

###### 关闭xdp

```
#关闭xdp
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp off
```

![image-20211123104454150](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123104454150.png)

xdp开启前后，接口的变化

```shell
x@zx:~$ ip a
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever
zx@zx:~$ ip a
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp/id:107 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever


【Q】xdp/id:107  含义是什么
```







#### [你的第一个TC BPF 程序](https://davidlovezoe.club/wordpress/archives/952)

##### 准备工作

```shell
x@zx:~$ sudo docker inspect nginx-xdp -f "{{.NetworkSettings.SandboxKey}}"
/var/run/docker/netns/1ce50d102049
zx@zx:~$ 
zx@zx:~$ sudo ls -l /var/run/docker/netns/1ce50d102049
-r--r--r-- 1 root root 0 11月 23 08:56 /var/run/docker/netns/1ce50d102049
zx@zx:~$ sudo mkdir -p /var/run/netns
zx@zx:~$ sudo ln -s /var/run/docker/netns/1ce50d102049 /var/run/netns/httpserver
zx@zx:~$ ll /var/run/netns/httpserver
lrwxrwxrwx 1 root root 34 11月 23 11:08 /var/run/netns/httpserver -> /var/run/docker/netns/1ce50d102049

zx@zx:~$ sudo ip netns exec httpserver ip a   #此时相当于进入了这个docker容器的内部执行ip a 命令
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
#这个if是该docker对外的接口，有ip地址
4: eth0@if5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.2/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever

#docker的主机执行
zx@zx:~$ ip addr 
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 08:00:27:b6:50:91 brd ff:ff:ff:ff:ff:ff
    inet 10.0.2.15/24 brd 10.0.2.255 scope global dynamic noprefixroute enp0s3
       valid_lft 59330sec preferred_lft 59330sec
    inet6 fe80::c7cf:3b07:7f39:68c8/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
3: docker0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:cc:40:33:a0 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:ccff:fe40:33a0/64 scope link 
       valid_lft forever preferred_lft forever
#这个docker主机的if是和docker的eth0@if5 成对的，通过网桥docker0和eth0@if5连接
5: veth0afb74f@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default 
    link/ether e2:2d:5e:ee:af:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::e02d:5eff:feee:af97/64 scope link 
       valid_lft forever preferred_lft forever
zx@zx:~$ 
zx@zx:~$ 

```



docker curl 使用[==还是别用curl了==，目的是为了验证tcp协议，用==nping==]

```shell
x@zx:~$ sudo ip netns exec httpserver curl --dns-servers 8.8.8.8 www.baidu.com
curl: (6) Could not resolve host: www.baidu.com
zx@zx:~$ sudo ip netns exec httpserver curl  www.baidu.com
curl: (6) Could not resolve host: www.baidu.com
zx@zx:~$ #说明无法解析域名
zx@zx:~$ sudo ip netns exec httpserver ping  www.baidu.com
ping: www.baidu.com: Temporary failure in name resolution

zx@zx:~$ sudo ip netns exec httpserver cat /etc/resolv.conf
nameserver 127.0.0.53
options edns0 trust-ad

#添加一个，如果还是不行，那么就给docker的主机也添加这一条（docker主机可能是虚拟机，也没有这一条）
zx@zx:~$ sudo ip netns exec httpserver vi /etc/resolv.conf
x@zx:~$ sudo ip netns exec httpserver cat /etc/resolv.conf
nameserver 8.8.8.8
#nameserver 127.0.0.53
options edns0 trust-ad

x@zx:~$ sudo ip netns exec httpserver ping  www.baidu.com
PING www.wshifen.com (104.193.88.77) 56(84) bytes of data.
64 bytes from 104.193.88.77 (104.193.88.77): icmp_seq=1 ttl=61 time=205 ms
64 bytes from 104.193.88.77 (104.193.88.77): icmp_seq=2 ttl=61 time=204 ms
^C
--- www.wshifen.com ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1001ms
rtt min/avg/max/mdev = 203.984/204.691/205.398/0.707 ms
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec httpserver curl  www.baidu.com

```



##### 反复开启关闭xdp访问外网

这个和 ==https://davidlovezoe.club/wordpress/archives/952 2. 在Nginx容器内部curl外部网站==结果不一样

```shell
#第一个终端上编译xdp，反复加载和卸载xdp程序
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ clang -I ./headers/ -O2 -target bpf -c tc-xdp-drop-tcp.c -o tc-xdp-drop-tcp.o

zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp object tc-xdp-drop-tcp.o section xdp verbose

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 18 (0 over limit)
 - License:      GPL

Verifier analysis:

processed 25 insns (limit 1000000) max_states_per_insn 0 total_states 2 peak_states 2 mark_read 1

zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ 
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp off
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ 
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp object tc-xdp-drop-tcp.o section xdp verbose

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 18 (0 over limit)
 - License:      GPL

Verifier analysis:

processed 25 insns (limit 1000000) max_states_per_insn 0 total_states 2 peak_states 2 mark_read 1

zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp off
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ 
zx@zx:~/works/ebpf/src_from_github/linux-bpf-learning/tc$ sudo ip link set dev veth0afb74f xdp object tc-xdp-drop-tcp.o section xdp verbose

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 18 (0 over limit)
 - License:      GPL

Verifier analysis:

processed 25 insns (limit 1000000) max_states_per_insn 0 total_states 2 peak_states 2 mark_read 1


#第二个终端上从docker内部使用nping+tcp 去ping外网
#一对SEND-RECV 表示ping通了，只有SEND表示没有ping通
x@zx:~$ sudo ip netns exec httpserver sudo nping -c 200 --tcp www.baidu.com

Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2021-11-23 14:06 CST
SENT (0.5159s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (1.5161s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (2.5172s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (3.5223s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (4.5225s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (5.5237s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (6.5248s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (6.7772s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6536 iplen=44  seq=3543249409 win=65535 <mss 1460>
SENT (7.5261s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (7.8120s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6537 iplen=44  seq=3543441409 win=65535 <mss 1460>
SENT (8.5279s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (8.8222s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6538 iplen=44  seq=3543633409 win=65535 <mss 1460>
SENT (9.5290s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (9.8139s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6539 iplen=44  seq=3543761409 win=65535 <mss 1460>
SENT (10.5305s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (10.8148s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6540 iplen=44  seq=3543953409 win=65535 <mss 1460>
SENT (11.5311s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (11.8254s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6541 iplen=44  seq=3544145409 win=65535 <mss 1460>
SENT (12.5323s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (12.8059s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6542 iplen=44  seq=3544273409 win=65535 <mss 1460>
SENT (13.5338s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (14.5345s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (15.5357s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (16.5366s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (17.5378s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (18.5385s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (19.5397s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (20.5409s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (20.7844s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6543 iplen=44  seq=3545297409 win=65535 <mss 1460>
SENT (21.5423s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (21.8139s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6544 iplen=44  seq=3545489409 win=65535 <mss 1460>
SENT (22.5456s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (22.7993s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6545 iplen=44  seq=3545617409 win=65535 <mss 1460>
SENT (23.5465s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (23.7998s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6546 iplen=44  seq=3545809409 win=65535 <mss 1460>
SENT (24.5477s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (24.7993s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6547 iplen=44  seq=3546001409 win=65535 <mss 1460>
SENT (25.5529s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (25.7986s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6548 iplen=44  seq=3546129409 win=65535 <mss 1460>
SENT (26.5545s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
RCVD (26.7965s) TCP 104.193.88.123:80 > 172.17.0.2:61797 SA ttl=63 id=6549 iplen=44  seq=3546257409 win=65535 <mss 1460>
SENT (27.5563s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (28.5575s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (29.5585s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (30.5597s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (31.5607s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (32.5620s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
SENT (33.5625s) TCP 172.17.0.2:61797 > 104.193.88.123:80 S ttl=64 id=11469 iplen=40  seq=1502809973 win=1480 
^C 
Max rtt: 294.219ms | Min rtt: 241.908ms | Avg rtt: 266.434ms
Raw packets sent: 34 (1.360KB) | Rcvd: 14 (616B) | Lost: 20 (58.82%)
Nping done: 1 IP address pinged in 33.68 seconds

```

![image-20211123141133282](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123141133282.png)

开启xdp后，docker容器去ping外网地址，需要经过主机上的docker0，也就是此时的veth6c30d72@if4接口，该接口上即开启了xdp程序，对于所有的ingress的tcp流量，是drop的，所以docker容器经过这个接口去外网的流量是出不去的，直接被veth6c30d72@if4接口丢弃，`由于xdp比抓包工具更早的接触到流量并抛弃`，因此tcpdump抓不到包的。

如图，容器通过docker0访问外网，配对的veth上开启了xdp，丢弃tcp数据，那么此时容器发出的nping-tcp就被丢弃了。

![image-20211123153200021](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123153200021.png)

![image-20211123151820047](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123151820047.png)



##### 反复开启关闭xdp访问配对的veth

![image-20211123154242780](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123154242780.png)



##### 同时使用XDP和TC

验证同时使用XDP和TC，控制RX和TX的TCP流量

###### 准备工作

```shell
x@zx:~$ sudo ip link set dev veth6c30d72 xdp object ~/works/ebpf/src_from_github/linux-bpf-learning/tc/tc-xdp-drop-tcp.o section xdp verbose
[sudo] password for zx: 

Prog section 'xdp' loaded (5)!
 - Type:         6
 - Instructions: 18 (0 over limit)
 - License:      GPL

Verifier analysis:

processed 25 insns (limit 1000000) max_states_per_insn 0 total_states 2 peak_states 2 mark_read 1


#tc
zx@zx:~$ tc qdisc show dev veth6c30d72
qdisc noqueue 0: root refcnt 2 
zx@zx:~$ sudo tc qdisc add dev veth6c30d72 clsact
zx@zx:~$ sudo tc qdisc show dev veth6c30d72
qdisc noqueue 0: root refcnt 2 
qdisc clsact ffff: parent ffff:fff1 

zx@zx:~$ sudo tc filter add dev veth6c30d72 egress bpf da obj ~/works/ebpf/src_from_github/linux-bpf-learning/tc/tc-xdp-drop-tcp.o sec tc

zx@zx:~$ sudo tc qdisc show dev veth6c30d72
qdisc noqueue 0: root refcnt 2 
qdisc clsact ffff: parent ffff:fff1 

zx@zx:~$ tc filter show dev veth6c30d72 egress
filter protocol all pref 49152 bpf chain 0 
filter protocol all pref 49152 bpf chain 0 handle 0x1 tc-xdp-drop-tcp.o:[tc] direct-action not_in_hw id 37 tag 2606b497477d080a 
zx@zx:~$ 


```



###### 开启xdp和tc程序

- docker容器对外网地址和docker0进行nping-tcp，均失败

因为主机上的veth口开启了xdp，所以docker容器发到该接口或者由该接口转发的数据，均被丢弃。

==而且tcpdump不能抓到数据。==

![image-20211123170040745](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123170040745.png)

- 主机访问docker容器的nginx服务，因为主机的egress是drop tcp的，所以失败

    ```shell
    curl -m 5 -vvv   localhost
    ```

    

    

![image-20211123170930224](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123170930224.png)

==那如何解释此处抓包也抓不到呢？==

![image-20211123171948700](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123171948700.png)

这样看，应该是tcpdump的钩子点比TC更晚，导致流量被TC丢弃了，类似于上面的xdp丢弃流量

【确认这一点？】

- 主机nping-tcp docker容器的eth0，因为主机的egress是drop tcp的，所以失败，==同样抓不到包==

```shell
zx@zx:~$ sudo nping -c 100 --tcp 172.17.0.2
[sudo] password for zx: 

Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2021-11-23 17:16 CST
SENT (0.0301s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (1.0303s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (2.0315s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (3.0326s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (4.0338s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (5.0350s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (6.0362s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
SENT (7.0367s) TCP 172.17.0.1:61418 > 172.17.0.2:80 S ttl=64 id=63792 iplen=40  seq=3749636460 win=1480 
^C 
Max rtt: N/A | Min rtt: N/A | Avg rtt: N/A
Raw packets sent: 8 (320B) | Rcvd: 0 (0B) | Lost: 8 (100.00%)
Nping done: 1 IP address pinged in 8.03 seconds

```



- 主机ping docker容器的eth0，因为ping是icmp协议，不是tcp协议，没有配置icmp协议的规则，所以通

    ```shell
    zx@zx:~$ sudo ping -c 5  172.17.0.2
    PING 172.17.0.2 (172.17.0.2) 56(84) bytes of data.
    64 bytes from 172.17.0.2: icmp_seq=1 ttl=64 time=0.038 ms
    64 bytes from 172.17.0.2: icmp_seq=2 ttl=64 time=0.045 ms
    64 bytes from 172.17.0.2: icmp_seq=3 ttl=64 time=0.044 ms
    64 bytes from 172.17.0.2: icmp_seq=4 ttl=64 time=0.043 ms
    64 bytes from 172.17.0.2: icmp_seq=5 ttl=64 time=0.041 ms
    
    --- 172.17.0.2 ping statistics ---
    5 packets transmitted, 5 received, 0% packet loss, time 4075ms
    rtt min/avg/max/mdev = 0.038/0.042/0.045/0.002 ms
    zx@zx:~$ 
    
    
    x@zx:~$ sudo nping -c 5 --icmp 172.17.0.2
    Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2021-11-23 17:31 CST
    SENT (0.0372s) ICMP [172.17.0.1 > 172.17.0.2 Echo request (type=8/code=0) id=54397 seq=1] IP [ttl=64 id=35334 iplen=28 ]
    RCVD (0.0374s) ICMP [172.17.0.2 > 172.17.0.1 Echo reply (type=0/code=0) id=54397 seq=1] IP [ttl=64 id=63620 iplen=28 ]
     
    Max rtt: 0.114ms | Min rtt: 0.025ms | Avg rtt: 0.055ms
    Raw packets sent: 5 (140B) | Rcvd: 5 (140B) | Lost: 0 (0.00%)
    Nping done: 1 IP address pinged in 4.07 seconds
    
    ```

    

- 同样道理,docker容器可以ping外部：

```shell
x@zx:~$ sudo ip netns exec httpserver sudo nping -c 5 --icmp 172.17.0.1

Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2021-11-23 17:28 CST
SENT (0.0340s) ICMP [172.17.0.2 > 172.17.0.1 Echo request (type=8/code=0) id=48168 seq=1] IP [ttl=64 id=31276 iplen=28 ]
RCVD (0.0341s) ICMP [172.17.0.1 > 172.17.0.2 Echo reply (type=0/code=0) id=48168 seq=1] IP [ttl=64 id=19702 iplen=28 ]

 
Max rtt: 0.034ms | Min rtt: 0.027ms | Avg rtt: 0.029ms
Raw packets sent: 5 (140B) | Rcvd: 5 (140B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 4.07 seconds
zx@zx:~$ 
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec httpserver sudo nping -c 5 --icmp www.baidu.com

Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2021-11-23 17:29 CST
SENT (0.5314s) ICMP [172.17.0.2 > 104.193.88.123 Echo request (type=8/code=0) id=53164 seq=1] IP [ttl=64 id=36694 iplen=28 ]
RCVD (0.7198s) ICMP [104.193.88.123 > 172.17.0.2 Echo reply (type=0/code=0) id=53164 seq=1] IP [ttl=61 id=48401 iplen=28 ]
SENT (1.5316s) ICMP [172.17.0.2 > 104.193.88.123 Echo request (type=8/code=0) id=53164 seq=3] IP [ttl=64 id=36694 iplen=28 ]

 
Max rtt: 188.727ms | Min rtt: 188.219ms | Avg rtt: 188.512ms
Raw packets sent: 5 (140B) | Rcvd: 5 (140B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 4.76 seconds

```





###### 先单独关闭xdp

- docker容器对外网地址和docker0进行nping-tcp，均失败

因为主机上的veth口关闭了xdp，所以可以接收docker容器发到该接口或者由该接口转发的数据，因此可以抓包。

（1）外网地址

需要继续对外转发，该接口开启了tc egress，会使 tcp 流量被丢弃，无法从该接口出去

（2）docker0

172.17.0.1，这里docker0会回复容器的nping，该接口开启了tc egress，会使 tcp 流量被丢弃，无法从该接口出去。

这里疑惑的是contrack类似的功能，==并不是，没有关系==，是docker0开启了tc egress tcp drop，和容器无关。

![image-20211123180313156](/home/zhouxu/works/notes/typora-notes/bpf_read/experiments/image-20211123180313156.png)



- 主机访问docker容器的nginx服务，因为主机的egress是drop tcp的，所以失败

```shell
curl -m 5 -vvv   localhost
```



- 主机nping-tcp docker容器的eth0，因为主机的egress是drop tcp的，所以失败，==同样抓不到包==



- 主机ping docker容器的eth0，因为ping是icmp协议，不是tcp协议，没有配置icmp协议的规则，所以通
- 同样道理,docker容器可以ping外部：



###### 再关闭tc

- docker容器对外网地址和docker0进行nping-tcp，均失败

因为主机上的veth口开启了xdp，所以docker容器发到该接口或者由该接口转发的数据，均被丢弃。

==而且tcpdump不能抓到数据。==





- 主机访问docker容器的nginx服务，因为主机的egress是drop tcp的，所以失败

```shell
curl -m 5 -vvv   localhost
```



- 主机nping-tcp docker容器的eth0，因为主机的egress是drop tcp的，所以失败，==同样抓不到包==



- 主机ping docker容器的eth0，因为ping是icmp协议，不是tcp协议，没有配置icmp协议的规则，所以通
- 同样道理,docker容器可以ping外部：

#### [调试你的BPF程序](https://davidlovezoe.club/wordpress/archives/963)







[编译运行LINUX内核源码中的BPF示例代码](https://davidlovezoe.club/wordpress/archives/988)

