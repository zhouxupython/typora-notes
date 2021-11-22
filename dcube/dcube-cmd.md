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