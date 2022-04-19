kern

```c
/* Copyright (c) 2015 PLUMgrid, http://plumgrid.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#include <linux/ptrace.h>
#include <linux/version.h>
#include <uapi/linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

struct pair {
	u64 val;
	u64 ip;
};

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, long);// 申请到的内存地址
	__type(value, struct pair);// {time, 调用函数}
	__uint(max_entries, 1000000);
} my_map SEC(".maps");

/* kprobe is NOT a stable ABI. If kernel internals change this bpf+kprobe
 * example will no longer be meaningful
 */
SEC("kprobe/kmem_cache_free")
/*
void kmem_cache_free (	struct kmem_cache * cachep,
 						void * objp);
*/
int bpf_prog1(struct pt_regs *ctx)
{
	long ptr = PT_REGS_PARM2(ctx);// 获取kmem_cache_free的第二个入参，就是需要释放的内存地址，是alloc申请到的地址

	bpf_map_delete_elem(&my_map, &ptr);// ptr就是map的key，内存释放后，就把这个键值对删除
	return 0;
}

SEC("kretprobe/kmem_cache_alloc_node")//必须挂载到 ret上，才能保证内存已经申请到了
/*
void * kmem_cache_alloc_node (	struct kmem_cache * cachep,
								gfp_t flags,
								int nodeid);
*/
int bpf_prog2(struct pt_regs *ctx)
{
	long ptr = PT_REGS_RC(ctx);//rax，宏获取函数返回值，这个函数挂载到kmem_cache_alloc_node ret上，返回值就是申请到的内存地址
	long ip = 0;

	/* ！！！get ip address of kmem_cache_alloc_node() caller */
	/* ip寄存器，调用处指令的下一条指令的地址，处于 kmem_cache_alloc_node 的调用函数内 */
	BPF_KRETPROBE_READ_RET_IP(ip, ctx);

	struct pair v = {
		.val = bpf_ktime_get_ns(),
		.ip = ip,
	};

	bpf_map_update_elem(&my_map, &ptr, &v, BPF_ANY);//map的key就是 申请到的内存地址
	return 0;
}
char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;

```

```c
// strace 追踪到的关键系统调用如下：

    execve("./tracex4", ["./tracex4"], ["LANG=en_US.UTF-8", "LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca"..., "TERM=xterm", "DISPLAY=localhost:12.0", "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin", "MAIL=/var/mail/root", "LOGNAME=root", "USER=root", "HOME=/root", "SHELL=/bin/bash", "SUDO_COMMAND=/usr/bin/strace -v -f -s 128 -o tracex4.txt ./tracex4", "SUDO_USER=wu", "SUDO_UID=1000", "SUDO_GID=1000"]) = 0
    ... ...
    bpf(BPF_MAP_CREATE, {map_type=BPF_MAP_TYPE_HASH, key_size=8, value_size=16, max_entries=1000000, map_flags=0, inner_map_fd=0, map_name="my_map", map_ifindex=0, btf_fd=0, btf_key_type_id=0, btf_value_type_id=0}, 112) = 4
    bpf(BPF_PROG_LOAD, {prog_type=BPF_PROG_TYPE_KPROBE, insn_cnt=9, insns=[{code=BPF_LDX|BPF_DW|BPF_MEM, dst_reg=BPF_REG_1, src_reg=BPF_REG_1, off=104, imm=0}, {code=BPF_STX|BPF_DW|BPF_MEM, dst_reg=BPF_REG_10, src_reg=BPF_REG_1, off=-8, imm=0}, {code=BPF_ALU64|BPF_X|BPF_MOV, dst_reg=BPF_REG_2, src_reg=BPF_REG_10, off=0, imm=0}, {code=BPF_ALU64|BPF_K|BPF_ADD, dst_reg=BPF_REG_2, src_reg=BPF_REG_0, off=0, imm=0xfffffff8}, {code=BPF_LD|BPF_DW|BPF_IMM, dst_reg=BPF_REG_1, src_reg=BPF_REG_1, off=0, imm=0x4}, {code=BPF_LD|BPF_W|BPF_IMM, dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}, {code=BPF_JMP|BPF_K|BPF_CALL, dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0x3}, {code=BPF_ALU64|BPF_K|BPF_MOV, dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}, {code=BPF_JMP|BPF_K|BPF_EXIT, dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}], license="GPL", log_level=0, log_size=0, log_buf=NULL, kern_version=KERNEL_VERSION(5, 4, 0), prog_flags=0, prog_name="", prog_ifindex=0, expected_attach_type=BPF_CGROUP_INET_INGRESS, prog_btf_fd=0, func_info_rec_size=0, func_info=NULL, func_info_cnt=0, line_info_rec_size=0, line_info=NULL, line_info_cnt=0, attach_btf_id=0}, 112) = 5
    openat(AT_FDCWD, "/sys/kernel/debug/tracing/kprobe_events", O_WRONLY|O_APPEND) = 6
    write(6, "p:kmem_cache_free kmem_cache_free", 33) = 33
    close(6)
    openat(AT_FDCWD, "/sys/kernel/debug/tracing/events/kprobes/kmem_cache_free/id", O_RDONLY) = 6
    read(6, "2145\n", 256)		 = 5
    close(6)
    perf_event_open({type=PERF_TYPE_TRACEPOINT, size=0 /* PERF_ATTR_SIZE_??? */, config=2145, sample_period=1, sample_type=PERF_SAMPLE_RAW, read_format=0, disabled=0, inherit=0, pinned=0, exclusive=0, exclusive_user=0, exclude_kernel=0, exclude_hv=0, exclude_idle=0, mmap=0, comm=0, freq=0, inherit_stat=0, enable_on_exec=0, task=0, watermark=0, precise_ip=0 /* arbitrary skid */, mmap_data=0, sample_id_all=0, exclude_host=0, exclude_guest=0, exclude_callchain_kernel=0, exclude_callchain_user=0, mmap2=0, comm_exec=0, use_clockid=0, context_switch=0, write_backward=0, namespaces=0, wakeup_events=1, config1=0}, -1, 0, -1, 0) = 6
    ... ...
    openat(AT_FDCWD, "/sys/kernel/debug/tracing/kprobe_events", O_WRONLY|O_APPEND) = 8
    write(8, "r:kmem_cache_alloc_node kmem_cache_alloc_node", 45) = 45
    openat(AT_FDCWD, "/sys/kernel/debug/tracing/events/kprobes/kmem_cache_alloc_node/id", O_RDONLY) = 8
    read(8, "2146\n", 256)		 = 5
    close(8)			 = 0
    perf_event_open({type=PERF_TYPE_TRACEPOINT, size=0 /* PERF_ATTR_SIZE_??? */, config=2146, sample_period=1, sample_type=PERF_SAMPLE_RAW, read_format=0, disabled=0, inherit=0, pinned=0, exclusive=0, exclusive_user=0, exclude_kernel=0, exclude_hv=0, exclude_idle=0, mmap=0, comm=0, freq=0, inherit_stat=0, enable_on_exec=0, task=0, watermark=0, precise_ip=0 /* arbitrary skid */, mmap_data=0, sample_id_all=0, exclude_host=0, exclude_guest=0, exclude_callchain_kernel=0, exclude_callchain_user=0, mmap2=0, comm_exec=0, use_clockid=0, context_switch=0, write_backward=0, namespaces=0, wakeup_events=1, config1=0}, -1, 0, -1, 0) = 8
    ioctl(8, PERF_EVENT_IOC_ENABLE, 0) = 0
    ioctl(8, PERF_EVENT_IOC_SET_BPF, 7) = 0
    write(1, "\33[1;1H\33[2J\0\0", 12) = 12
    bpf(BPF_MAP_GET_NEXT_KEY, {map_fd=4, key=0x7ffffaf162b0, next_key=0x7ffffaf162b8}, 112) = 0
    bpf(BPF_MAP_LOOKUP_ELEM, {map_fd=4, key=0x7ffffaf162b8, value=0x7ffffaf162d0, flags=BPF_ANY}, 112) = 0
```







user

```c
// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2015 PLUMgrid, http://plumgrid.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>

struct pair {
	long long val;
	__u64 ip;
};

static __u64 time_get_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void print_old_objects(int fd)
{
	long long val = time_get_ns();
	__u64 key, next_key;
	struct pair v;

	key = write(1, "\e[1;1H\e[2J", 12); /* clear screen */

	key = -1;
	while (bpf_map_get_next_key(fd, &key, &next_key) == 0) {
		bpf_map_lookup_elem(fd, &next_key, &v);
		key = next_key;
		if (val - v.val < 1000000000ll)
			/* object was allocated more then 1 sec ago */
			continue;
		printf("obj 0x%llx is %2lldsec old was allocated at ip %llx\n",
		       next_key, (val - v.val) / 1000000000ll, v.ip);
	}
}

int main(int ac, char **argv)
{
	struct bpf_link *links[2];
	struct bpf_program *prog;
	struct bpf_object *obj;
	char filename[256];
	int map_fd, i, j = 0;

	snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);
	obj = bpf_object__open_file(filename, NULL);
	if (libbpf_get_error(obj)) {
		fprintf(stderr, "ERROR: opening BPF object file failed\n");
		return 0;
	}

	/* load BPF program */
	if (bpf_object__load(obj)) {
		fprintf(stderr, "ERROR: loading BPF object file failed\n");
		goto cleanup;
	}

	map_fd = bpf_object__find_map_fd_by_name(obj, "my_map");
	if (map_fd < 0) {
		fprintf(stderr, "ERROR: finding a map in obj file failed\n");
		goto cleanup;
	}

	bpf_object__for_each_program(prog, obj) {
		links[j] = bpf_program__attach(prog);
		if (libbpf_get_error(links[j])) {
			fprintf(stderr, "ERROR: bpf_program__attach failed\n");
			links[j] = NULL;
			goto cleanup;
		}
		j++;
	}

	for (i = 0; ; i++) {
		print_old_objects(map_fd);
		sleep(1);
	}

cleanup:
	for (j--; j >= 0; j--)
		bpf_link__destroy(links[j]);

	bpf_object__close(obj);
	return 0;
}

```

