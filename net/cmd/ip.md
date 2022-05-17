# ip

## netns

#### blogs

| links                                                        | comment                                         |
| ------------------------------------------------------------ | ----------------------------------------------- |
| [ip netns的使用及network namespace 简介](https://blog.csdn.net/hbuxiaofei/article/details/107116064) | netns之间通信；使用 bridge 连接不同的 namespace |
| [Linux Namespace : 简介](https://www.cnblogs.com/sparkdev/p/9365405.html) | clone,setns,unshare                             |
| [Linux Namespace : Network](https://www.cnblogs.com/sparkdev/p/9462762.html) | 例子                                            |
| [Linux ip netns 命令](https://www.cnblogs.com/sparkdev/p/9253409.html) | 命令；ip netns add 命令的本质                   |
|                                                              |                                                 |
|                                                              |                                                 |
|                                                              |                                                 |
|                                                              |                                                 |
|                                                              |                                                 |
|                                                              |                                                 |

在 Linux 中，网络名字空间可以被认为是隔离的拥有单独网络栈（网卡、路由转发表、iptables）的环境。网络名字空间经常用来隔离网络设备和服务，只有拥有同样网络名字空间的设备，才能看到彼此。

从==逻辑==上说，网络命名空间是网络栈的副本，有自己的网络设备、路由选择表、邻接表、Netfilter表、网络套接字、网络procfs条目、网络sysfs条目和其他网络资源。

从==系统的角度==来看，当通过clone()系统调用创建新进程时，传递标志CLONE_NEWNET将在新进程中创建一个全新的网络命名空间。

从==用户的角度==来看，我们只需使用工具`ip`（package is iproute2）来创建一个新的持久网络命名空间。
————————————————

------

#### 命令行

| 用法                              | 含义                                                         |                                                              |
| --------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| ip netns list                     | 列出网络命名空间。此命令显示的是 “/var/run/netns” 中的所有网络命名空间。 |                                                              |
| ip netns add NAME                 | 添加网络命名空间                                             |                                                              |
| ip [-all] netns delete [NAME]     | 删除网络命名空间                                             | -all删除所有的ns                                             |
| ip [-all] netns exec [NAME] cmd … | 在指定的网络命名空间中执行命令，进入网络命名空间可以用 exec 执行 bash 即可：ip netns exec test1 bash | -all 选项可以在所有网络命名空间中执行命令：ip -all netns exec ip a |
| ip netns set NAME NETNSID         | 给网络命名空间分配id                                         |                                                              |
| ip netns identify [PID]           | 查看进程的网络命名空间                                       | root@cp:~# ip netns identify 11421<br/> test                 |
| ip netns pids NAME                | 查找使用此网络命名空间并将其作为主要网络命名空间的进程。此命令会从 /proc 目录中遍历。 | root@cp:~# ip netns pids test<br/>11421                      |
|                                   |                                                              |                                                              |
| ip netns monitor                  | 监控网络命名空间的添加和删除事件，只能监控添加和删除         | root@cp:~# ip netns del cp<br/>root@cp:~# ip netns add cp<br/><br/>另一个终端同步反馈的<br/>root@cp:~# ip netns monitor<br/>delete cp<br/>add cp |

------

#### veth-pair

创建一对 veth pair 虚拟网络设备接口，然后将其分别分配给两个网络命名空间，去连接两个网络命名空间。
  注：veth-pair 是一对的虚拟网络设备接口，它都是成对出现的，所以它常常充当着一个桥梁，我们可以用它实现 “网络命名空间之间的连接”、“Docker 容器之间的连接” 、“Docker 容器和网桥间的连接” 等等。

#### docker网络命名空间的问题

  当 docker 容器被创建出来后，你会发现使用 ip netns 命令无法看到容器对应的网络命名空间。这是因为 ip netns 命令是从 /var/run/netns 文件夹中读取内容的，而 docker 容器的网络命名空间不是在 /var/run/netns 下，而是位于 /proc/[pid]/ns/net。想要使用 ip netns 命令去管理 docker 容器的网络命名空间，就需要将它的网络命名空间显示在 /var/run/netns 目录下，那就要先找到容器的网络命名空间在哪里，然后做一个==软链接==即可。

首先查询容器的PID。

```shell
root@cp:~# docker inspect --format '{{.State.Pid}}' web
4775
```

然后创建软链接，建议指定在 /var/run/netns/ 中的名字，因为每个容器都是net。

```shell
root@cp:~# ln -s /proc/4775/ns/net /var/run/netns/web
root@cp:~# ip netns ls
web (id: 0)
test2 (id: 2)
test1 (id: 1)
```

此时就可以用 ip netns 命令去管理 docker 容器的网络命名空间了。


## addr

2、为网络接口分配IPv4地址
asn@dell60:~$ sudo ip addr add 192.168.15.60/24 dev enp65s0f1
asn@dell60:~$ sudo ip addr add 192.168.16.60/24 dev enp65s0f0


sudo ip addr del 192.168.15.60/24 dev enp65s0f1
ip link set enp0s3 down/up

asn@dell61:~$ sudo ip addr add 192.168.15.61/24 dev enp65s0f1
asn@dell61:~$ sudo ip addr add 192.168.16.61/24 dev enp65s0f0



## link





查看全部网卡

目录 /sys/class/net/

    [root@localhost ~]# ls /sys/class/net/
    br-df65b94a220f  docker0  enp0s31f6  lo  veth1706661  veth2566f96  veth7c083c7  vethd4a4beb  vethfa8ecf9  vethfd44a20  wlp1s0

查看虚拟网卡

目录 /sys/devices/virtual/net/

    [root@localhost ~]# ls /sys/devices/virtual/net/
    br-df65b94a220f  docker0  lo  veth1706661  veth2566f96  veth7c083c7  vethd4a4beb  vethfa8ecf9  vethfd44a20

查看物理网卡

    [root@localhost ~]# ls /sys/class/net/ | grep -v "`ls /sys/devices/virtual/net/`"
    enp0s31f6
    wlp1s0




