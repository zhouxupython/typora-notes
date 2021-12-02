# 1

## 循环

循环时，使用$符号取得循环的idx值

```shell
#!/bin/bash
for port in {20000..20010}
do
    ./asnsncli firewall insert --interface data --direction IN --protocol tcp --port $port --mask=0 --action ALLOW
done
```

```shell
#!/bin/bash
for a in {1..10}
do
        mkdir /datas/aaa$a
        cd /datas/aaa$a
        for b in {1..10}
        do
                mkdir bbb$b
        done
done
```

