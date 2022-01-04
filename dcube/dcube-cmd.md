# cubectl

## ddos-mitigator

​    **ddos-mitigator** DDoS-mitigator Cube Subcommands
​            **rule**        DDoS-mitigator Cube Rule Subcommands
​                    **append**       Append a ddos-mitigator rule
​                            **l3**          Append a ddos-mitigator l3 rule
​                                    -c, --cube string    cube name
​                                          --srcip string   rule source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                            **l4**          Append a ddos-mitigator l4 rule
​                                    -c, --cube string       cube name
​                                          --dstport string    rule destination port
​                                          --proto string      rule protocol < icmp | tcp | udp >
​                                          --srcport string    rule source port
​                                          --tcpflags string   rule tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')
​                    **batch-append** Batch append ddos-mitigator rules
​                                    -c, --cube string   cube name
​                                    -f, --file string   configure file (json)

​                    **delete**       Delete a ddos-mitigator rule
​                            **l3**          Delete a ddos-mitigator l3 rule
​                                    -c, --cube string    cube name
​                                         --srcip string   rule source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                            **l4**          Delete a ddos-mitigator l4 rule
​                                    -c, --cube string       cube name
​                                          --dstport string    rule destination port
​                                          --proto string      rule protocol < icmp | tcp | udp >
​                                          --srcport string    rule source port
​                                          --tcpflags string   rule tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')
​                    **flush**        Flush all ddos-mitigator rules
​                            -c, --cube string   cube name
​                            -t, --type string   rule type < l3 | l4 | all >

​            **set**         DDoS-mitigator Cube Set Subcommands
​                    **ratelimit**   Set the ratelimit parameters of the ddos-mitigator cube
​                            **mode**        Set the ratelimit mode of the ddos-mitigator cube
​                                    -c, --cube string   cube name
​                                    -m, --mode string   rate-limit mode < disable | pps | bps >
​                            **value**       Set the ratelimit value of the ddos-mitigator cube
​                                    -c, --cube string    cube name
​                                    -p, --proto string   rate-limit protocol < icmp | tcp | udp >
​                                    -v, --value uint     rate-limit value of the protocol (if the value is 0, packets of this protocol are not limited, *不限速*)
​                    **workmode**    Set the workmode of the ddos-mitigator cube
​                            -c, --cube string   cube name
​                            -m, --mode string   work mode < normal | black-hole >  (default "normal")

​            **show**        DDoS-mitigator Cube Show Subcommands
​                    **rule**        Display all rules of the ddos-mitigator cube
​                            -c, --cube string   cube name
​                            -t, --type string   rule type < l3 | l4 | all >



## firewall

​    **firewall**       Firewall Cube Subcommands

​            **rule**        Firewall Cube Rule Subcommands

​                    **append**       Append a firewall rule
​                            -a, --action string      rule match action < accept | deny > (only for normal work mode)
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >
​                                  --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --dstport string     destination port
​                                  --proto string       protocol < icmp | tcp | udp >
​                                  --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --srcport string     source port
​                                  --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')

​                    **batch-append** Batch append firewall rules
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >
​                                  --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --dstport string     destination port
​                            -f, --file string        batch configure json file name
​                                  --proto string       protocol < icmp | tcp | udp >
​                                  --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --srcport string     source port
​                                  --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')

​                    **delete**       Delete a firewall rule
​                            **id**          Delete the specified ordinal number rule
​                                    -c, --cube string        cube name
​                                          --direction string   rule direction < ingress | egress >
​                                          --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                          --dstport string     destination port
​                                          --id uint32          rule id
​                                          --proto string       protocol < icmp | tcp | udp >
​                                          --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                          --srcport string     source port
​                                          --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >
​                                  --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --dstport string     destination port
​                                  --proto string       protocol < icmp | tcp | udp >
​                                  --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --srcport string     source port
​                                  --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')

​                    **flush**        Flush all firewall rules
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >
​                                  --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --dstport string     destination port
​                                  --proto string       protocol < icmp | tcp | udp >
​                                  --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --srcport string     source port
​                                  --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')

​                    **insert**       Insert a firewall rule
​                            -a, --action string      rule match action < accept | deny > (only for normal work mode)
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >
​                                  --dstip string       destination ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --dstport string     destination port
​                                  --proto string       protocol < icmp | tcp | udp >
​                                  --ruleid uint32      rule id
​                                  --srcip string       source ipaddr (format cidr: xxx.xxx.xxx.xxx/xx)
​                                  --srcport string     source port
​                                  --tcpflags string    tcpflags  < urg,ack,psh,rst,syn,fin,ece,cwr > (multiple flags are separated by ',')

​            **set**         Firewall Cube Set Subcommands
​                    **action**      Set firewall ==default== action (==only== for normal work mode)
​                            -a, --action string      default action < accept | deny > (only for normal work mode)
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >

​            **show**        Firewall Cube Show Subcommands
​                    **rule**        Display all rules of a firewall cube
​                            -c, --cube string        cube name
​                                  --direction string   rule direction < ingress | egress >



```shell
zx@zx-dcube1:~/works/dcube-tmp/tests$ cubectl cube create firewall --help
Create a firewall cube

Usage:
  cubectl cube create firewall CUBE_NAME [flags]

Aliases:
  firewall, fw

Flags:
  -c, --conntrack string           conntrack mode < disable | auto > (default "auto")
  -h, --help                       help for firewall
      --log-level string           trace log level < debug | info | warning | error | none >  (default "error")
      --max-conntrack-num uint32   firewall max conntrack num (limit: 655360) (default 65536)
      --max-rule-num uint32        firewall max rule num  (range: 64~16000) (default 4196)
      --statistic-enable           statistic enable if set this flag
  -w, --workmode string            work mode type < normal | whitelist | blacklist > (default "normal")
```





```shell
zx@zx-dcube1:~$ cubectl cube create firewall fw1 --workmode=normal

zx@zx-dcube1:~$ cubectl cube  show  list fw

firewall         cube list <num=1> 
------------------------------------------------------------
id: 0            name: fw1     

zx@zx-dcube1:~/works/dcube-tmp/tests$ cubectl cube  show  info fw1

------------------------------------------------------------
              transparent cube <fw1> basic info
------------------------------------------------------------
cube-type     : firewall  statistic     : false   
prog-type     : tc        hook-type     : tc-both 
log-level     : error     attach-status : none    
------------------------------------------------------------
                hook <ingress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 0   
------------------------------------------------------------
                hook <egress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 1   
------------------------------------------------------------
                firewall private info
------------------------------------------------------------
work-mode    : normal     conntrack          : auto    
max-rule-num : 4196   max-conntrack-num  : 65536
hook <ingress > : 
    default-action: accept  rule-num: 0      loop-num: 0   
hook <egress  > : 
    default-action: accept  rule-num: 0      loop-num: 0   
```



未指定时：

work-mode    		: 		normal     

conntrack         	 : 		auto    

default-action		:	 	accept



work-mode、direction、default-action、conntrack、{5}+flag、action



匹配顺序：

conntrack enable		cubectl cube create
config rules					cubectl firewall rule append/insert
default action				cubectl firewall set action

## slimfirewall

​    **slimfirewall**   SlimFirewall Cube Subcommands

​            **rule**        SlimFirewall Cube Rule Subcommands

​                    **append**       Append a slimfirewall rule
​                            -c, --cube string        cube name
​                                --direction string   rule direction < ingress | egress >
​                                --dstport string     rule destination port
​                                --ipaddr string      rule source address for ingress or destination address for egress (format cidr: xxx.xxx.xxx.xxx/xx)
​                                --proto string       rule protocol < icmp | tcp | udp >

​                    **batch-append** Batch append slimfirewall rules
​                            -c, --cube string        cube name
​                                 --direction string   rule direction < ingress | egress >
​                            -f, --file string        batch configure json file name

​                    **delete**       Delete a slimfirewall rule
​                            -c, --cube string        cube name
​                                 --direction string   rule direction < ingress | egress >
​                                 --dstport string     rule destination port
​                                 --ipaddr string      rule source address for ingress or destination address for egress (format cidr: xxx.xxx.xxx.xxx/xx)
​                                --proto string       rule protocol < icmp | tcp | udp >

​                    **flush**        Flush all slimfirewall rules
​                            -c, --cube string        cube name
​                                 --direction string   rule direction < ingress | egress >

​            **set**         SlimFirewall Cube Set Subcommands
​                    **action**      Set slimfirewall global action
​                            -a, --action string      global action < accept-all | deny-all | normal >
​                            -c, --cube string        cube name
​                                 --direction string   rule direction < ingress | egress >

​            **show**        SlimFirewall Cube Show Subcommands
​                    **rule**        Display all rules of a slimfirewall cube
​                            -c, --cube string        cube name
​                                 --direction string   rule direction < ingress | egress >



```shell
zx@zx-dcube1:~$ cubectl cube create slimfirewall --help
Create a slimfirewall cube

Usage:
  cubectl cube create slimfirewall CUBE_NAME [flags]

Aliases:
  slimfirewall, slimfw

Flags:
  -c, --conntrack string           conntrack mode < disable | auto > (default "auto")
  -h, --help                       help for slimfirewall
      --log-level string           trace log level < debug | info | warning | error | none >  (default "error")
      --max-conntrack-num uint32   slimfirewall max conntrack num (limit: 655360) (default 65536)
      --max-rule-num uint32        slimfirewall max rule num (limit: 20648) (default 8192)
      --statistic-enable           statistic enable if set this flag
  -w, --workmode string            work mode type < whitelist | blacklist > (default "whitelist")

```

```shell
zx@zx-dcube1:~$ cubectl cube create slimfirewall sfw1

zx@zx-dcube1:~$ cubectl cube show list slimfirewall

slimfirewall     cube list <num=1> 
------------------------------------------------------------
id: 0            name: sfw1    


zx@zx-dcube1:$ cubectl cube  show  info sfw1

------------------------------------------------------------
              transparent cube <sfw1> basic info
------------------------------------------------------------
cube-type     : slimfirewall  statistic     : false   
prog-type     : tc        hook-type     : tc-both 
log-level     : error     attach-status : none    
------------------------------------------------------------
                hook <ingress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 0   
------------------------------------------------------------
                hook <egress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 1   
------------------------------------------------------------
               slimfirewall private info
------------------------------------------------------------
work-mode    : whitelist   conntrack          : auto        k8s: false
max-rule-num : 8192   max-conntrack-num  : 65536
hook <ingress >  : 
    global-action  : normal  rule-num  : 0   
hook <egress  > : 
    global-action  : normal  rule-num  : 0   

```

