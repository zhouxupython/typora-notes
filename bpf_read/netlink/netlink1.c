/**
http://guochongxin.github.io/c/c/c++/linux/netlink/rj45/%E7%BC%96%E7%A8%8B%E8%AF%AD%E8%A8%80/%E7%BD%91%E5%8D%A1/2014/12/05/tong_guo_netlink_jian_ce_wang_xian_cha_ba


最近有个需求需要检测RJ45网卡的网线有没有接上，而最近正在了解Netlink相关资料，刚好也看下通过Netlink可以进行检测，故在此做下粗略笔记：

    1.首先要创建一个Netlink Socket，在用户层使用如下参数来调用socket()函数：

       fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE); 

       上面这个函数第一个参数必须是AF_NETLINK或PF_NETLINK，这两个标志在Linux下是一样的，第二个参数可以是SOCK_RAW或SOCK_DGRAM（对应用到TCP或UDP协议），而最后一个参数NETLINK_ROUTE为“路由守护进程”，用于接收来自内核的路由通知事件。

    2.将上面创建的Socket绑定

       addr.nl_family = AF_NETLINK;  
       addr.nl_groups = RTNLGRP_LINK; //指定接收路由多播组消息  
       bind(fd, (struct sockaddr*)&addr, sizeof(addr)); 

       上面将创建的socket与相应的协议族和组进行绑定，接下来通过读fd这个socket来获得相应的消息数据struct nlmsghdr，再对该结构体数据进行判断来获得网线是接上或是拔掉，相应的源码如下：
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


