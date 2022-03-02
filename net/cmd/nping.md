# nping

```shell
zx@zx:~$ sudo ip netns exec sns1 ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
80: eth0@if79: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether 26:11:16:b2:88:f4 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 110.1.1.2/16 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::2411:16ff:feb2:88f4/64 scope link 
       valid_lft forever preferred_lft forever
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns2 ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
82: eth0@if81: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether 0a:f1:1a:5c:89:24 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 110.1.1.3/16 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::8f1:1aff:fe5c:8924/64 scope link 
       valid_lft forever preferred_lft forever
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns1 nping --icmp -c 1 --ttl 2 110.1.1.3
Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2022-01-04 22:00 CST
SENT (0.0027s) ICMP [110.1.1.2 > 110.1.1.3 Echo request (type=8/code=0) id=8083 seq=1] IP [ttl=2 id=3493 iplen=28 ]
RCVD (0.0029s) ICMP [110.1.1.3 > 110.1.1.2 Echo reply (type=0/code=0) id=8083 seq=1] IP [ttl=64 id=17087 iplen=28 ]
 
Max rtt: 0.111ms | Min rtt: 0.111ms | Avg rtt: 0.111ms
Raw packets sent: 1 (28B) | Rcvd: 1 (28B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 1.00 seconds
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns1 nping --tcp -c 1 --ttl 2 110.1.1.3
Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2022-01-04 21:51 CST
SENT (0.0025s) TCP 110.1.1.2:58251 > 110.1.1.3:80 S ttl=2 id=19254 iplen=40  seq=1074966041 win=1480 
RCVD (0.0027s) TCP 110.1.1.3:80 > 110.1.1.2:58251 RA ttl=64 id=0 iplen=40  seq=0 win=0 
 
Max rtt: 0.131ms | Min rtt: 0.131ms | Avg rtt: 0.131ms
Raw packets sent: 1 (40B) | Rcvd: 1 (40B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 1.00 seconds
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns1 nping --udp -c 1 --ttl 2 110.1.1.3
Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2022-01-04 21:51 CST
SENT (0.0025s) UDP 110.1.1.2:53 > 110.1.1.3:40125 ttl=2 id=2494 iplen=28 
RCVD (0.0027s) ICMP [110.1.1.3 > 110.1.1.2 Port unreachable (type=3/code=3) ] IP [ttl=64 id=7836 iplen=56 ]
 
Max rtt: 0.113ms | Min rtt: 0.113ms | Avg rtt: 0.113ms
Raw packets sent: 1 (28B) | Rcvd: 1 (56B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 1.00 seconds
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns1 nping --tcp -c 1 --ttl 2  -g 1111 -p 2222   110.1.1.3
Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2022-01-04 21:59 CST
SENT (0.0026s) TCP 110.1.1.2:1111 > 110.1.1.3:2222 S ttl=2 id=56409 iplen=40  seq=3638107171 win=1480 
RCVD (0.0028s) TCP 110.1.1.3:2222 > 110.1.1.2:1111 RA ttl=64 id=0 iplen=40  seq=0 win=0 
 
Max rtt: 0.144ms | Min rtt: 0.144ms | Avg rtt: 0.144ms
Raw packets sent: 1 (40B) | Rcvd: 1 (40B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 1.00 seconds
zx@zx:~$ 
zx@zx:~$ sudo ip netns exec sns1 nping --udp -c 1 --ttl 2  -g 1111 -p 2222   110.1.1.3
Starting Nping 0.7.80 ( https://nmap.org/nping ) at 2022-01-04 21:59 CST
SENT (0.0034s) UDP 110.1.1.2:1111 > 110.1.1.3:2222 ttl=2 id=50892 iplen=28 
RCVD (0.0036s) ICMP [110.1.1.3 > 110.1.1.2 Port unreachable (type=3/code=3) ] IP [ttl=64 id=16871 iplen=56 ]
 
Max rtt: 0.105ms | Min rtt: 0.105ms | Avg rtt: 0.105ms
Raw packets sent: 1 (28B) | Rcvd: 1 (56B) | Lost: 0 (0.00%)
Nping done: 1 IP address pinged in 1.00 seconds
zx@zx:~$ 


```



<font>Q为何UDP的答复这么诡异？</font>



