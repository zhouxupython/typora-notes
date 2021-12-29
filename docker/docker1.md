# docker

### install

```shell
# in ubuntu20.04
sudo apt-get update
sudo apt-get install -y apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add - 
sudo add-apt-repository "deb [arch=amd64] http://mirrors.aliyun.com/docker-ce/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get install -y docker-ce
```



### [docker 镜像加速，修改为阿里云镜像](https://www.cnblogs.com/codeBang/p/11904924.html)

```shell
zx@u18-1:/etc/docker$ sudo cat daemon.json
{
  "registry-mirrors": ["http://hub-mirror.c.163.com"]
}

sudo systemctl daemon-reload
sudo systemctl restart docker

或者：
zx@zx-docker:~$ sudo cat /etc/docker/daemon.json
[sudo] password for zx:
{
  "registry-mirrors": ["https://7ixh250y.mirror.aliyuncs.com"]
}

```



### docker run -p

```shell
# 指定端口映射，格式为：宿主机IP:宿主机端口:容器端口

# Bind TCP port 8080 of the container to TCP port 80 on 127.0.0.1 of the host machine.
$ sudo docker run -p 127.0.0.1:80:8080 <image> <cmd> 

# Bind TCP port 8080 of the container to a dynamically allocated TCP port on 127.0.0.1 of the host machine. 
$ sudo docker run -p 127.0.0.1::8080 <image> <cmd> 

# Bind TCP port 8080 of the container to TCP port 80 on all available interfaces of the host machine. 
$ sudo docker run -p 80:8080 <image> <cmd> 

# Bind TCP port 8080 of the container to a dynamically allocated TCP port on all available interfaces 
$ sudo docker run -p 8080 <image> <cmd>

# Bind UDP port 5353 of the container to UDP port 53 on 127.0.0.1 of the host machine. 
$ sudo docker run -p 127.0.0.1:53:5353/udp <image> <cmd>

# https://blog.csdn.net/zhangyaouuuu/article/details/80094857
```



### 重新运行退出的容器

```shell
zx@zx-docker:~$ docker run -ti ubuntu-net bash OR  docker run -it --name c1 --net backend ubuntu-net /bin/bash
exit

zx@zx-docker:~$ docker ps -a
CONTAINER ID   IMAGE        COMMAND       CREATED        STATUS                      PORTS     NAMES
b518627b0da2   ubuntu-net   "/bin/bash"   46 hours ago   Exited (0) 22 hours ago     
zx@zx-docker:~$
zx@zx-docker:~$ docker start b518627b0da2
b518627b0da2
zx@zx-docker:~$
zx@zx-docker:~$ docker ps
CONTAINER ID   IMAGE        COMMAND       CREATED        STATUS         PORTS     NAMES
b518627b0da2   ubuntu-net   "/bin/bash"   46 hours ago   Up 8 seconds             c3
zx@zx-docker:~$
zx@zx-docker:~$ docker attach b518627b0da2
root@b518627b0da2:/#

```



### 保存修改过的容器为镜像

zx@zx-docker:~$ docker ps
CONTAINER ID   IMAGE     COMMAND   CREATED       STATUS       PORTS     NAMES
bfca866a2e61   ubuntu    "bash"    2 hours ago   Up 2 hours             elated_edison
zx@zx-docker:~$ docker `commit`  bfca866a2e61 ubuntu-net
sha256:aef5b30ea099683e89b205366d2c3ca1ec08de2705ebc97e69b7aa004bac73c1

zx@zx-docker:~$ docker images
REPOSITORY    TAG       IMAGE ID       CREATED          SIZE
ubuntu-net    latest    aef5b30ea099   15 minutes ago   110MB
ubuntu        latest    ba6acccedd29   2 months ago     72.8MB
hello-world   latest    feb5d9fea6a5   3 months ago     13.3kB
zx@zx-docker:~$
zx@zx-docker:~$ docker `save` aef5b30ea099 -o ~/mydocker_images/ubuntu-net
zx@zx-docker:~$ ll ~/mydocker_images
-rw-------  1 zx zx 112392192 12月 24 00:54 ubuntu-net

### 停止所有容器

➜  ~ docker ps -a | grep "Exited" | awk '{print $1 }'|xargs docker stop

### 删除所有的容器

zx@zx-docker:~$ docker ps -a
CONTAINER ID   IMAGE        COMMAND       CREATED      STATUS                    PORTS     NAMES
b518627b0da2   ubuntu-net   "/bin/bash"   2 days ago   Up 2 hours                          c3

zx@zx-docker:~$ docker ps -a  -q
b518627b0da2

 docker rm $(docker ps -a  -`q`)

### 删除所有已退出的容器
➜  ~ docker ps -a | grep "Exited" | awk '{print $1 }'|xargs docker rm

### 删除所有none镜像
➜  ~ docker images|grep none|awk '{print $3 }'|xargs docker rmi



### 如何解决 image has dependent child images 错误

删除镜像ba6acccedd29有问题，因为会有其他镜像是依赖这个镜像的

zx@zx-docker:~$ docker images
REPOSITORY        TAG       IMAGE ID       CREATED        SIZE
ubuntu-stress     latest    5c3cceec4fc2   24 hours ago   106MB
ubuntu-net        latest    39d8ac57f22e   2 days ago     151MB
ubuntu            latest    `ba6acccedd29`   2 months ago   72.8MB
progrium/stress   latest    db646a8f4087   7 years ago    282MB
zx@zx-docker:~$查看依赖`ba6acccedd29`的镜像
zx@zx-docker:~$ docker image inspect --format='{{.RepoTags}} {{.Id}} {{.Parent}}' $(docker image ls -q --filter since=`ba6acccedd29`)
[ubuntu-stress:latest] sha256:5c3cceec4fc2...   sha256:ba6acccedd29...
[ubuntu-net:latest] sha256:39d8ac57f22e... sha256:ba6acccedd29...



https://www.jianshu.com/p/3c2e0a89d618
https://blog.csdn.net/weixin_30677073/article/details/96539016
https://segmentfault.com/a/1190000011153919



### veth pair

root@48dce31eb6c0:/# ethtool -S eth0
NIC statistics:
     peer_ifindex: `47`

zx@zx-docker:~$ ip a
`47`: vetha0dc811@if`46`:



### 这个d4f77f58e7f0表示什么

zx@zx-docker:/sys/fs/cgroup/cpu$ sudo ls /var/run/docker/netns  -l
total 0
-r--r--r-- 1 root root 0 12月 29 21:41 `d4f77f58e7f0`



zx@zx-docker:/sys/fs/cgroup/cpu$ sudo ls /proc/2744/ns/net     某容器PID
/proc/2744/ns/net
zx@zx-docker:/sys/fs/cgroup/cpu$ sudo readlink /proc/2744/ns/net
net:[4026532197]





### 网络学习的镜像需要安装


traceroute nmap net-tools iproute2 ethtool bridge-utils curl inetutils-ping



### 两个同网段容器互ping

zx@zx-docker:~$ ip a
3: docker0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
    link/ether 02:42:56:2d:d2:a6 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:56ff:fe2d:d2a6/64 scope link
       valid_lft forever preferred_lft forever
51: veth0838f56@if50: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default
    link/ether c2:fc:c6:ee:72:0e brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::c0fc:c6ff:feee:720e/64 scope link
       valid_lft forever preferred_lft forever
53: veth75a7a9a@if52: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker0 state UP group default
    link/ether fe:db:6b:87:3e:e3 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet6 fe80::fcdb:6bff:fe87:3ee3/64 scope link
       valid_lft forever preferred_lft forever
zx@zx-docker:~$ brctl show
bridge name     bridge id               STP enabled     interfaces
docker0         8000.0242562dd2a6       no               veth0838f56
                                                        								veth75a7a9a

zx@zx-docker:~$ sudo  tcpdump -i veth75a7a9a
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on veth75a7a9a, link-type EN10MB (Ethernet), capture size 262144 bytes
23:38:08.794042 IP 172.17.0.3.37498 > 172.17.0.2.33434: UDP, length 32
23:38:08.794095 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33434 unreachable, length 68
23:38:08.794112 IP 172.17.0.3.59894 > 172.17.0.2.33435: UDP, length 32
23:38:08.794123 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33435 unreachable, length 68
23:38:08.794135 IP 172.17.0.3.48602 > 172.17.0.2.33436: UDP, length 32
23:38:08.794145 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33436 unreachable, length 68
23:38:08.794157 IP 172.17.0.3.60037 > 172.17.0.2.33437: UDP, length 32
23:38:08.794167 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33437 unreachable, length 68
23:38:08.794179 IP 172.17.0.3.56982 > 172.17.0.2.33438: UDP, length 32
23:38:08.794188 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33438 unreachable, length 68
23:38:08.794199 IP 172.17.0.3.52889 > 172.17.0.2.33439: UDP, length 32
23:38:08.794208 IP 172.17.0.2 > 172.17.0.3: ICMP 172.17.0.2 udp port 33439 unreachable, length 68
23:38:08.794219 IP 172.17.0.3.45400 > 172.17.0.2.33440: UDP, length 32
23:38:08.794233 IP 172.17.0.3.42248 > 172.17.0.2.33441: UDP, length 32
23:38:08.794246 IP 172.17.0.3.44393 > 172.17.0.2.33442: UDP, length 32
23:38:08.794261 IP 172.17.0.3.40178 > 172.17.0.2.33443: UDP, length 32
23:38:08.794274 IP 172.17.0.3.41526 > 172.17.0.2.33444: UDP, length 32
23:38:08.794287 IP 172.17.0.3.57453 > 172.17.0.2.33445: UDP, length 32
23:38:08.794300 IP 172.17.0.3.33179 > 172.17.0.2.33446: UDP, length 32
23:38:08.794313 IP 172.17.0.3.43824 > 172.17.0.2.33447: UDP, length 32
23:38:08.794326 IP 172.17.0.3.35676 > 172.17.0.2.33448: UDP, length 32
23:38:08.794339 IP 172.17.0.3.49172 > 172.17.0.2.33449: UDP, length 32
23:38:08.794978 IP 172.17.0.3.50945 > dns1.ctcdma.com.domain: 11890+ PTR? 2.0.17.172.in-addr.arpa. (41)
23:38:08.800531 IP dns1.ctcdma.com.domain > 172.17.0.3.50945: 11890 NXDomain* 0/0/0 (41)
23:38:13.801897 ARP, Request who-has 172.17.0.3 tell zx-docker, length 28
23:38:13.801967 ARP, Request who-has zx-docker tell 172.17.0.3, length 28
23:38:13.801974 ARP, Reply zx-docker is-at 02:42:56:2d:d2:a6 (oui Unknown), length 28
23:38:13.801977 ARP, Request who-has 172.17.0.3 tell 172.17.0.2, length 28
23:38:13.801978 ARP, Request who-has 172.17.0.2 tell 172.17.0.3, length 28
23:38:13.801980 ARP, Reply 172.17.0.3 is-at 02:42:ac:11:00:03 (oui Unknown), length 28
23:38:13.801988 ARP, Reply 172.17.0.3 is-at 02:42:ac:11:00:03 (oui Unknown), length 28
23:38:13.801990 ARP, Reply 172.17.0.2 is-at 02:42:ac:11:00:02 (oui Unknown), length 28
23:39:18.744635 IP zx-docker.mdns > 224.0.0.251.mdns: 0 [2q] PTR (QM)? _ipps._tcp.local. PTR (QM)? _ipp._tcp.local. (45)



**同时抓某个veth和docker0，数据包基本上一样**

zx@zx-docker:~$ sudo tcpdump -i veth76c677a
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on veth76c677a, link-type EN10MB (Ethernet), capture size 262144 bytes
22:48:07.100945 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 0, length 64
22:48:07.101005 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 0, length 64
22:48:08.101417 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 1, length 64
22:48:08.101465 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 1, length 64
22:48:09.102692 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 2, length 64
22:48:09.102740 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 2, length 64
22:48:10.103725 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 3, length 64
22:48:10.103788 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 3, length 64
22:48:12.264827 ARP, Request who-has 172.17.0.2 tell 172.17.0.3, length 28
22:48:12.264849 ARP, Request who-has 172.17.0.3 tell 172.17.0.2, length 28
22:48:12.264867 ARP, Reply 172.17.0.2 is-at 02:42:ac:11:00:02 (oui Unknown), length 28
22:48:12.264871 ARP, Reply 172.17.0.3 is-at 02:42:ac:11:00:03 (oui Unknown), length 28



zx@zx-docker:~$ sudo tcpdump -i docker0
[sudo] password for zx: 
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on docker0, link-type EN10MB (Ethernet), capture size 262144 bytes
22:48:07.100986 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 0, length 64
22:48:07.101006 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 0, length 64
22:48:08.101444 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 1, length 64
22:48:08.101466 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 1, length 64
22:48:09.102718 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 2, length 64
22:48:09.102743 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 2, length 64
22:48:10.103759 IP 172.17.0.2 > 172.17.0.3: ICMP echo request, id 12, seq 3, length 64
22:48:10.103790 IP 172.17.0.3 > 172.17.0.2: ICMP echo reply, id 12, seq 3, length 64
22:48:12.264844 ARP, Request who-has 172.17.0.2 tell 172.17.0.3, length 28
22:48:12.264853 ARP, Request who-has 172.17.0.3 tell 172.17.0.2, length 28
22:48:12.264869 ARP, Reply 172.17.0.2 is-at 02:42:ac:11:00:02 (oui Unknown), length 28
22:48:12.264872 ARP, Reply 172.17.0.3 is-at 02:42:ac:11:00:03 (oui Unknown), length 28