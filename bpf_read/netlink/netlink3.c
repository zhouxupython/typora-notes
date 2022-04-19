/**
https://blog.csdn.net/sourthstar/article/details/7975999

http://www.cpplive.com/html/1542.html
之前有一篇文章《Netlink实现Linux内核与用户空间通信》专门介绍了Netlink相比其他内核交互方式的优点以及Netlink的调用方法，并以NETLINK_KOBJECT_UEVENT（内核事件向用户态通知）为例演示了U盘热插拔信息的捕捉，衍生出另一篇文章《Linux下自动检测USB热插拔》，今天尝试用Netlink来捕捉一下网络接口信息，实现的主要功能是实时打印发生变化的网络接口的序列号、上下线状态和接口名称。    

为了创建一个 netlink socket，用户需要使用如下参数调用 socket():

fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  
 
第一个参数必须是 AF_NETLINK 或 PF_NETLINK，在 Linux 中，它们俩实际为一个东西，它表示要使用netlink，第二个参数必须是SOCK_RAW或SOCK_DGRAM，第三个参数指定netlink协议类型，NETLINK_ROUTE意为“路由守护进程”，绑定该协议创建的fd可以接收到来自内核的路由通知事件（如网路接口eth0上线）。

函数 bind() 用于把一个打开的 netlink socket 与 netlink 源 socket 地址绑定在一起。netlink socket 的地址初始化及绑定如下：


addr.nl_family = AF_NETLINK;  
addr.nl_groups = RTNLGRP_LINK; //指定接收路由多播组消息  
bind(fd, (struct sockaddr*)&addr, sizeof(addr));  

*/

#include <sys/types.h>  
#include <sys/socket.h>  
#include <asm/types.h>  
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <string.h>  
  
#define BUFLEN 20480  
  
int main(int argc, char *argv[])  
{  
    int fd, retval;  
    char buf[BUFLEN] = {0};  
    int len = BUFLEN;  
    struct sockaddr_nl addr;  
    struct nlmsghdr *nh;  
    struct ifinfomsg *ifinfo;  
    struct rtattr *attr;  
  
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));  
    memset(&addr, 0, sizeof(addr));  
    addr.nl_family = AF_NETLINK;  
    addr.nl_groups = RTNLGRP_LINK;  
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));  
    while ((retval = read(fd, buf, BUFLEN)) > 0)  
    {  
        for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))  
        {  
            if (nh->nlmsg_type == NLMSG_DONE)  
                break;  
            else if (nh->nlmsg_type == NLMSG_ERROR)  
                return;  
            else if (nh->nlmsg_type != RTM_NEWLINK)  
                continue;  
            ifinfo = NLMSG_DATA(nh);  
            printf("%u: %s", ifinfo->ifi_index,  
                    (ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );  
            attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));  
            len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));  
            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))  
            {  
                if (attr->rta_type == IFLA_IFNAME)  
                {  
                    printf(" %s", (char*)RTA_DATA(attr));  
                    break;  
                }  
            }  
            printf("\n");  
        }  
    }  
  
    return 0;  
}  
