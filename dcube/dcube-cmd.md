# cubectl

## cube

**cube**		Cube Common Subcommands 

​			**create**      Create a cube							

​						**ddos-mitigator**		 Create a ddos-mitigator cube

​									*CUBE_NAME*

​                             --log-level string      trace log level < debug | info | warning | error | none >  (default "error")
​                             --max-rule-num uint32   max rule num per rule type (limit: 20648) (default 8192)
​                             --statistic-enable      statistic enable if set this flag

​                             -t, --type string           ebpf program type < ==xdp== | tc > (default "xdp")



​						**firewall**      				 Create a firewall cube

​									*CUBE_NAME*

​                             -c, --conntrack string           conntrack mode < disable | ==auto== > (default "auto")
​                             --log-level string           trace log level < debug | info | warning | error | none >  (default "error")
​                             --max-conntrack-num uint32   firewall max conntrack num (limit: 655360) (default 65536)
​                             --max-rule-num uint32        firewall max rule num  (range: 64~16000) (default 4196)
​                             --statistic-enable           statistic enable if set this flag
​                             -w, --workmode string            work mode type < ==normal== | whitelist | blacklist > (default "normal")



​						**forwarder**      			Create a forwarder cube

​									*CUBE_NAME*

​                             --log-level string   trace log level < debug | info | warning | error | none >  (default "error")

​                             -t, --type string        ebpf program type < ==xdp== | tc > (default "xdp")



​						**loadbalancer**   		 Create a loadbalancer cube

​									*CUBE_NAME*



​						**nat**            					Create a nat cube

​									*CUBE_NAME*



​						**packetcapture**  		Create a packetcapture cube

​									*CUBE_NAME*



​						**slimfirewall**   			Create a slimfirewall cube

​                             *CUBE_NAME*

​                             -c, --conntrack string           conntrack mode < disable | ==auto== > (default "auto")
​                             -h, --help                       help for slimfirewall
​                            --log-level string           trace log level < debug | info | warning | error | none >  (default "error")
​                            --max-conntrack-num uint32   slimfirewall max conntrack num (limit: 655360) (default 65536)
​                            --max-rule-num uint32        slimfirewall max rule num (limit: 20648) (default 8192)
​                            --statistic-enable           statistic enable if set this flag
​                             -w, --workmode string            work mode type < ==whitelist== | blacklist > (default "whitelist")



​			**destroy**     Destroy a cube

​					--force   force delete flag



​			**attach**      Attach a <font title="gray">transparent cube</font> to a <font title="blue">physical port</font>

​						*CUBE_NAME*   *IFNAME*  [flags]

​                             -l, --location string     attach location <before:cube-name | after:cube-name | fix:first | fix:last>
​                             --xdp-driver string   xdp driver <==auto== | generic | native> (only valid for xdp type cube and                    port is not attached or connected with other xdp cube)

​	                Examples:
​	                	// attach cube0 to eth0
​	                	$ cubectl cube attach cube0 eth0
​	                	// attach cube1 to eth0 and its location is after cube0 which is attached
​	                	$ cubectl cube attach cube0 eth0 --location=after:cube0	

​			**detach**      Detach a transparent cube from the physical port

​						*CUBE_NAME*



​			**connect**     Connect the <font style="background-color:#8bc34a">standard cube</font>'s <font title="yellow">virtual port</font> to a <font title="blue">physical port</font> or another <font title="yellow">virtual port</font>

​						  *CUBE_NAME:PORT_NAME*   *CUBE_NAME:PORT_NAME* | *IFNAME*   [flags]

​								  	--xdp-driver string   xdp driver  <==auto== | generic | native> (only valid for xdp type cube and port is not attached or connected with other xdp cube)

​	                Examples:

​	                	// connect port2 of fwd0 to port3 of fwd1

​	                	$ cubectl cube connect fwd0:port2   fwd1:port3

​	                	// connect port1 of fwd0 to veth1

​	                	$ cubectl cube connect fwd0:port1   VETH1



​	   	**disconnect**  Disconnect a standard cube's virtual port or a physical port from its counterpart

​			  			*CUBE_NAME:PORT_NAME*  |  *IFNAME*

​							Examples:

​                       $ cubectl cube disconnect fwd0:port2   



​       	 **port**        Port Subcmds

​       			      **add**         Add a <font title="yellow">virtual port</font> to a <font style="background-color:#8bc34a">standard cube</font>

​		          					*CUBE_NAME    PORT_NAME*

​           	 		 **delete**      Delete a virtual port of a standard cube

​          							*CUBE_NAME PORT_NAME*   [flags]

​					     		     --force   force delete flag



​    		**statistics**      Cube Statistics Subcommands, depends on whether use --statistic-enable on cube create 

​      				    **clear**       Clear statistics of a cube

​          							 *CUBE_NAME*

​      				    **show**       Show statistics of a cube

​	          						 *CUBE_NAME*



​     		**clear**       Clear all cubes of a certain type

​      		 			*CUBE_TYPE*

​						    	currently supported cube types : 

​											ddos-mitigator (dm) | firewall (fw) | slimfirewall (slimfw) | nat | forwarder (fwd)

​											loadbalancer (lb) | packetcapture(pcap) | ==all==



​      		**show**        Show Subcmds

​       		 			**info**        Display the details of a cube

​           							 *CUBE_NAME*		

​	 		  **list**        Display a list of all cubes of a certain type

​			   			 *CUBE_TYPE*
​										currently supported types : 

​													ddos-mitigator | firewall | slimfirewall | nat | forwarder | packetcapture

​										Examples:										

​										// display all firewall cubes, get the wanted fw name, then use info to get details

​										$ cubectl cube show list firewall

​										$ cubectl cube show info fw0

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



```shell
zx@zx-dcube1:~$ cubectl cube create ddos-mitigator --help
Create a ddos-mitigator cube

Usage:
  cubectl cube create ddos-mitigator CUBE_NAME [flags]

Aliases:
  ddos-mitigator, dm

Flags:
  -h, --help                  help for ddos-mitigator
      --log-level string      trace log level < debug | info | warning | error | none >  (default "error")
      --max-rule-num uint32   max rule num per rule type (limit: 20648) (default 8192)
      --statistic-enable      statistic enable if set this flag
  -t, --type string           ebpf program type < xdp | tc > (default "xdp")

```



```shell
zx@zx-dcube1:~$ cubectl cube  show  info d0

------------------------------------------------------------
              transparent cube <d0> basic info
------------------------------------------------------------
cube-type     : ddos-mitigator  statistic     : false   
prog-type     : xdp       hook-type     : xdp-in  
log-level     : error     attach-status : none    
xdp-driver    : auto    
------------------------------------------------------------
                hook <ingress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 0   
------------------------------------------------------------
ddos-mitigator private info
------------------------------------------------------------
hook <ingress > : 
    workmode       : normal  
    l3-rule-max    : 8192  l3-rule-num : 0   
    l4-rule-max    : 8192  l4-rule-num : 0   
    ratelimt-mode  : disable 
    ratelimt-value : icmp : 0  udp : 0  tcp : 0

```

```shell
zx@zx-dcube1:~$ cubectl cube create ddos-mitigator d1 --type tc 
 
zx@zx-dcube1:~$ cubectl cube  show  info d1

------------------------------------------------------------
              transparent cube <d1> basic info
------------------------------------------------------------
cube-type     : ddos-mitigator  statistic     : false   
prog-type     : tc        hook-type     : tc-in   
log-level     : error     attach-status : none    
------------------------------------------------------------
                hook <ingress> basic info
------------------------------------------------------------
link-type     : none      
prev-map      : 0     prev-prog     : 0   
next-map      : 0     next-prog     : 0   
------------------------------------------------------------
ddos-mitigator private info
------------------------------------------------------------
hook <ingress > : 
    workmode       : normal  
    l3-rule-max    : 8192  l3-rule-num : 0   
    l4-rule-max    : 8192  l4-rule-num : 0   
    ratelimt-mode  : disable 
    ratelimt-value : icmp : 0  udp : 0  tcp : 0
```

```shell
zx@zx-dcube1:~$ cubectl cube attach --help
Attach a transparent cube to a physical port

Usage:
  cubectl cube attach CUBE_NAME IFNAME [flags]

Examples:
	# attach cube0 to eth0
	$ cubectl cube attach cube0 eth0
	# attach cube1 to eth0 and its location is after cube0 which is attached
	$ cubectl cube attach cube0 eth0 --location=after:cube0
	

Flags:
  -h, --help                help for attach
  -l, --location string     attach location <before:cube-name | after:cube-name | fix:first | fix:last>
      --xdp-driver string   xdp driver <auto | generic | native> (only valid for xdp type cube and port is not attached or connected with other xdp cube)

```



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
​                            -a, --action string      default action < ==accept== | deny > (only for normal work mode)
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

work-mode    		: 		==normal==   

conntrack         	 : 		==auto==      

default-action		:	 	==accept== 



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



## forwarder

**forwarder**  	Forwarder Cube Subcommands

​		**rule**        Forwarder Cube Rule Subcommands

​					**append**      Append a forwarder rule

​						    -c, --cube string       		cube name
​						    -i, --in-port string   		  forwarder input port
​						    -o, --out-port string    	forwarder output port

​					**delete**        Delete a forwarder rule

​						    -c, --cube string       cube name
​						    -i, --in-port string    forwarder input port
​						    -o, --out-port string   forwarder output port

​					**flush**          Flush a forwarder rule

​						    -c, --cube string   cube name

​		**show**        Forwarder Cube Show Subcommands

​					**rule**        Display all rules of the forwarder cube

  						   -c, --cube string   cube name



```shell
cubectl cube create forwarder fwd0
cubectl cube create forwarder fwd1

cubectl cube port add fwd0 port0
cubectl cube port add fwd0 port1
cubectl cube port add fwd1 port2
cubectl cube port add fwd1 port3

# connect是在不同的forward中的port，或者和端口进行
cubectl cube connect fwd0:port1 VETH1
cubectl cube connect fwd1:port3 VETH2
cubectl cube connect fwd0:port2 fwd1:port4

# rule却是同一个forward中的port指定in-out规则
cubectl forwarder rule append -c fwd0 -i port1 -o port2
cubectl forwarder rule append -c fwd0 -i port2 -o port1
cubectl forwarder rule append -c fwd1 -i port3 -o port4
cubectl forwarder rule append -c fwd1 -i port4 -o port3
```

