## KSUID

http://www.voidcc.com/project/ksuid

https://www.jianshu.com/p/0a3fe7dad84c











## 使用go module导入本地包

[使用go module导入本地包](https://zhuanlan.zhihu.com/p/109828249)

### 举个例子

我们现在有文件目录结构如下：

```bash
├── p1
│   ├── go.mod
│   └── main.go
└── p2
    ├── go.mod
    └── p2.go
```

`p1/main.go`中想要导入`p2.go`中定义的函数。

`p2/go.mod`内容如下：

```go
module liwenzhou.com/q1mi/p2

go 1.14
```

`p1/main.go`中按如下方式导入

```go
import (
    "fmt"
    "liwenzhou.com/q1mi/p2"
)
func main() {
    p2.New()
    fmt.Println("main")
}
```

因为我并没有把`liwenzhou.com/q1mi/p2`这个包上传到`liwenzhou.com`这个网站，我们只是想导入本地的包，这个时候就需要用到`replace`这个指令了。

`p1/go.mod`内容如下：

```go
module github.com/q1mi/p1

go 1.14


require "liwenzhou.com/q1mi/p2" v0.0.0
replace "liwenzhou.com/q1mi/p2" => "../p2"
```

此时，我们就可以正常编译`p1`这个项目了。



### dcube依赖本地修改后的cubeutils编译

原始的dcube/go.mod 内容如下：

```go
module github.com/amianetworks/d-cube

go 1.15

require (
	github.com/amianetworks/cubeutils v0.1.3
    github.com/amianetworks/go2ebpf v0.1.6
...
)
```

现在本地修改了cubeutils，注意这个修改的代码不是编译时下载的依赖库，而是别的路径下的cubeutils的代码。

那么编译dcube时，需要如下修改 dcube/go.mod

```go
module github.com/amianetworks/d-cube

go 1.15

require (
	//github.com/amianetworks/cubeutils v0.1.3
	github.com/amianetworks/go2ebpf v0.1.6
...
)

require "github.com/amianetworks/cubeutils" v0.1.3
replace "github.com/amianetworks/cubeutils" => "../../3-cubeutils/cubeutils"
```

这个路径是相对路径，表示从dcube/go.mod的位置，可以访问到 cubeutils/go.mod

```bash
   3-cubeutils
            |
            ├── cubeutils
            │   ├── go.mod
            │   └── 
            |           
     4-attach                
            |    
            └── d-cube
                ├── go.mod
                └── 
```























