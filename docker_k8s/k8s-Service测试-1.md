【k8s Service测试】服务IP和后端IP，看看禁止了哪一个

**Part 1**

创建service.yaml

```yaml
apiVersion: v1
kind: Service
metadata:
  name: myservice
spec:
  ports:
    - protocol: TCP
      port: 80
      targetPort: 80
  selector:
      role: mysvc
  type: NodePort
```

执行

```shell
kubectl create -f service.yaml
```

查询

```shell
asn@asn-m:~/test$ kubectl get svc
NAME         TYPE        CLUSTER-IP      EXTERNAL-IP   PORT(S)   AGE
kubernetes   ClusterIP   10.96.0.1       <none>        443/TCP   30h
myservice    ClusterIP   10.109.246.63   <none>        80/TCP    1s
```

service`myservice`创建成功。

这时`kubectl get ep`还观察不到service的endpoint。service通过selector的标签来选择对应的后端pod，因此接下来我们创建pod并赋予`role=mysvc`标签。

```shell
asn@asn-m:~/test$ kubectl run backend --image=nginx --port=80
pod/backend created
asn@asn-m:~/test$ kubectl label pod backend role=mysvc
pod/backend labeled
asn@asn-m:~/test$ kubectl get ep
NAME         ENDPOINTS         AGE
kubernetes   100.2.1.11:6443   30h
myservice    33.0.1.27:80      2m17s
```

pod挂载在节点asn-n0上，这时在节点asn-n0执行

```shell
curl 10.109.246.63:80
curl 33.0.1.27:80
```

**均能进行访问。**



**Part 2**

创建一个客户端pod

```shell
kubectl run client --image=nginx --port=80
asn@asn-m:~/test$ kubectl get pods -o wide
NAME      READY   STATUS    RESTARTS   AGE     IP          NODE     NOMINATED NODE   READINESS GATES
backend   1/1     Running   0          8m9s    33.0.1.27   asn-n0   <none>           <none>
client    1/1     Running   0          5m11s   33.0.1.28   asn-n0   <none>           <none>
```

进入客户端pod，然后对service以及backend pod进行访问。

```shell
asn@asn-m:~/test$ kubectl exec -it client bash
kubectl exec [POD] [COMMAND] is DEPRECATED and will be removed in a future version. Use kubectl exec [POD] -- [COMMAND] instead.
root@client:/# curl 10.109.246.63:80
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
root@client:/# curl 33.0.1.27:80
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
```

由输出可知，**在client pod的内部均能对service以及backend pod进行访问。**



**Part 3**

现在，我们对client pod下发一个k8spolicy，ipblock字段设置为`10.109.246.63/32`，即myservice的IP地址。

先给client pod赋予标签

```shell
kubectl label pod client role=client

asn@asn-m:~/test$ kubectl get pods --show-labels
NAME      READY   STATUS    RESTARTS   AGE   LABELS
backend   1/1     Running   0          21m   role=mysvc,run=backend
client    1/1     Running   0          18m   role=client,run=client
```

创建client_policy.yaml

```yaml
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: client-policy
  namespace: default
spec:
  podSelector: 
    matchLabels:
      role: client
  policyTypes:
    - Ingress
    - Egress
  ingress:
    - from:
      - ipBlock:
          cidr: 10.109.246.63/32
      ports:
        - port: 80
  egress:
    - to:
      - ipBlock:
          cidr: 10.109.246.63/32
      ports:
        - port: 80
```

执行

```shell
asn@asn-m:~/test$ kubectl apply -f client_policy.yaml 
networkpolicy.networking.k8s.io/client-policy created
asn@asn-m:~/test$ kubectl get networkpolicy
NAME            POD-SELECTOR   AGE
client-policy   role=client    5s
```

接下来进入到client pod中

```shell
asn@asn-m:~/test$ kubectl apply -f client_policy.yaml 
networkpolicy.networking.k8s.io/client-policy created
asn@asn-m:~/test$ kubectl exec -it client bash
kubectl exec [POD] [COMMAND] is DEPRECATED and will be removed in a future version. Use kubectl exec [POD] -- [COMMAND] instead.
root@client:/# curl 10.109.246.63:80
curl: (7) Failed to connect to 10.109.246.63 port 80: Connection refused
root@client:/# curl 33.0.1.27:80
curl: (7) Failed to connect to 33.0.1.27 port 80: Connection refused
root@client:/# 
```

**输出结果发现，backend pod和myservice均不能访问。**



**Part 4**

现在将规则中的ipBlock修改为`33.0.1.27/32`，即backend pod的IP地址。

创建client_policy02.yaml

```yaml
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: client-policy
  namespace: default
spec:
  podSelector: 
    matchLabels:
      role: client
  policyTypes:
    - Ingress
    - Egress
  ingress:
    - from:
      - ipBlock:
          cidr: 33.0.1.27/32
      ports:
        - port: 80
  egress:
    - to:
      - ipBlock:
          cidr: 33.0.1.27/32
      ports:
        - port: 80
```

同样下发规则并进入到client pod，访问输出

```shell
asn@asn-m:~/test$ kubectl apply -f client_policy02.yaml 
networkpolicy.networking.k8s.io/client-policy created
asn@asn-m:~/test$ kubectl exec -it client bash
kubectl exec [POD] [COMMAND] is DEPRECATED and will be removed in a future version. Use kubectl exec [POD] -- [COMMAND] instead.
root@client:/# curl 33.0.1.27:80
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
root@client:/# curl 10.109.246.63:80
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
```

**backend pod以及myservice均可访问。**



【测试环境】service，backend pod， client pod

对client pod下发策略，ipBlock设置为**serviceIP**。client pod内部对service以及backend pod**均不能访问**。

对client pod下发策略，ipBlock设置为**backend pod IP**。client pod内部对service以及backend pod**均能访问**。

总结：说明kube-router是先进行loadbalance再进行防火墙。



-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

测试了如果有两个backend pod，下发策略时，如果ipBlock设置为backend1 pod IP，那么client pod只能访问backend1，service以及backend2均不能访问。如果ipBlock设置为service IP，仍然都不能访问。
