## firewall

1. batch-append 为何只有一个 -f 不就可以了？其余的都在json里面配置？

2. delete + id 为何还有这么多选项，只有一个id不就行了？

3. 在规则集序号为0的位置插入规则   这一条没有  -a, --action string   正确吗？

    cubectl firewall rule insert -c <CUBE_NAME> --direction egress --proto icmp --ruleid=0

4. 展示防火墙的统计信息    ？？？ 有这个命令吗

    cubectl firewall statistics show -c fw1

5. 一个cube不能attach到多个口上吗？

6. ​    \# repeat to rule append 为啥？

    ​    exec_cmd "cubectl firewall rule append -c fw7 --direction ingress -a deny --srcip 110.1.1.2 --dstip 110.1.1.3/24"

    ​    exec_err_cmd "cubectl firewall rule append -c fw7 --direction ingress -a deny --srcip 110.1.1.2/32 --dstip 110.1.1.8/24"

7. 

8. 

9. 

10. 

11. 

      







2. 黑白名单指定后，后续如何进一步配置，还是到此为止？

    黑名单：后续添加上的策略，都是需要阻止的，其余放行

    白名单：后续添加上的策略，才是需要放行的，其余阻止

3. eno1， ens33  这种口的意义？

4. type veth peer name  ?

5. namespace是什么？  ip netns add。。。

6. 这个与==iptables conntrack”连接追踪”==的对比

7.  

8.  

9.  

10. 







