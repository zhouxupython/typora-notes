## 1. proxy

```Go
go env -w GOPROXY=https://goproxy.cn
```

## 2. cidr-tool单元测试运行设置

zhouxu@zhouxu:~/ go get github.com/amianetworks/cidr-tool
zhouxu@zhouxu:~/works/dcube-comment/cidr-tool$ go mod init cidr-tool
zhouxu@zhouxu:~/works/dcube-comment/cidr-tool$ go mod tidy

修改go mod文件

module github.com/amianetworks/cidr-tool

go 1.16

然后根据
https://www.cnblogs.com/bbllw/p/12377155.html
settings        Go          GoModules   第一个打勾，env空着就行了

ok后，proj的external lib会出现： Go Modules有cird-tool，

当然，首先需要go get github.com/amianetworks/cidr-tool

![](image-20220304111015278-16463637155721.png)

![](image-20220304111151962-16463637451792.png)

