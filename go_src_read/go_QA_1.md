### 1. 几种类型的fd（int、NetFD、FD、pollDesc等）如何与goroutine对应

go中使用`epoll_wait`，将当前这个goroutine挂起，通过`gopark`切换到别的goroutine，那么epoll_wait返回时，拿到的是events-fd，

所以如何根据events-fd定位到对应的goroutine？

即，这个挂起的goroutine对应的fd有IO时，如何通过fd找到这个goroutine重新调度以执行？goready？

```go
// 建立链接的时候，进行了初始化
func (fd *FD) Init(net string, pollable bool) error 

//internal/poll/fd_poll_runtime.go
func (pd *pollDesc) init(fd *FD) error 
    func poll_runtime_pollOpen(fd uintptr) (*pollDesc, int)
        netpollopen(fd, pd)
            *(**pollDesc)(unsafe.Pointer(&ev.data)) = pd    //@@@@@@@@@@@@@@@@@@@@@@@

```

```go
//runtime/proc.go
func schedule() {
    var gp *g
    gp, inheritTime = findrunnable() // blocks until work is available 调度器会重新调度选择一个goroutine去运行；
    
    execute(gp, inheritTime)

}

//一开始的疑问就是，为何在steal之前就执行netpoll，不是堵塞在这儿干等着吗，这样就算有别的goroutine需要执行，不也无法执行吗？
//其实是调用的非阻塞netpoll，不管有没有IO事件都会返回，有IO事件就执行对应的goroutine，没有就去steal
//steal不到，那么就调用堵塞的netpoll，等着IO事件的发生
//https://studygolang.com/articles/7852
func findrunnable() (gp *g, inheritTime bool) {}
		list := netpoll(0)  // non-blocking 非阻塞调用，直接返回

		list := netpoll(delta) // block until new work is available
		gp := list.pop()	// 从待执行的goroutine list中取一个
		return gp, false
}

//runtime/netpoll_epoll.go
func netpoll(delay int64) gList {
    n := epollwait(epfd, &events[0], int32(len(events)), waitms)
	var toRun gList				// 有IO的goroutine添加到这个list中，待后续调度
    for i := int32(0); i < n; i++ {
        ev := &events[i]
        pd := *(**pollDesc)(unsafe.Pointer(&ev.data))    //@@@@@@@@@@@@@@@@@@@@@@@
        netpollready(&toRun, pd, mode)
            	rg = netpollunblock(pd, 'r', true)
                		gpp := &pd.rg
        		toRun.push(rg)
        
    return toRun
 }
    
func execute(gp *g, inheritTime bool) {
	gogo(&gp.sched) // runtime/asm_amd64.s	gopark==>mcall 中 保存当前goroutine的状态(PC/SP)到g->sched中，下次调度回来时使用；
}

gp.sched：
type gobuf struct {
	// The offsets of sp, pc, and g are known to (hard-coded in) libmach.
	sp   uintptr
	pc   uintptr
	g    guintptr
	ctxt unsafe.Pointer
	ret  sys.Uintreg
	lr   uintptr
	bp   uintptr // for framepointer-enabled architectures
}
```



---



### 2. pd.rg何时赋值的？

上面的gpp := &pd.rg，pd.rg何时赋值的？

```go
```



### 3. Read函数如何与上面的流程串联起来？

```go
// Read堵塞，使用gopark将对应的goroutine切换出去，findrunnable选择一个新的goroutine执行，
// 如果local runq, global runq没有找到的话，就进入Poll network，等待观察的fd有IO事件到来.
// 即进入netpoll的流程，通过epollwait等待有IO发生，选一个IO对应的goroutine进行调度，
// Read有数据时，对应的goroutine最终将会被调度到，从切换出去的位置返回，继续执行
func (c *conn) Read(b []byte) (int, error) 	// 堵塞
    n, err := c.fd.Read(b) ==> func (fd *netFD) Read(p []byte) (n int, err error)
        n, err = fd.pfd.Read(p) ==> func (fd *FD) Read(p []byte) (int, error) 
            for {
                    if err = fd.pd.waitRead(fd.isFile); ==> func (pd *pollDesc) waitRead(isFile bool) error 
                        pd.wait('r', isFile) ==> func (pd *pollDesc) wait(mode int, isFile bool) error  // internal/poll/fd_poll_runtime.go
                            res := runtime_pollWait(pd.runtimeCtx, mode) ==> func poll_runtime_pollWait(pd *pollDesc, mode int) int     // runtime/netpoll.go
                                netpollblock(pd, int32(mode), false) ==> func netpollblock(pd *pollDesc, mode int32, waitio bool) bool
                                    gopark(netpollblockcommit, unsafe.Pointer(gpp), waitReasonIOWait, traceEvGoBlockNet, 5)     // runtime/proc.go
                                        mcall(park_m) // 在g0的堆栈上执行park_m
                                            func park_m(gp *g) // park continuation on g0.  gp 就是当前调用Read被堵塞的goroutine
                                                schedule()	// Q-1 调度器会重新调度选择一个goroutine去运行；
                									findrunnable() // blocks until work is available
                										list := netpoll(delta) // block until new work is available
                											n := epollwait(......) // 最终堵塞的地方	Q-1		
                											根据epollwait的返回，将有IO的goroutine加入list
                										toRun < --- list
                										gp := list.pop()	// 从待执行的goroutine list中取一个
                									execute(gp, inheritTime) // 执行这个goroutine
                									//类似与c，再次调度即有Read数据时，会从此处返回
                
```



---

### 4. findrunnable中使用的netpoll的疑问

([go runtime scheduler](https://studygolang.com/articles/7852)):    *该文章附带资料较多*

schedule()函数首先调用runqget()从当前*P*的队列中取一个可以执行的*G*。 

如果队列为空，继续调用findrunnable()函数。findrunnable()函数会按照以下顺序来取得*G*：

1. 调用runqget()从当前*P*的队列中取*G*（和schedule()中的调用相同）；
2. 调用globrunqget()从全局队列中取可执行的*G*；
3. 调用netpoll()取异步调用结束的*G*，该次调用为**非阻塞调用**，直接返回；
4. 调用runqsteal()从其他*P*的队列中“偷”。

 如果以上四步都没能获取成功，就继续执行一些低优先级的工作：

1. 如果处于垃圾回收标记阶段，就进行垃圾回收的标记工作；
2. 再次调用globrunqget()从全局队列中取可执行的*G*；
3. 再次调用netpoll()取异步调用结束的*G*，该次调用为**阻塞调用**。

```go
//一开始的疑问就是，为何在steal之前就执行netpoll，不是堵塞在这儿干等着吗，这样就算有别的goroutine需要执行，不也无法执行吗？
//其实是调用的非阻塞netpoll，不管有没有IO事件都会返回，有IO事件就执行对应的goroutine，没有就去steal
//steal不到，那么就调用堵塞的netpoll，等着IO事件的发生
//https://studygolang.com/articles/7852
func findrunnable() (gp *g, inheritTime bool) {}
		list := netpoll(0)  // non-blocking 非阻塞调用，直接返回

		list := netpoll(delta) // block until new work is available
		gp := list.pop()	// 从待执行的goroutine list中取一个
		return gp, false
}
```



### 5. 连续栈

 （[Golang连续栈](https://docs.google.com/document/d/1wAaf1rYoM4S4gtnPh0zOlGzWtrZFQ5suE8qr2sD8uWQ/pub)） 

### 6. 协程切换原理---细节

[一文教你搞懂 Go 中栈操作](https://www.cnblogs.com/luozhiyun/p/14619585.html)

[详解Go语言调度循环源码实现](https://www.cnblogs.com/luozhiyun/p/14426737.html)

([协程切换原理](https://zhuanlan.zhihu.com/p/29887309?ivk_sa=1024320u))

([Golang-gopark函数和goready函数原理分析](https://blog.csdn.net/u010853261/article/details/85887948))

([golang并发原理剖析](https://blog.csdn.net/zhongcanw/article/details/89948984))

([[转]Golang中goroutine的调度器详解](https://blog.csdn.net/heiyeshuwu/article/details/51178268))



```go
runtime·mcall
get_tls(CX)
gobuf_xxx如何定义
```

