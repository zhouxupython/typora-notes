# docker_cgroup

------



https://blog.csdn.net/flag2920/category_10852090.html	对docker的cpu mem进行限制



```shell
zx@zx-docker:~$ docker ps -a
CONTAINER ID   IMAGE        COMMAND       CREATED        STATUS                      PORTS     NAMES
`b518627b0da2`   ubuntu-net   "/bin/bash"   46 hours ago   Exited (0) 22 hours ago               c3
zx@zx-docker:~$ docker start b518627b0da2
b518627b0da2
zx@zx-docker:~$ docker inspect b518627b0da2 | grep -i pid
            "Pid": `2744`,
zx@zx-docker:~$ cd /sys/fs/cgroup/cpu/docker
zx@zx-docker:/sys/fs/cgroup/cpu/docker$ ls
`b518627b0da25ab...` 
......
zx@zx-docker:/sys/fs/cgroup/cpu/docker$ cd b518627b0da25ab.../
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$ ls
cgroup.clone_children  cpuacct.usage_all          cpuacct.usage_sys   cpu.shares      notify_on_release
cgroup.procs           cpuacct.usage_percpu       cpuacct.usage_user  cpu.stat        tasks
cpuacct.stat           cpuacct.usage_percpu_sys   cpu.cfs_period_us   cpu.uclamp.max
cpuacct.usage          cpuacct.usage_percpu_user  cpu.cfs_quota_us    cpu.uclamp.min
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$ cat cpu.cfs_period_us
100000
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$ cat cpu.cfs_quota_us
-1
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$ cat tasks
`2744`
zx@zx-docker:/sys/fs/cgroup/cpu/docker/b518627b0da25ab...$ cat `/proc/2744/cgroup`
12:pids:/docker/b518627b0da25ab...
11:freezer:/docker/b518627b0da25ab...
10:rdma:/
9:hugetlb:/docker/b518627b0da25ab...
8:memory:/docker/b518627b0da25ab...
7:devices:/docker/b518627b0da25ab...
6:cpuset:/docker/b518627b0da25ab...
5:perf_event:/docker/b518627b0da25ab...
4:cpu,cpuacct:/docker/b518627b0da25ab...
3:net_cls,net_prio:/docker/b518627b0da25ab...
2:blkio:/docker/b518627b0da25ab...
1:name=systemd:/docker/b518627b0da25ab...
0::/system.slice/containerd.service

#好像只有这个路径下执行才有效
zx@zx-docker:/sys/fs/cgroup/cpu$ `cgget -a docker/b518627b0da25ab...`
docker/b518627b0da25ab... :
blkio.throttle.read_iops_device:
......
net_cls.classid: 0
net_prio.prioidx: 3
net_prio.ifpriomap: lo 0
        enp0s3 0
        br-4a44e5fb02c2 0
        br-63ec0e397c57 0
        docker0 0
        veth4224a1d 0
cpu.cfs_period_us: 100000
cpu.stat: nr_periods 0
        nr_throttled 0
        throttled_time 0
cpu.shares: 1024
cpu.cfs_quota_us: -1
...
pids.current: 1
pids.events: max 0
pids.max: max

zx@zx-docker:/sys/fs/cgroup/cpu$ `cgget -r cpu.cfs_quota_us   docker/b518627b0da25ab...`
docker/b518627b0da25ab... :
cpu.cfs_quota_us: -1
```



![image-20211230112400522](image-20211230112400522.png)

容器在宿主机的进程号，会写入该docker的cgroup：tasks文件中；

在容器中执行top，这个docker的cgroup：tasks 会出现另一个进程，就是宿主机中看到的这个top进程。

容器取消top的执行，docker的cgroup：tasks 中这个进程号消失。

容器内部的所有进程，包括容器自身的进程，==宿主机都能够感知到，也都会在对应的cgroup中监控==。

上面的两个进程，2409是docker的pid，也就是这个bash的pid；4413是docker执行的top命令的pid



类似上面的，在cgroup/pids下，监控的进程，也会随着docker内部的命令而变化

![image-20211230144415455](image-20211230144415455.png)



`tasks`文件中是线程ID，`cgroup.procs`文件中是进程ID。

zx@zx:/sys/fs/cgroup/cpu/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat cgroup.procs
5784
6035
zx@zx:/sys/fs/cgroup/cpu/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat tasks
5784
6035





```shell
zx@zx:/sys/fs/cgroup$ cat /proc/cgroups
#controller名称  挂载位置ID   使用该controller的cgroup数量   是否启用
#subsys_name	hierarchy	num_cgroups		enabled
cpuset			10			3				1
cpu				3			107				1
cpuacct			3			107				1
blkio			9			107				1
memory			2			160				1
devices			4			107				1
freezer			8			3				1
net_cls			5			3				1
perf_event		7			3				1
net_prio		5			3				1
hugetlb			6			3				1
pids			12			115				1
rdma			11			1				1
zx@zx:/sys/fs/cgroup$
zx@zx:/sys/fs/cgroup$
zx@zx:/sys/fs/cgroup$
zx@zx:/sys/fs/cgroup$ cat /proc/5784/cgroup
#第一列hierarchy-ID与/proc/cgroups的第二列对应；第二列是controller名称；第三列是cgroup目录路径。
12:pids:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
11:rdma:/
10:cpuset:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
9:blkio:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
8:freezer:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
7:perf_event:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
6:hugetlb:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
5:net_cls,net_prio:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
4:devices:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
3:cpu,cpuacct:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
2:memory:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
1:name=systemd:/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83
0::/system.slice/containerd.service

#比如，3:cpu,cpuacct:/docker/8bf...   
#    {/sys/fs/cgroup} + {3:cpu,cpuacct 对应的相对路径：cpu,cpuacct} + {自身目录：/docker/8bf... }
#  = /sys/fs/cgroup/cpu,cpuacct/docker/8bf...  这个就是挂载目录

zx@zx:/sys/fs/cgroup$ ps aux | grep docker
root         948  0.0  0.9 1383656 79068 ?       Ssl  13:02   0:02 /usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock
zx          5918  0.0  0.5 1276496 46816 pts/4   Sl+  14:38   0:00 docker attach 8bf42f21ddf5

zx@zx:/sys/fs/cgroup$ cat /proc/948/cgroup
12:pids:/system.slice/docker.service
11:rdma:/
10:cpuset:/
9:blkio:/system.slice/docker.service
8:freezer:/
7:perf_event:/
6:hugetlb:/
5:net_cls,net_prio:/
4:devices:/system.slice/docker.service
3:cpu,cpuacct:/system.slice/docker.service
2:memory:/system.slice/docker.service
1:name=systemd:/system.slice/docker.service
0::/system.slice/docker.service

zx@zx:/sys/fs/cgroup$ cat /proc/5918/cgroup
12:pids:/user.slice/user-1000.slice/session-8.scope
11:rdma:/
10:cpuset:/
9:blkio:/user.slice
8:freezer:/
7:perf_event:/
6:hugetlb:/
5:net_cls,net_prio:/
4:devices:/user.slice
3:cpu,cpuacct:/user.slice
2:memory:/user.slice/user-1000.slice/session-8.scope
1:name=systemd:/user.slice/user-1000.slice/session-8.scope
0::/user.slice/user-1000.slice/session-8.scope
```



cgroup是以目录的形式呈现的，`/`是cgroup的根目录，注意cgroup的根目录和挂载目录不是一回事，cgroup可能挂载在/sys/fs/cgroup或者/tmp/cgroup等任意目录，无论挂载在哪里，cgroup的根目录都是“`/`”。

假设cgroup的cpu controller的挂载点是/sys/fs/cgroup/cpu，那么目录`/sys/fs/cgroup/cpu/cg1`对应的cgroup的目录是`/cg1`。

为什么要强调这个，因为在调查一个kubelet的问题的时候，不确定`--runtime-cgroups`参数值中要不要包含挂载点路径，直到[用cadvisor查出所有cgroup](https://www.lijiaocn.com/问题/2019/01/25/kubernetes-failed-to-get-cgroup-stats.html#直接用cadvisor查询所有cgroup)后，才确定不应该带有挂载路径。现在从Linux手册中找到支持了：

```shell
A cgroup filesystem initially contains a single root cgroup, '/',
which all processes belong to.  A new cgroup is created by creating a
directory in the cgroup filesystem:

      mkdir /sys/fs/cgroup/cpu/cg1
```







### net

```shell
#逐层深入，net_prio.ifpriomap一直不变，显示的是宿主机的if；net_prio.prioidx在变化
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
3: docker0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
5: veth9dfcb22@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default

zx@zx:/sys/fs/cgroup/net_cls,net_prio$ cat net_cls.classid
0
zx@zx:/sys/fs/cgroup/net_cls,net_prio$ cat net_prio.prioidx
1
zx@zx:/sys/fs/cgroup/net_cls,net_prio$ cat net_prio.ifpriomap
lo 0
enp0s3 0
docker0 0
veth9dfcb22 0

zx@zx:/sys/fs/cgroup/net_cls,net_prio$ cd docker/
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ cat cgroup.procs
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ cat net_cls.classid
0
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ cat net_prio.ifpriomap
lo 0
enp0s3 0
docker0 0
veth9dfcb22 0
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ cat net_prio.prioidx
2

zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker$ cd 8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83/
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat cgroup.procs
5784
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat net_cls.classid
0
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat net_prio.ifpriomap
lo 0
enp0s3 0
docker0 0
veth9dfcb22 0
zx@zx:/sys/fs/cgroup/net_cls,net_prio/docker/8bf42f21ddf537c69627fe6da34f3bea3ffc46e10120841c07a2529a6a1e7e83$ cat net_prio.prioidx
3
```



## cgroup 子系统 net_cls (Network classifier cgroup)

[cgroup 子系统之 net_cls 和 net_prio](https://ggaaooppeenngg.github.io/zh-CN/2017/05/19/cgroup-%E5%AD%90%E7%B3%BB%E7%BB%9F%E4%B9%8B-net-cls-%E5%92%8C-net-prio/)

net_cls 可以给 packet 打上 classid 的标签，用于过滤分类，有了上面的详细解释，这个 classid 的作用也非常明显了，就是用于标记`skb`所属的 qdisc class 的。

有了这个标签，流量控制器（tc）可以对不同的 cgroup 的 packet 起作用，Netfilter（iptables）也可以基于这个标签有对应的动作。创建一个 net_cls cgroup 对应的是创建一个 net_cls.classid 文件，这个文件初始化为 0。

可以写 16 进制的 0xAAAABBBB 到这个文件里面，**AAAA** 是 major 号，**BBBB** 是 minor 号。读这个文件返回的是十进制的数字。

这个值，既是cgroup：net_cls.classid，又是tc：handle

例子

```shell
#0x100001  0x00100001    0010：0001     10:1
mkdir /sys/fs/cgroup/net_cls
mount -t cgroup -onet_cls net_cls /sys/fs/cgroup/net_cls
mkdir /sys/fs/cgroup/net_cls/0
echo 0x100001 >  /sys/fs/cgroup/net_cls/0/net_cls.classid
```

设置一个 10:1 handle.  也就是classid=0x100001

```shell
cat /sys/fs/cgroup/net_cls/0/net_cls.classid
1048577
```

配置 tc:

```shell
tc qdisc add dev eth0 root handle 10: htb
tc class add dev eth0 parent 10: classid 10:1 htb rate 40mbit
```

创建 traffic class 10:1

```shell
tc filter add dev eth0 parent 10: protocol ip prio 10 handle 1: cgroup
```

配置 iptables，也可以用于这个 classid。

```shell
iptables -A OUTPUT -m cgroup ! --cgroup 0x100001 -j DROP
```

对应的实现在`net/core/netclassid_cgroup.c`下面。起作用的方式是`css_cls_state`的`classid`并且`sock_cgroup_set_classid(&sock->sk->sk_cgrp_data,(unsigned long)v)`来设置`sock`的`classid`。





`net_cls` 子系统使用等级识别符（classid）标记网络数据包，可允许 Linux 流量控制程序（**tc**）识别从具体 cgroup 中生成的数据包。可将流量控制程序配置为给不同 cgroup 中的数据包分配不同的优先权。

- net_cls.classid

    `net_cls.classid` 包含一个说明流量控制*句柄*的十六进制的值。例如：`0x1001` 代表通常写成 `10:1` 的句柄，这是 iproute2 使用的格式。这些句柄的格式为：`0x*AAAA**BBBB*`，其中 *AAAA* 是十六进制主设备号，*BBBB* 是十六进制副设备号。您可以忽略前面的零；`0x10001` 与 `0x00010001` 一样，代表 `1:1`。



net_cls实现的基本的思想就是将控制组和内核现有的网络包分类和调度的机制相关联。net_cls通过给控制组分配一个类标识符（classid）来指定该控制组的数据包将被分到哪个traffic class（net_cls只支持可分类的qdsic队列规则）。数据包在发送的时候会根据添加到设备qdisc上的cgroup filter将数据包分到与其classid相符的traffic class队列中，再由设备上设置的具体qdsic来控制数据包的发送，以达到控制网络资源使用的目的。

创建一个挂有net_cls的cgroup后，在其下会生有个名为net_cls.classid（默认初始值为0）的文件，通过这个文件指定该组进程相关的数据包进入哪个traffic class。通过向文件写入形如0xAAAABBBB的十六进制值（AAAA为主处理号，BBBB为次处理号，读取该值是以十进制显示），设置cgroup的classid后，再进行相应的tc配置，添加符合classid的traffic class，并使用cgroup filter。这样当cgroup中的进程需要使用网络接口发送数据包的时候，则会按照接口上classid相应的tc策略控制进程对网络资源的使用。

需要进行tc控制的进程，将自己的pid写入对应的cgroup.procs/tasks即可。

task、cgroup、tc规则 三者关联起来

task								cgroup								   	       tc

|---------------------->  net_cls.cgroup.procs/tasks

Pid							net_cls.classid		<----------------->  classid --------> tc-config

​                                                                                                          |

​                                                                                                          |--------> traffic class & cgroup filter

https://www.cnblogs.com/xingmuxin/p/10813386.html

https://blog.csdn.net/tanzhe2017/article/details/81001621

https://www.cnblogs.com/sammyliu/p/5886833.html

https://blog.csdn.net/hu1610552336/article/details/118642410
