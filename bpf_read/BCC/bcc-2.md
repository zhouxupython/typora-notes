# 2





|                                                              |                                                              |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [【译】eBPF 概述：第 1 部分：介绍](https://www.ebpf.top/post/ebpf-overview-part-1/) | ==libbpf==:<br/>bpf_create_map(BPF_MAP_TYPE_ARRAY, sizeof(key), sizeof(value),	256, 0)<br/>bpf_load_program(BPF_PROG_TYPE_SOCKET_FILTER, prog, insns_cnt, 				   "GPL", 0, bpf_log_buf, BPF_LOG_BUF_SIZE)<br/>setsockopt(sock, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd, sizeof(prog_fd)<br/> |
| [【译】eBPF 概述：第 2 部分：机器和字节码](https://www.ebpf.top/post/ebpf-overview-part-2/) |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |





[【译】eBPF 概述：第 1 部分：介绍](https://www.ebpf.top/post/ebpf-overview-part-1/)

```c
/* include/uapi/linux/in.h#L28 
Standard well-defined IP protocols.  
*/
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
#define IPPROTO_IP		IPPROTO_IP
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
#define IPPROTO_ICMP		IPPROTO_ICMP
  IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
#define IPPROTO_IGMP		IPPROTO_IGMP
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
#define IPPROTO_IPIP		IPPROTO_IPIP
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
#define IPPROTO_TCP		IPPROTO_TCP
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
#define IPPROTO_EGP		IPPROTO_EGP
  IPPROTO_PUP = 12,		/* PUP protocol				*/
#define IPPROTO_PUP		IPPROTO_PUP
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
```









[【译】eBPF 概述：第 2 部分：机器和字节码](https://www.ebpf.top/post/ebpf-overview-part-2/)







