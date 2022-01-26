# bcc流程分析

## perf

[perf工作原理简析](https://my.oschina.net/u/2475751/blog/1823736)

perf是由用户态的perf tool命令和内核态perf驱动两部分，加上一个连通用户态和内核态的系统调用sys_perf_event_open组成。

perf系统调用定义如下(linux/kernel/events/core.c):

```c
/**
 * sys_perf_event_open - open a performance event, associate it to a task/cpu
 *
 * @attr_uptr:  event_id type attributes for monitoring/sampling
 * @pid:                target pid
 * @cpu:                target cpu
 * @group_fd:           group leader event fd
 */
SYSCALL_DEFINE5(perf_event_open,
                struct perf_event_attr __user *, attr_uptr,
                pid_t, pid, int, cpu, int, group_fd, unsigned long, flags)
```

```c
  perf_event ioctl calls
       Various ioctls act on perf_event_open() file descriptors:

PERF_EVENT_IOC_SET_BPF (since Linux 4.1)
              This allows attaching a Berkeley Packet Filter (BPF)
              program to an existing kprobe tracepoint event.  You need
              CAP_PERFMON (since Linux 5.8) or CAP_SYS_ADMIN privileges
              to use this ioctl.

              The argument is a BPF program file descriptor that was
              created by a previous bpf(2) system call.
    
PERF_EVENT_IOC_ENABLE
              This enables the individual event or event group specified
              by the file descriptor argument.

              If the PERF_IOC_FLAG_GROUP bit is set in the ioctl
              argument, then all events in a group are enabled, even if
              the event specified is not the group leader (but see
              BUGS).
```

