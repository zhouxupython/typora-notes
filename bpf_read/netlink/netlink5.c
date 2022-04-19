/**
https://stackoverflow.com/questions/42264101/does-libevent-support-netlink-socket
使用libevent

Yes, libevent supports netlink socket.
There is https://github.com/libevent/libevent/blob/master/sample/hello-world.c, it is modified below to listen to netlink socket.

The basic example listens to Linux network interface creation/deletion and can be executed with sudo to gain privilege needed.
It listens to same events as ip monitor link.
Another example of listening to RAW sockets with libevent is here
https://github.com/bodgit/libevent-natpmp/blob/master/natpmp.c.

*/
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <net/if.h>
#include <net/route.h>

#include <fcntl.h>
#include <unistd.h>
#include <event.h>
#include <string.h>
#include <errno.h>
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  

#define BUF_SIZE 10240

static void link_recvmsg(int fd, short event, void *arg)
{
    char          buf[NLMSG_SPACE(BUF_SIZE)] = {0};
    socklen_t     socklen;
    struct iovec  iov = {.iov_base = buf, .iov_len = sizeof(buf)};
    struct sockaddr addr;
    memset(&addr, 0, sizeof(struct sockaddr));

    if (!fd || -1 == fd)
        return;

    int status = getsockname(fd, &addr, &socklen);
    if(-1 == status)
        return;

    struct msghdr mh = {.msg_name = NULL, .msg_namelen = 0, .msg_iov = &iov, .msg_iovlen = 1,
        .msg_flags = 0, .msg_name = &addr, .msg_namelen = sizeof(struct sockaddr)};

    status = recvmsg(fd, &mh, 0);
    if ((-1 == status) && ((EINTR == errno) || (EAGAIN == errno)))
        return;
    if(-1 == status)
        return;
    if ((mh.msg_flags & MSG_TRUNC) == MSG_TRUNC)
        return;
    if ((mh.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
        return;

    for (const struct nlmsghdr *h = (struct nlmsghdr *)buf; NLMSG_OK(h, status); h = NLMSG_NEXT(h, status)) {
        switch (h->nlmsg_type) {
            case RTM_NEWLINK:
                fprintf(stderr, "got RTM_NEWLINK\n");
                break;
            case RTM_DELLINK:
                fprintf(stderr, "got RTM_DELLINK\n");
                break;
            default:
                fprintf(stderr, "unexpected case in swtch statement\n");
                break;
        }
    }
}
int main(int argc, char **argv)
{
    /* some init code here */
    /* NETLINK socket */
    int status;
    int buf_size = BUF_SIZE; 
    struct sockaddr_nl src_addr;

    struct event_base *base;
    base = event_base_new();


    int socket_nl = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK, NETLINK_ROUTE);
    if(-1 == socket_nl) return -1;

    memset(&src_addr, 0, sizeof(struct sockaddr_nl));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups |= RTNLGRP_LINK;

    status = setsockopt(socket_nl, SOL_SOCKET, SO_RCVBUF,
            &buf_size, sizeof(buf_size));
    if(-1 == status) return -1; 

    status = bind(socket_nl, (struct sockaddr *)&src_addr, sizeof(struct sockaddr_nl));
    if(status < 0) return -1;

    static struct event nl_ev;
    event_set(&nl_ev, socket_nl, EV_READ|EV_PERSIST, link_recvmsg,
            NULL);
    if (base) {
        event_base_set(base, &nl_ev);
    }
    event_add(&nl_ev, NULL);
    /* some other code, dispatch event and deinit */
    event_base_dispatch(base);
}
