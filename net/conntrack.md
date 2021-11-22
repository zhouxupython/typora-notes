# conntrack

#### blogs

| links                                                        | comment                                                      |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [iptables nat&conntrack的特殊之处](https://yuerblog.cc/2020/01/06/iptables-natconntrack的特殊之处/) |                                                              |
| [iptables详解（1）：iptables概念](https://www.zsythink.net/archives/1199) | iptables入门系列                                             |
| [介绍Linux系统下的conntrack命令：允许您检查和修改跟踪的连接](https://www.cnblogs.com/cheyunhua/p/15194952.html) | NAT                                                          |
| [iptables conntrack](https://www.cnblogs.com/saolv/p/13096965.html) | ==NEW & ESTABLISHED==<br/>tcp、udp通信时conntrack的状态变化  |
| [(五)洞悉linux下的Netfilter&iptables：如何理解连接跟踪机制？【上】](https://www.cnblogs.com/masterpanda/p/5700498.html)<br/>[(六)洞悉linux下的Netfilter&iptables：如何理解连接跟踪机制？【中】](https://www.cnblogs.com/masterpanda/p/5700497.html)<br/>[(七)洞悉linux下的Netfilter&iptables：如何理解连接跟踪机制？【下】](https://www.cnblogs.com/masterpanda/p/5700496.html) | 图片缺失，未看                                               |
| [iptables连接跟踪ip_conntrack](https://blog.51cto.com/wushank/1264758) | ==NEW & ESTABLISHED & RELATED==<br/>tcp、udp、icmp通信时conntrack的状态变化 |
|                                                              |                                                              |
|                                                              |                                                              |
|                                                              |                                                              |





#### **Conntrack状态表（无NAT）**

连接跟踪子系统跟踪已看到的所有数据包流，运行“sudo conntrack -L”以查看其内容：

tcp 6 43184 ESTABLISHED ==src=192.168.2.5== dst=10.25.39.80 ==sport=5646== dport=443 src=10.25.39.80 ==dst=192.168.2.5== sport=443 ==dport=5646== [ASSURED] mark=0 use=1

tcp 6 26 SYN_SENT src=192.168.2.5 dst=192.168.2.10 sport=35684 dport=443 [UNREPLIED] src=192.168.2.10 dst=192.168.2.5 sport=443 dport=35684 mark=0 use=1

udp 17 29 src=192.168.8.1 dst=239.255.255.250 sport=48169 dport=1900 [UNREPLIED] src=239.255.255.250 dst=192.168.8.1 sport=1900 dport=48169 mark=0 use=1

每行显示一个连接跟踪条目。您可能会注意到，每行两次显示地址和端口号，甚至是反向的地址和端口对。这是因为每个条目两次插入到状态表中。第一个地址四元组（源地址和目标地址以及端口）是在原始方向上记录的地址，即发起方发送的地址。第二个四元组是conntrack希望在收到来自对等方的答复时看到的内容。这解决了两个问题：

如果NAT规则匹配（例如IP地址伪装），则将其记录在连接跟踪条目的答复部分中，然后可以自动将其应用于属于同一流的所有将来的数据包。

状态表中的查找将是成功的，即使它是对应用了任何形式的网络或端口地址转换的流的答复包。

原始的（第一个显示的）四元组永远不会改变：它是发起方发送的。NAT操作只会将回复（第二个）更改为四倍，因为这将是接收者看到的内容。对第一个四倍的更改将毫无意义：netfilter无法控制启动程序的状态，它只能影响数据包的接收/转发。当数据包未映射到现有条目时，conntrack可以为其添加新的状态条目。对于UDP，此操作会自动发生。对于TCP，conntrack可以配置为仅在TCP数据包设置了SYN位的情况下添加新条目。默认情况下，conntrack允许中流拾取不会对conntrack变为活动状态之前存在的流造成问题。

#### **Conntrack状态表和NAT**

如上一节所述，列出的答复元组包含NAT信息。可以过滤输出以仅显示应用了源或目标nat的条目。这样可以查看在给定流中哪种类型的NAT转换处于活动状态。“sudo conntrack -L -p tcp –src-nat”可能显示以下内容：

tcp 6 114 TIME_WAIT ==src=10.0.0.10== dst=10.8.2.12 ==sport=5536== dport=80 src=10.8.2.12 `dst=192.168.1.2 `sport=80 `dport=5536` [ASSURED]

此项显示从10.0.0.10:5536到10.8.2.12:80的连接。但是，与前面的示例不同，答复方向不是原始的反向方向：源地址已更改。目标主机（10.8.2.12）将答复数据包发送到`192.168.1.2`，而不是10.0.0.10。每当10.0.0.10发送一个数据包时，具有此条目的**路由器**将源地址替换为`192.168.1.2`。当10.8.2.12发送答复时，它将目的地更改回==10.0.0.10==。==src=10.0.0.10==是对外地址，`192.168.1.2`是内部地址

此源NAT是由于nft假装规则所致：

inet nat postrouting meta oifname "veth0" masquerade

其他类型的NAT规则，例如“dnat to”或“redirect to”，将以类似的方式显示，其回复元组的目的地不同于原始的。



#### 状态跟踪

conntrack将数据流的状态信息以Hash表的形式储存在内存中，包括五元组信息以及超时时间等。这里说的状态跟踪并非是指状态协议（如TCP）中连接状态的跟踪，而是**conntrack特有的与网络传输协议无关**的状态的跟踪。

##### TCP的状态跟踪

从TCP开始讨论的原因也是因为TCP本身是状态协议。TCP通过三次握手建立连接，分别是SYN,SYN/ACK和ACK，当完成之后连接成为ESTABLISHED状态。也就是说TCP完成ESTABLISHED的时候，C和S两端已经进行了三次交互。

对于iptables而言，每一次握手，都需要对连接过滤，如前面所说NEW和ESTABLISHED状态，分别指的是，当nf_conntrack第一次发现该连接的时候，会将其状态设置为==NEW==，当反方向也出现包的时候，即认为是==ESTABLISHED==。第1次握手与第2次握手间是NEW，第2次握手**之后**就是==ESTABLISHED==。某种角度来说，可以认为两次握手对conntrack而言就已经完成了ESTABLISHED

打开/proc/net/nf_conntrack，可以看到类似如下的内容。

先说下每行entry的意思，第1列是网络层协议名称；第2列是协议代号；第3列代表传输层协议名称；第4列是传输层协议代号，其中tcp是6，udp是17；第5列的`117`指的是TTL；第6列是TCP的状态；第7列是表示源目地址以及对应端口；第8列是表示是否收到回应包；第9列表示期望收到的回包的源目地址及端口。

ipv4  2  tcp   6 117 *SYN_SENT* src=192.168.1.5 dst=192.168.1.7 sport=1031 dport=23 [`UNREPLIED`] src=192.168.1.7 dst=192.168.1.5 sport=23 dport=1031 use=1
1
第1次握手时（SYN），TCP的状态是SYN_SENT，且有 [UNREPLIED] 标记，意味着还没有收到回包。此时，连接是NEW的状态，这点很好理解。
接着往下看

ipv4  2  tcp   6 57 *SYN_RECV* src=192.168.1.5 dst=192.168.1.7 sport=1031 dport=23 src=192.168.1.7 dst=192.168.1.5 sport=23 dport=1031 use=1
1
第2次握手时（SYN/ACK），TCP连接的状态为SYN_RECV，也就是说，kernel收到了回复（replay）， 在初次握手时候的[UNREPLIED]的tag被去除。需要注意的是，此时conntrack已经是ESTABLISHED，再往下看

ipv4  2 tcp   6 431999 *ESTABLISHED* src=192.168.1.5 dst=192.168.1.7 sport=1031 dport=23 src=192.168.1.7 dst=192.168.1.5 sport=23 dport=1031 [`ASSURED`] use=1
1
第3次握手时（ACK），TCP连接的状态变为ESTABLISHED，也就是说，Client发出了ACK的确认标记包，此时三次握手完毕，TCP连接建立。

TCP总结： 对Client而言，TCP状态与conntrack状态对比是SYN_SENT(**NEW**),SYN_RECV(**ESTABLISHED**),ESTABLISHED(**ESTABLISHED**)；

> 上面是对同一个tcp连接表项在三次握手不同阶段的表现

| tcp连接状态 | conntrack状态 |
| ----------- | ------------- |
| SYN_SENT    | UNREPLIED     |
| SYN_RECV    | 清除UNREPLIED |
| ACK         | ASSURED       |



配置iptables规则
对于Client是192.168.1.5:1031，Server是192.168.1.7:23。需求是Client可以主动连接Server，而反向主动的连接不可以。==Client==上防火墙规则如下：

```shell
iptables -A INPUT -s 192.168.1.7 -p tcp --dport 1031 --sport 23 -m conntrack --ctstate ESTABLISHED -j ACCEPT
iptables -A OUTPUT -d 192.168.1.7 -p tcp --sport 1031 --dport 23 -m conntrack --ctstate NEW,ESTABLISHED -j ACCEPT
```

第二条的new表示允许Client发sync；

第一条的establish表示允许收server的syn&ack；

第二条的establish表示允许Client发ack



##### UDP的状态跟踪

UDP是无状态的传输协议，不需要三次握手也没有SYN和ACK等各种标签和状态。虽然没有三次握手的概念，但是我们还是来看三次连接的状态记录。

第1次连接与第2次连接间是**NEW**的状态，第2次连接之后就是**ESTABLISHED**。对UDP而言，也可以认为两次握手对conntrack而言就已经完成了ESTABLISHED



ipv4  2  udp   17 20 src=192.168.1.2 dst=192.168.1.5 sport=137 dport=1025 [**UNREPLIED**] src=192.168.1.5 dst=192.168.1.2 sport=1025 dport=137 use=1
1
首先可以看到的是UDP没有SYN_SENT这种TCP特有的状态标签，但是有 [UNREPLIED] 的标识，说明这个包是初次发出的包，还没有收到回应。此时conntrack中对应的状态是NEW，接着往下看。

ipv4  2  udp   17 170 src=192.168.1.2 dst=192.168.1.5 sport=137 dport=1025 src=192.168.1.5 dst=192.168.1.2 sport=1025 dport=137 [ASSURED] use=1
1
同样还是没有标识，但是 [UNREPLIED] 变成了 [ASSURED] ，表明已经收到了回包，且连接建立完成。另外TTL变成了170，这时由于在该连接状态下，默认的TTL是180，而第一次连接时默认值是30。此时conntrack中定义的状态是ESTABLISHED。接着往下看

ipv4  2  udp   17 175 src=192.168.1.5 dst=195.22.79.2 sport=1025 dport=53 src=195.22.79.2 dst=192.168.1.5 sport=53 dport=1025 [ASSURED] use=1
1
Client发出第3次包之后，可以发现TTL变为了175，说明TTL更新了，也同时说明第二次和第三次中包的状态或者说标签是一致的，所以TTL默认值才会相同。

UDP总结： 对Client而言，UDP过程与conntrack状态对比是，第一次连接(NEW),第二次连接(ESTABLISHED)；

配置iptables规则
对于Client是192.168.1.5:1031，Server是192.168.1.7:23。需求是Client可以主动连接Server，而反向主动的连接不可以。==Client==上iptables规则如下：

```shell
iptables -A INPUT -s 192.168.1.7 -p udp --dport 1031 --sport 23 -m conntrack --ctstate ESTABLISHED -j ACCEPT
iptables -A OUTPUT -d 192.168.1.7 -p udp --sport 1031 --dport 23 -m conntrack --ctstate NEW,ESTABLISHED -j ACCEPT
```



第二条的new表示允许Client发第一个udp数据报文；

第一条的establish表示允许收server的udp报文；

第二条的establish表示允许Client发后续的udp报文



网络防火墙更关心的“进”和“出”，他有自己的考虑和规则，至于进出的包依循的是什么，他并不关心。

当连接初次出现的时候，该连接就是NEW，当出现了对应的反向连接的时候，那么该连接就是ESTABLISHED。看起来有点像UDP，是不是？

至于为什么会设计成这种模式，**可能是考虑到防火墙只是涉及进出两个方向，而两次握手已经可以代表两次的方向，也可能考虑到对UDP以及ICMP的兼容等问题**，在这就不去深入讨论了。









状态防火墙
