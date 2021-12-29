# docker_cgroup

```shell

```

zx@zx-docker:~$ docker ps -a
CONTAINER ID   IMAGE        COMMAND       CREATED        STATUS                      PORTS     NAMES
`b518627b0da2`   ubuntu-net   "/bin/bash"   46 hours ago   Exited (0) 22 hours ago               c3
zx@zx-docker:~$ docker start b518627b0da2
b518627b0da2
zx@zx-docker:~$ docker inspect b518627b0da2 | grep -i pid
            "Pid": `2744`,
zx@zx-docker:~$ cd /sys/fs/cgroup/
zx@zx-docker:/sys/fs/cgroup$ cd cpu
zx@zx-docker:/sys/fs/cgroup/cpu$ cd docker/
zx@zx-docker:/sys/fs/cgroup/cpu/docker$ ls
`b518627b0da25ab...`  cpuacct.usage_percpu_sys   cpu.stat
cgroup.clone_children                                             cpuacct.usage_percpu_user  cpu.uclamp.max
cgroup.procs                                                      cpuacct.usage_sys          cpu.uclamp.min
cpuacct.stat                                                      cpuacct.usage_user         notify_on_release
cpuacct.usage                                                     cpu.cfs_period_us          tasks
cpuacct.usage_all                                                 cpu.cfs_quota_us
cpuacct.usage_percpu                                              cpu.shares
zx@zx-docker:/sys/fs/cgroup/cpu/docker$
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
2744
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



***好像只有这个路径下执行才有效***		

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

zx@zx-docker:/sys/fs/cgroup/cpu$ cgget -r cpu.cfs_quota_us   docker/b518627b0da25ab...
docker/b518627b0da25ab... :
cpu.cfs_quota_us: -1