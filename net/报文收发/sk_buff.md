[sk_buff封装和解封装网络数据包的过程详解](https://blog.csdn.net/Sophisticated_/article/details/84878085)

[原创                   Linux-4.20.8内核桥收包源码解析（一）----------sk_buff（详细）                       ](https://blog.csdn.net/Sophisticated_/article/details/87453506)

[原创                   Linux-4.20.8内核桥收包源码解析（二）----------sk_buff的操作 ](https://blog.csdn.net/Sophisticated_/article/details/87625495)

[原创                   sk_buff详解                       ](https://blog.csdn.net/hzj_001/article/details/100621914)[合集]

[套接字缓存之sk_buff结构](https://www.cnblogs.com/wanpengcoder/p/7529486.html)

[skb的分配以及释放](https://www.cnblogs.com/codestack/p/14292068.html)

[Difference between skbuff frags and frag_list](https://www.cnblogs.com/codestack/p/13567025.html)

[sk_buff结构--转载](https://www.cnblogs.com/codestack/p/11853392.html)

https://xingkunz.github.io/tags/网络

https://jiyang.site/posts/2020-01-02-理解linux内部网络实现之关键数据结构之-sk_buff/





sk_buff
https://blog.csdn.net/Sophisticated_/article/details/87453506
https://blog.csdn.net/Sophisticated_/article/details/87625495
https://blog.csdn.net/Sophisticated_/article/details/84878085

https://www.cnblogs.com/tzh36/p/5424564.html
https://blog.csdn.net/hzj_001/article/details/100621914
https://liu-jianhao.github.io/2019/05/linux%E5%86%85%E6%A0%B8%E7%BD%91%E7%BB%9C%E6%BA%90%E7%A0%81%E8%A7%A3%E6%9E%901sk_buff%E7%BB%93%E6%9E%84/
https://blog.csdn.net/YuZhiHui_No1/article/details/38666589
https://blog.csdn.net/denglin12315/article/details/115974066    常见的二层协议和三层协议结构以及IP头checksum计算方法
https://blog.csdn.net/ythunder/article/details/65664309     TCP/IP报文头部结构整理
https://www.cnblogs.com/sunnypoem/p/12491036.html   TCP/IP校验和
https://blog.csdn.net/yiluyangguang1234/article/details/78026020    IP首部校验和计算原理
https://bbs.csdn.net/topics/390365436   IP“首部检验和”的用途和用法
https://zhuanlan.zhihu.com/p/362826470  TCP/IP/ICMP 报文格式
https://www.cnblogs.com/mengwang024/p/4442370.html  《TCP/IP详解卷1：协议》第6章 ICMP：Internet控制报文协议-读书笔记
https://zhuanlan.zhihu.com/p/369623317  深入理解ICMP协议



| [Linux内核网络源码解析1——sk_buff结构](https://liu-jianhao.github.io/2019/05/linux内核网络源码解析1sk_buff结构/) |         |      |
| ------------------------------------------------------------ | ------- | ---- |
| [Linux内核网络源码解析2——sk_buff操作](https://liu-jianhao.github.io/2019/05/linux内核网络源码解析2sk_buff操作/) |         |      |
| [linux内核网络sk_buff结构详解](https://blog.csdn.net/yuzhihui_no1/category_9262940.html) | 5篇都读 |      |
|                                                              |         |      |
|                                                              |         |      |
|                                                              |         |      |
|                                                              |         |      |
|                                                              |         |      |
|                                                              |         |      |
|                                                              |         |      |



## sk_buff

![](D:\10000_works\zzztmp\截图\sk_buff_2.6.20.png)

![sk_buff_2.6.20_data_ptr](D:\10000_works\zzztmp\截图\sk_buff_2.6.20_data_ptr.png)



```c
/* 分配sk_buff, 图中padding是为了对齐而填充的区域，填充区域上面区域的大小为size
size，待分配sk_buff的线性存储区的长度
gfp_mask，分配内存的方式
fclone，预测是否会克隆，用于确定从哪个高速缓存中分配
node，当支持NUMA(非均匀质存储结构)时，用于确定何种区域中分配sk_buff
*/
struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
			    int fclone, int node)
```

![](D:\10000_works\zzztmp\截图\alloc_skb.png)

```c
/*
释放sk_buff
只在 skb->users 为1的情况下才释放内存，否则只是简单的递减
*/
void kfree_skb(struct sk_buff *skb)
```

------

## 四个常用函数

==skb分配好之后，head、end两个指针不变了，下面的操作都是data、tail两个指针在变化，表示user data区间的变化==

```c
/**
 *	skb_put - add data to a buffer	在buffer结尾处扩展
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned. 返回的是扩展之前的tail位置，即增加的区域的起始位置
 
 开始存储数据时，通过调用函数skb_put()来使tail指针向下移动空出空间来添加数据，此时skb->data和skb->tail之间存放的都是数据信息，无协议信息。
 */
void *skb_put(struct sk_buff *skb, unsigned int len)
{
	void *tmp = skb_tail_pointer(skb);
	SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len  += len;
	if (unlikely(skb->tail > skb->end))
		skb_over_panic(skb, len, __builtin_return_address(0));
	return tmp;
}

----------

/**
 *	skb_push - add data to the start of a buffer	在buffer开头处扩展
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned. 返回的是扩展后的data位置，也算是扩展进来的区域的起始位置
 */
void *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
	if (unlikely(skb->data < skb->head))
		skb_under_panic(skb, len, __builtin_return_address(0));
	return skb->data;
}

-----------------

/**
 *	skb_pull - remove data from the start of a buffer	从起始处减少buffer，扩大headroom
 *	@skb: buffer to use
 *	@len: amount of data to remove						返回的是缩小后的data位置
 *
 *	This function removes data from the start of a buffer, returning
 *	the memory to the headroom. A pointer to the next data in the buffer
 *	is returned. Once the data has been pulled future pushes will overwrite
 *	the old data.  										因为pull而缩小data区域，将来再次push时，数据会被覆盖
 */
void *skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull_inline(skb, len);
}
static inline void *skb_pull_inline(struct sk_buff *skb, unsigned int len)
{
	return unlikely(len > skb->len) ? NULL : __skb_pull(skb, len);
}
static inline void *__skb_pull(struct sk_buff *skb, unsigned int len)
{
	skb->len -= len;
	BUG_ON(skb->len < skb->data_len);
	return skb->data += len;
}

---------------
/**
 *	skb_reserve - adjust headroom
 *	@skb: buffer to alter
 *	@len: bytes to move
 *
 *	Increase the headroom of an empty &sk_buff by reducing the tail
 *	room. This is only allowed for an empty buffer.
 开始准备存储应用层下发过来的数据，通过调用函数 skb_reserve()来使data指针和tail指针同时向下移动，空出一部分空间来为后期添加协议信息。
 */
static inline void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
}
```

![](D:\10000_works\zzztmp\截图\sk_buff_2.6.20_put_push_pull_reserve.png)

**reserve**：

开始准备存储应用层下发过来的数据，通过调用函数 skb_reserve()来使data指针和tail指针同时向下移动，空出一部分空间来为后期添加==协议==信息。

**put**：

开始存储数据时，通过调用函数skb_put()来使tail指针向下移动空出空间来添加数据，此时skb->data和skb->tail之间存放的都是==数据==信息，无协议信息。

**push**、**pull**处理各层协议头的：

push是高层协议到低层协议，通常用于发送的数据包后在各层由上往下传递时，添加下层的协议首部；这时就开始调用函数skb_push()来使data指针向上移动，空出空间来添加各层==协议==信息。直到最后到达二层，添加完帧头然后就开始发包了。

pull是低层协议到高层协议，通常用于接收的数据包后在各层由下往上传递时，上层忽略下层的==协议==信息。

![](D:\10000_works\zzztmp\截图\sk_buff在各层协议之间传输.png)

------



```c
/*
拷贝sk_buff及其数据
修改的数据在skb->head和skb->end之间，可使用pskb_copy()来复制这部分数据
如果同时需要修改聚合分散IO存储区中的数据，就必须使用skb_copy()
*/
pskb_copy
skb_copy
```

![](D:\10000_works\zzztmp\截图\pskb_copy() & skb_copy().png)

------

## skb_add_data

```c
/*
添加尾部数据
将指定用户空间的数据添加到skb_buff的数据缓存区的尾部
*/
// skb为被添加的sk_buff类型的结构体，from为将要添加的数据源，copy为数据源的长度
static inline int skb_add_data(struct sk_buff *skb,
			       char __user *from, int copy)
{
	const int off = skb->len;
 	// skb_put返回的是 增加的区域的起始位置，这是拷贝目的地
	if (skb->ip_summed == CHECKSUM_NONE) {// 表示检验ip包的校验
		int err = 0;
// 数据拷贝操作，这里调用了skb_put()函数让tail往下移空出控件来存放将要拷贝的数据，并且返回tail指针
		__wsum csum = csum_and_copy_from_user(from, skb_put(skb, copy), 
							    copy, 0, &err); 
		if (!err) {
			skb->csum = csum_block_add(skb->csum, csum, off); // 这个应该是IP校验计算吧
			return 0;
		}
	} else if (!copy_from_user(skb_put(skb, copy), from, copy)) // 这是最本质的数据拷贝操作宏，同样调用了skb_put()函数返回tail指针
		return 0;
 
	__skb_trim(skb, off); // 这个是删除数据操作，将在下一个数据删除（skb_trim()函数）分析
	return -EFAULT;
}
 
static inline
__wsum csum_and_copy_from_user (const void __user *src, void *dst,
				      int len, __wsum sum, int *err_ptr)
{
	if (access_ok(VERIFY_READ, src, len)) // 判断数据长度关系
		return csum_partial_copy_from_user(src, dst, len, sum, err_ptr); // 调用拷贝函数
 
	if (len)
		*err_ptr = -EFAULT;
	return sum;
}
 
static __inline__
__wsum csum_partial_copy_from_user(const void __user *src,
                                         void *dst, int len, __wsum sum,
                                         int *err_ptr)
{
        if (copy_from_user(dst, src, len)) { // 拷贝操作
                *err_ptr = -EFAULT;
                return (__force __wsum)-1;
        }
        return csum_partial(dst, len, sum); // 设置校验和
}
 
// 这是调用memcpy()函数来对数据进行拷贝，to是tail指针，from是将要插入的数据源指针，n是数据源长度
#define copy_from_user(to, from, n)	(memcpy((to), (from), (n)), 0) 
```

![](D:\10000_works\zzztmp\截图\skb_add_data.png)

------

## skb_trim

```c
/*
删除尾部数据：skb_trim()
与skb_add_data()操作相反
*/
// 这里值得注意的是len不是要删除的数据长度，而是删除后的数据长度，即是新的数据长度。
// 所以新的数据长度不能比开始的skb的长度还大，否则就是插入增加数据函数而不是删除数据函数了
void skb_trim(struct sk_buff *skb, unsigned int len)
{	
	if (skb->len > len) 
		__skb_trim(skb, len);// 调用函数进行删除数据操作
}
 
static inline void __skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (unlikely(skb->data_len)) {
		WARN_ON(1);
		return;
	}
	skb->len = len; // 为新的skb赋上删除后的len值
	skb_set_tail_pointer(skb, len); // 调用函数删除操作
}
 
static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset; // 实质上没有对数据进行删除，只是让tail指针偏移，改变有效数据值的范围
}
```

![](D:\10000_works\zzztmp\截图\skb_trim.png)

------

## pskb_trim

删除sk_buff结构中分片结构的数据区数据函数：

pskb_trim()函数其实包含了skb_trim()函数，如果当分片结构数据区没有数据则skb_trim()函数和pskb_trim()函数是一样的。

如果分片结构数据区有数据时，则pskb_trim()函数不仅要删除sk_buff结构数据区数据（skb_trim()函数功能），还要删除分片结构数据区数据。

```c
static inline int pskb_trim(struct sk_buff *skb, unsigned int len)
{
	return (len < skb->len) ? __pskb_trim(skb, len) : 0; // 这个功能和上面类似，如果新len值小于skb原有的值，则做删除操作
}
static inline int __pskb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->data_len)// 如果分片结构数据区有数据
		return ___pskb_trim(skb, len);// 则调用该函数来删除分片结构中的数据区数据，三个下划线开头，不是递归调用
	__skb_trim(skb, len);// 这个和上面删除sk_buff结构中的数据区数据一样
	return 0;
}
```



------

## skb_split

==参数len为拆分后的skb的新长度==

```c
/*
拆分数据：skb_split()
根据指定长度拆分sk_buff，使得原sk_buff中的数据长度为指定长度，剩下的数据保存到拆分得到的sk_buff中

LEN：拆分长度；hlen：线性数据长度
（1）当拆分数据的长度小于线性数据长度时，直接拆分线性数据区即可
（2）拆分数据的长度大于线性数据长度时，则需要拆分非线性区域中的数据，拆分长度LEN大于hlen并且LEN小于hlen+S1
*/
// skb为原来的skb结构体（将要被拆分的），skb1为拆分后得到的子skb，len为拆分后的skb的新长度
void skb_split(struct sk_buff *skb, struct sk_buff *skb1, const u32 len)
{
	int pos = skb_headlen(skb);// pos = skb->len - skb->data_len，pos是skb数据区长度
 
	if (len < pos)	// （1）如果拆分长度小于skb数据区中的有效长度，则调用下面函数
		skb_split_inside_header(skb, skb1, len, pos);// 该函数只拆分skb数据区中的数据
	else  // （2）反之，如果拆分长度不小于skb数据区中的有效长度，则调用下面函数
		skb_split_no_header(skb, skb1, len, pos);// 拆分skb结构中的分片结构中数据区数据
}
```

（1）当拆分数据的长度小于线性数据长度时，直接拆分线性数据区即可

```c
// 这是只拆分sk_buff结构数据区的数据，其他参数不变，参数：pos则是sk_buff结构数据区中有效数据长度
static inline void skb_split_inside_header(struct sk_buff *skb,
					   struct sk_buff* skb1,
					   const u32 len, const int pos)//len为拆分后的skb的新长度;pos是skb数据区长度
{
	int i;
	// pos - len 是转移到skb1的数据长度，skb1通过skb_put()来使skb1->tail向下移动pos - len来接收数据。
    // memcpy(skb_put返回值, skb->data + len, pos - len); 
    // skb->data的前len字节数据自己仍然保留，余下的pos - len字节，拷贝到skb1->tail开始的位置
	skb_copy_from_linear_data_offset(skb, len, skb_put(skb1, pos - len), pos - len);
    
    // nr_frags为多少个分片数据区，循环把所有分片数据拷贝到skb1中
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
		skb_shinfo(skb1)->frags[i] = skb_shinfo(skb)->frags[i];// skb_frag_t结构体的赋值，让skb1中的分片结构关联到分片数据区。
	
	//下面做的都是些成员字段拷贝赋值操作，并且设置skb的字段
	skb_shinfo(skb1)->nr_frags = skb_shinfo(skb)->nr_frags;//skb1拥有所有的 分片数据区，而且其frags[]也已经全部关联到分片数据区了。
	skb_shinfo(skb)->nr_frags  = 0;// 虽然skb的分片结构没有改变，仍然关联到分片数据区，但是这里通过修改nr_frags，将skb的分片置为“无”
	skb1->data_len		   = skb->data_len;// skb1拥有所有的 分片数据区
	skb1->len		   += skb1->data_len;// skb1->len = pos - len + skb1->data_len
	skb->data_len		   = 0;// skb的分片置为“无”
	skb->len		   = len;// skb的数据长度是split后的len字节，没有分片数据区
	skb_set_tail_pointer(skb, len);// ->data不变，skb->tail = skb->data + len;
}

        /*
        // 这是个把sk_buff结构中有效数据拷贝到新的skb1中,pos为有效数据长度，len为剩下数据长度，得：pos-len为要拷贝的数据长度
        // skb_put(skb1,pos-len)是移动tail指针让skb1结构数据区空出空间来存放将要拷贝的数据，该函数返回tail指针
        skb_copy_from_linear_data_offset(skb, len, skb_put(skb1, pos - len),
                         pos - len);
					 
		为了方便理解，把该函数实现代码注释进来
        skb为要被拆分的sk_buff结构，offset为剩下新的skb数据长度，to为skb1结构中tail指针，len为要拷贝的数据长度
        static inline void skb_copy_from_linear_data_offset(const struct sk_buff *skb,
	    			    const int offset, void *to,
	    			    const unsigned int len)
	    {
			// 从skb要剩下的数据位置开始（即是skb->data+offset，skb->data和skb->data+offset之间的数据是要保留的）
            // to则是tail指针移动前返回的一个位置指针（详细请看skb_put()函数实现），拷贝len长度内容
			memcpy(to, skb->data + offset, len);
	    }        
		*/

 		//	static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
		//	{
		//		// 这是把tail指针移到数据区的最后面
		//		skb->tail = skb->data + offset;	
		//	}
```



![](D:\10000_works\zzztmp\截图\skb_split-直接拆分.jpg) 



（2）拆分数据的长度大于线性数据长度时，则需要拆分非线性区域中的数据，

例如：拆分长度LEN大于hlen并且LEN小于（hlen + S1），所以需要从S1中取一部分补给skb，S1剩余 S1 - (LEN - hlen)给skb1。

如果分片完全用来补给skb，那么该分片数据区仅仅关联skb，与skb1无关。

```c
// 这是拆分分片结构数据区数据，同理，其他参数不变，参数：pos则是sk_buff结构数据区中有效数据长度
static inline void skb_split_no_header(struct sk_buff *skb,
				       struct sk_buff* skb1,
				       const u32 len, int pos)//len为拆分后的skb的新长度;pos是skb数据区长度
{
	int i, k = 0;
	// 开始设置sk_buff结构数据区内容
	const int nfrags 				= skb_shinfo(skb)->nr_frags;
	skb_shinfo(skb)->nr_frags 		= 0;// 因为当前不知道需要几个分片数据能够补上len，所以先为0
	skb1->len		  				= skb1->data_len = skb->len - len;// skb1只有分片数据区
	skb->len		  				= len;
	skb->data_len		  			= len - pos;// skb的数据区长度不够，所以不可能再分割，还是pos字节，那么需要还分片数据区长度len - pos
	
	// 这是循环拆分分片结构数据区数据
	for (i = 0; i < nfrags; i++) {
		int size = skb_shinfo(skb)->frags[i].size;
		// 其实拆分，数据区存储不会动，动的只是指向这些数据存储的位置指针
        // 下面都是把skb的一些指向分片结构数据区的指针赋值给skb1中的数据区相关变量
        // pos不再表示skb的数据区长度，而是当前skb的数据总长度。判断当前skb总长度加上当前分片长度是否满足len：满足
        // 所以拆分当前分片，部分给skb，刚好总长度达到len；其余给skb1
		if (pos + size > len) {
            // 第一次进入时，k为0，size的一部分长度会分给skb1的第一个分片数据区
            // 继续循环时，还会进入这里，k持续增加，也就是满足skb的len要求后，剩余的分片，全部给skb1（@@@继续循环继续吃@@@）
			skb_shinfo(skb1)->frags[k] = skb_shinfo(skb)->frags[i];
			if (pos < len) {// 仅满足一次，也就是满足了len要求后，skb吃饱了，就不再吃了
                // pos + size > len && pos < len，说明第一次pos达到len要求；后面pos += size;以后，肯定不会再进来了
                // 当前分片数据区长度满足分割要求时，分成两部分
				get_page(skb_shinfo(skb)->frags[i].page);
				skb_shinfo(skb1)->frags[0].page_offset += len - pos;// 分片数据区偏移len - pos后，是skb1的第一个分片数据区的开始
				skb_shinfo(skb1)->frags[0].size -= len - pos;// skb1的第一个分片数据区长度，因为需要拿出len - pos给skb
				skb_shinfo(skb)->frags[i].size	= len - pos;// skb的最后一个分片数据区长度，skb吃饱了
				skb_shinfo(skb)->nr_frags++;				// skb的最后一个分片数据区长度，skb吃饱了
			}
			k++;//@@@继续循环继续吃@@@
		} 
        else// 不满足，skb完全吃下当前分片，@@@继续循环继续吃@@@
			skb_shinfo(skb)->nr_frags++;
        
        // 当进入分支if (pos < len)后，再执行到这一步，pos会大于len，不会再进入这个if分支了。所以后面skb分片不再变化，skb1吃掉剩余的分片
		pos += size;
	}
	skb_shinfo(skb1)->nr_frags = k;
}
```



![](D:\10000_works\zzztmp\截图\skb_split-拆分非线性区域.jpg)

------

## skb_shared_info

分片结构：用来表示IP分片的一个结构体，实则上是和sk_buff结构的数据区相连的，即是end指针的下一个字节开始就是分片结构。也正是此原因，所以分片结构和sk_buff数据区内存分配及销毁时都是一起的。

这个分片结构体和sk_buff结构的数据区是一体的，所以在各种操作时都把他们两个结构看做是一个来操作。

![](D:\10000_works\zzztmp\截图\sk_buff结构的数据区和分片结构的关系图.png)

从上图也可以看出来分片结构和sk_buff的数据区连在一起，==end指针的下个字节==就是分片结构的开始位置。

访问分片结构时，可以直接用end指针作为这个分片结构体的开始（记得要强转成分片结构体）或者用内核定义好的宏： 

#define skb_shinfo(SKB) ((struct skb_shared_info *)((SKB)->end)) 	去访问也可以，其本质也是返回个sk_buff的end指针。

分片结构体有两种组织形式，一种是skb_frag_t ==数组==，记录了分片存储的页、页中偏移、长度；

另一种是sk_buff组成的==链表==。

还要注意上面结构的组成：

skb_shared_info结构体 是和 sk_buff的数据区连接在一起的，分配sk_buff结构体的时候，一起分配的。

先分配sk_buff结构体的内存，然后分配skb_shared_info结构体 是和 sk_buff的数据区的内存，使用sk_buff的四个指针进行关联。

skb_shared_info的数据区，也就是分片数据，并不和 sk_buff的数据区连接在一起，是外部的，由skb_shared_info结构体中的字段来进行指示存放位置。

```c

/* This data is invariant across clones and lives at
 * the end of the header data, ie. at skb->end.
 */
struct skb_shared_info {
	__u8		flags;
	__u8		meta_len;
	__u8		nr_frags;// 表示有多少个分片结构
	__u8		tx_flags;
	unsigned short	gso_size;
	/* Warning: this field is not always filled in (UFO)! */
	unsigned short	gso_segs;
	struct sk_buff	*frag_list; // 这也是一种类型的分片数据，不是采用frags[]这种数组方式连接分片，而是链表方式
	struct skb_shared_hwtstamps hwtstamps;
	unsigned int	gso_type;// 分片的类型
	u32		tskey;

	/*
	 * Warning : all fields before dataref are cleared in __alloc_skb()--->__build_skb_around()
	 // 用于数据区的引用计数,克隆一个skb结构体时，会增加一个引用计数
	 */
	atomic_t	dataref;

	/* Intermediate layers must ensure that destructor_arg
	 * remains valid until skb destructor */
	void *		destructor_arg;

	/* must be last field, see pskb_expand_head() */
	skb_frag_t	frags[MAX_SKB_FRAGS];// 数组形式连接的分片结构的数据区
};

typedef struct bio_vec skb_frag_t;

/**  linux-5.14.14/include/linux/bvec.h
 * struct bio_vec - a contiguous range of physical memory addresses
 * @bv_page:   First page associated with the address range.
 * @bv_len:    Number of bytes in the address range.
 * @bv_offset: Start of the address range relative to the start of @bv_page.
 *
 * The following holds for a bvec if n * PAGE_SIZE < bv_offset + bv_len:
 *
 *   nth_page(@bv_page, n) == @bv_page + n
 *
 * This holds because page_is_mergeable() checks the above property.
 */
struct bio_vec {
	struct page	*bv_page;// 指向分片数据区的指针，类似于sk_buff中的data指针
	unsigned int	bv_len;// 数据区的长度，即：sk_buff结构中的data_len
	unsigned int	bv_offset;// 偏移量，表示从page指针指向的地方，偏移page_offset
};
```

![](D:\10000_works\zzztmp\截图\两种存储方式的分片数据区.png)

------



## alloc_skb

size是sk_buff数据区长度。

分配好之后，申请的内存包括：

sk_buff结构体一个（或者sk_buff_fclones）、

size大小的sk_buff数据区、

skb_shared_info分片结构体一个，不包括分片数据区

```c
/**
 * alloc_skb - allocate a network buffer
 * @size: size to allocate
 * @priority: allocation mask
 *
 * This function is a convenient wrapper around __alloc_skb().
 */
static inline struct sk_buff *alloc_skb(unsigned int size,
					gfp_t priority)
{
	return __alloc_skb(size, priority, 0, NUMA_NO_NODE);
}

/* 	Allocate a new skbuff. We do this ourselves so we can fill in a few
 *	'private' fields and also do memory statistics to find all the
 *	[BEEP] leaks.
 *
 */

/**
 *	__alloc_skb	-	allocate a network buffer
 *	@size: size to allocate
 *	@gfp_mask: allocation mask
 *	@flags: If SKB_ALLOC_FCLONE is set, allocate from fclone cache
 *		instead of head cache and allocate a cloned (child) skb.
 *		If SKB_ALLOC_RX is set, __GFP_MEMALLOC will be used for
 *		allocations in case the data is required for writeback
 *	@node: numa node to allocate memory on
 *
 *	Allocate a new &sk_buff. The returned buffer has no headroom and a
 *	tail room of at least size bytes. The object has a reference count
 *	of one. The return is the buffer. On a failure the return is %NULL.
 *
 *	Buffers may only be allocated from interrupts using a @gfp_mask of
 *	%GFP_ATOMIC.
 */
struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
			    int flags, int node)
{
	struct kmem_cache *cache;
	struct sk_buff *skb;
	u8 *data;
	bool pfmemalloc;

	cache = (flags & SKB_ALLOC_FCLONE)
		? skbuff_fclone_cache : skbuff_head_cache;// skbuff_head_cache, 见下一节讨论

	if (sk_memalloc_socks() && (flags & SKB_ALLOC_RX))
		gfp_mask |= __GFP_MEMALLOC;

	/* Get the HEAD */
	if ((flags & (SKB_ALLOC_FCLONE | SKB_ALLOC_NAPI)) == SKB_ALLOC_NAPI &&
	    likely(node == NUMA_NO_NODE || node == numa_mem_id()))
		skb = napi_skb_cache_get();
	else
		skb = kmem_cache_alloc_node(cache, gfp_mask & ~GFP_DMA, node);// 从指定段中为skb分配内存，分配的方式是 不允许在DMA内存中分配，因为DMA内存比较小，且有特定的作用，一般不用来分配skb。
	if (unlikely(!skb))
		return NULL;
	prefetchw(skb);

	/* We do our best to align skb_shared_info on a separate cache
	 * line. It usually works because kmalloc(X > SMP_CACHE_BYTES) gives
	 * aligned memory blocks, unless SLUB/SLAB debug is enabled.
	 * Both skb->head and skb_shared_info are cache line aligned.
	 */
	size = SKB_DATA_ALIGN(size);// sk_buff数据区长度
	size += SKB_DATA_ALIGN(sizeof(struct skb_shared_info));// sk_buff数据区长度 + skb_shared_info结构体长度
	data = kmalloc_reserve(size, gfp_mask, node, &pfmemalloc);// 申请的内存区域，长度是 sk_buff数据区长度 + skb_shared_info结构体长度
	if (unlikely(!data))
		goto nodata;
	/* kmalloc(size) might give us more room than requested.
	 * Put skb_shared_info exactly at the end of allocated zone,
	 * to allow max possible filling before reallocation.
	 */
	size = SKB_WITH_OVERHEAD(ksize(data));// sk_buff数据区长度 + skb_shared_info结构体长度
	prefetchw(data + size);

	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
	memset(skb, 0, offsetof(struct sk_buff, tail));
	__build_skb_around(skb, data, 0);
	skb->pfmemalloc = pfmemalloc;

	if (flags & SKB_ALLOC_FCLONE) {
		struct sk_buff_fclones *fclones;
        
		/* sk_buff_fclones : [struct sk_buff: skb1][struct sk_buff: skb2][refcount_t: fclone_ref] */
		fclones = container_of(skb, struct sk_buff_fclones, skb1);// container_of(ptr, type, member) skb1--->fclones

		skb->fclone = SKB_FCLONE_ORIG;// 1
		refcount_set(&fclones->fclone_ref, 1);

		fclones->skb2.fclone = SKB_FCLONE_CLONE;// 2
	}

	return skb;

nodata:
	kmem_cache_free(cache, skb);
	return NULL;
}


/* Caller must provide SKB that is memset cleared */
static void __build_skb_around(struct sk_buff *skb, void *data,
			       unsigned int frag_size)
{
	struct skb_shared_info *shinfo;
	unsigned int size = frag_size ? : ksize(data);// 申请的内存区域，长度是 sk_buff数据区长度 + skb_shared_info结构体长度

	size -= SKB_DATA_ALIGN(sizeof(struct skb_shared_info));// size本来是 sk_buff数据区长度 + skb_shared_info结构体长度 之和，减法后就是  sk_buff数据区长度

	/* Assumes caller memset cleared SKB */
	skb->truesize = SKB_TRUESIZE(size); // size目前是sk_buff数据区长度，skb->truesize 包括sk_buff结构体长度、sk_buff数据区长度以及skb_shared_info结构体长度
	refcount_set(&skb->users, 1);// 有多少处正在引用这个结构体，为0时才能free
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb); // skb->tail = skb->data; 这仨都指向 data，即申请到的内存区域的起始位置
	skb->end = skb->tail + size; // size目前是sk_buff数据区长度，所以end指向了sk_buff数据区结尾处
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb); // 宏 ((struct skb_shared_info *)(skb->end))，就是紧跟在sk_buff数据区结尾处的skb_shared_info结构体，表示该sk_buff的分片信息，分片数据存储区域还没有申请
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);

	skb_set_kcov_handle(skb, kcov_common_handle());
}

#define SKB_TRUESIZE(X) ((X) +						\
			 SKB_DATA_ALIGN(sizeof(struct sk_buff)) +	\
			 SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
```

------



## clone相关

```c
	cache = (flags & SKB_ALLOC_FCLONE)
		? skbuff_fclone_cache : skbuff_head_cache;// skbuff_head_cache
```

由传入的参数来决定用哪个缓冲区中的内存来分配（一般是用*skbuff_head_cache*缓存池来分配）。

【缓存池】内核对于sk_buff结构的内存分配不是和一般的结构动态内存申请一样：只分配指定大小的内存空间，而是在开始的时候，在初始化函数skb_init()中就分配了两段内存（*skbuff_head_cache* 和 *skbuff_fclone_cache* ）来供sk_buff后期申请时用，所以后期要为sk_buff结构动态申请内存时，都会从这两段内存中来申请（其实这不叫申请了，因为这两段内存开始就申请好了的，只是根据你要的内存大小从某个你选定的内存段中还回个指针给你罢了）。

如果在这个内存段中申请失败，则再用内核中用最低层，最基本的kmalloc()来申请内存了（这才是真正的申请）。

释放时也一样，并不会真正的释放，只是把数据清零，然后放回内存段中，以供下次sk_buff结构的申请。

这是内核动态申请的一种策略，专门为那些经常要申请和释放的结构设计的，这种策略不仅可以提高申请和释放时的效率，而且还可以减少内存碎片的。（注意：上面提到的内存段中的段不是指内存管理中的段、页的段，而是表示块，就是两块比较大的内存）



```c
	if (flags & SKB_ALLOC_FCLONE) {
		struct sk_buff_fclones *fclones;// fclones, for clones
        
		/* sk_buff_fclones : [struct sk_buff: skb1][struct sk_buff: skb2][refcount_t: fclone_ref] */
		fclones = container_of(skb, struct sk_buff_fclones, skb1);// container_of(ptr, type, member) skb1--->fclones

		skb->fclone = SKB_FCLONE_ORIG;// 1
		refcount_set(&fclones->fclone_ref, 1);

		fclones->skb2.fclone = SKB_FCLONE_CLONE;// 2
	}
```

*skbuff_fclone_cache* 和 *skbuff_head_cache*两个块内存缓存池是不一样的。

我们一般是在skbuff_head_cache这块缓存池中来申请内存的，但是如果你开始就知道这个skb将很有可能**被克隆**（至于克隆和复制将在下一篇bolg讲），那么你最好还是选择在skbuff_fclone_cache缓存池中申请。

因为在这块缓存池中申请的话，将会返回==2个skb==的内存空间，第二个skb则是用来作为克隆时使用(sk_buff_fclones.skb2)。（其实从函数名字就应该知道是为克隆准备的，fclone嘛）虽然是分配了两个sk_buff结构内存，但是数据区却是只有一个的，所以是**两个sk_buff结构中的指针都是指向这一个数据区的**。

也正因为如此，所以分配sk_buff结构时也顺便分配了个引用计数器(sk_buff_fclones.fclone_ref)，就是来表示有多少个sk_buff结构在指向数据区（引用同一个数据区），这是为了防止还有sk_buff结构在指向数据区时，而销毁掉这个数据区。有了这个引用计数，一般在销毁时，先查看这个引用计数是否为0，如果不是为0，就让引用计数将去1；如果是0，才真正销毁掉这个数据区。

```c
// 老版本代码更能说明 两个skb、fclone_ref的存储关系，
// 存储关系是：[struct sk_buff: skb1][struct sk_buff: skb2][refcount_t: fclone_ref]，只是没有封装sk_buff_fclones结构体
if (fclone) {
    struct sk_buff *child = skb + 1;// 用child结构体变量来指向第二sk_buff结构体内存地址, skb是sk_buff* 类型指针
    atomic_t *fclone_ref = (atomic_t *) (child + 1);// 获取到引用计数器，因为引用计数器是在第二个sk_buff结构体内存下一个字节空间开始的，所以用(child + 1)来获取到引用计数器的开始地址。

    atomic_set(fclone_ref, 1);

}
```



-  skb_clone函数仅仅是克隆个sk_buff结构体，其他数据都是共享；

-  pskb_copy函数克隆复制了sk_buff和其数据区(包括分片结构体)，其他数据共享；

-  skb_copy函数则是完全的复制拷贝函数了，把sk_buff结构体和其数据区（包括分片结构体）、分片结构的数据区都复制拷贝了一份。

    

### skb_clone

```c
enum {
	SKB_FCLONE_UNAVAILABLE,	/* skb has no fclone (from head_cache) */
	SKB_FCLONE_ORIG,	/* orig skb (from fclone_cache) */
	SKB_FCLONE_CLONE,	/* companion fclone skb (from fclone_cache) */
};

/**
 *	skb_clone	-	duplicate an sk_buff
 *	@skb: buffer to clone
 *	@gfp_mask: allocation priority
 *
 *	Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *	copies share the same packet data but not structure. The new
 *	buffer has a reference count of 1. If the allocation fails the
 *	function returns %NULL otherwise the new buffer is returned.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 */
/*
skb->fclone 这个值：
在 __alloc_skb 函数中，一开始使用 memset(skb, 0, offsetof(struct sk_buff, tail)); 将其置为0（SKB_FCLONE_UNAVAILABLE）；
然后判断flags是否有 SKB_ALLOC_FCLONE 置位，如果有，那么skb->fclone = SKB_FCLONE_ORIG; 并且 fclones->skb2.fclone = SKB_FCLONE_CLONE;
*/

/*  整体看__alloc_skb 函数中，clone方面的代码
	cache = (flags & SKB_ALLOC_FCLONE)
		? skbuff_fclone_cache : skbuff_head_cache;// skbuff_fclone_cache
		
	skb = kmem_cache_alloc_node(cache, gfp_mask & ~GFP_DMA, node);// 所以此时是两个skb结构体返回，其实申请的是sk_buff_fclones
																  // 返回了第一个skb位置
	if (flags & SKB_ALLOC_FCLONE) {
		struct sk_buff_fclones *fclones;

		fclones = container_of(skb, struct sk_buff_fclones, skb1);// 所以才能执行这样的操作，要是非clone，不会有两个skb
																  // 返回这个skb所属的sk_buff_fclones
		skb->fclone = SKB_FCLONE_ORIG;// 值为1
		refcount_set(&fclones->fclone_ref, 1);// ref为1，表示没有被clone过

		fclones->skb2.fclone = SKB_FCLONE_CLONE;// 值为2
	}
*/
struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff_fclones *fclones = container_of(skb,
						       struct sk_buff_fclones,
						       skb1);
	struct sk_buff *n;

	if (skb_orphan_frags(skb, gfp_mask))
		return NULL;

	if (skb->fclone == SKB_FCLONE_ORIG &&//说明__alloc_skb 中，flags有 SKB_ALLOC_FCLONE 置位
	    refcount_read(&fclones->fclone_ref) == 1) {//上面代码也有这个
		n = &fclones->skb2;// 因为本身是SKB_ALLOC_FCLONE，所以__alloc_skb中，分配了两个skb，n指向第二个
		refcount_set(&fclones->fclone_ref, 2);// ref为2，表示clone了
	} else {// __alloc_skb只分配了一个skb，这里需要再分配一个
		if (skb_pfmemalloc(skb))
			gfp_mask |= __GFP_MEMALLOC;
		
        // 因为还不能确定这个skb(即clone出来的这个)是否还会被克隆，所以到skbuff_head_cache缓存池上去申请；
        // 同时设置其clone标志为0（SKB_FCLONE_UNAVAILABLE）
		n = kmem_cache_alloc(skbuff_head_cache, gfp_mask);
		if (!n)
			return NULL;

		n->fclone = SKB_FCLONE_UNAVAILABLE;
	}
	// skb_clone()函数实现的仅仅是克隆一个skb内存空间，而一些数据拷贝复制则是用_skb_clone()函数来完成。
    // 所以 __skb_clone() 函数主要是实现从skb中把相关成员字段拷贝到n中去
	return __skb_clone(n, skb);
}

/*
 * You should not add any new code to this function.  Add it to
 * __copy_skb_header above instead.
 */
static struct sk_buff *__skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
#define C(x) n->x = skb->x	// @@@ C(x)作用域开始

	n->next = n->prev = NULL;
	n->sk = NULL;
	__copy_skb_header(n, skb);

	C(len);
	C(data_len);
	C(mac_len);
	n->hdr_len = skb->nohdr ? skb_headroom(skb) : skb->hdr_len;
	n->cloned = 1;
	n->nohdr = 0;
	n->peeked = 0;
	C(pfmemalloc);
	C(pp_recycle);
	n->destructor = NULL;
	C(tail);
	C(end);
	C(head);
	C(head_frag);
	C(data);
	C(truesize);
	refcount_set(&n->users, 1);//设置n的引用计数为1，表明还有另外一个skb（其实就是父skb），防止n释放时连同共享数据区一起释放(因为父skb还在)。
	
    /* 
    __build_skb_around 中，已经将shinfo->dataref设置为1，所以clone后，ref需要增加，表示两个位置的skb在引用这个分片结构体。    
    这个简单的说就是，因为开始也讲过sk_buff的数据区和分片结构是一体的，连内存申请和释放都是一起的。
    而dataref是分片结构skb_shared_info中的一个 表示sk_buff的数据区和分片结构被多少skb共享的 成员字段。
    这里调用atomic_inc()函数让该引用计数器自增，表明克隆skb对sk_buff数据区和分片结构的共享引用。*/
	atomic_inc(&(skb_shinfo(skb)->dataref));
	skb->cloned = 1;//表明这是个克隆的skb结构体。

	return n;
#undef C					// @@@  C(x)作用域结束
}
```

skb_clone()函数的效果图

![](D:\10000_works\zzztmp\截图\skb_clone.png)

![skb_clone2](D:\10000_works\zzztmp\截图\skb_clone2.png)

其实上面的方法：由skb_clone()函数克隆一个skb，然后共享其他数据。虽然可以提高效率，但是存在一个很大的==缺陷==：

就是当有克隆skb指向共享数据区（skb数据区、分片结构和分片数据区）时，那么共享数据区的数据就不能被修改了。

所以说如果只是让多个skb查看共享数据区内容，则可以用skb_clone()函数来克隆这几个skb出来，提高效率。

但如果涉及到某个skb要修改sk_buff结构的数据区，则必须要用下面这几个函数来克隆拷贝出skb。


### pskb_copy

```c
static inline struct sk_buff *pskb_copy(struct sk_buff *skb,
					gfp_t gfp_mask)
{
	return __pskb_copy(skb, skb_headroom(skb), gfp_mask);// headroom: skb->data - skb->head;
}

static inline struct sk_buff *__pskb_copy(struct sk_buff *skb, int headroom,
					  gfp_t gfp_mask)
{
	return __pskb_copy_fclone(skb, headroom, gfp_mask, false);
}


/**
 *	__pskb_copy_fclone	-  create copy of an sk_buff with private head.
 *	@skb: buffer to copy
 *	@headroom: headroom of new skb
 *	@gfp_mask: allocation priority
 *	@fclone: if true allocate the copy of the skb from the fclone
 *	cache instead of the head cache; it is recommended to set this
 *	to true for the cases where the copy will likely be cloned
 *
 *	Make a copy of both an &sk_buff and part of its data, located
 *	in header. Fragmented data remain shared. This is used when
 *	the caller wishes to modify only header of &sk_buff and needs
 *	private copy of the header to alter. Returns %NULL on failure
 *	or the pointer to the buffer on success.
 *	The returned buffer has a reference count of 1.
 */

struct sk_buff *__pskb_copy_fclone(struct sk_buff *skb, int headroom,
				   gfp_t gfp_mask, bool fclone)
{
    // headlen: len - data_len; 另  len = (tail - data) + data_len  所以headlen=tail - data
    // headlen + headroom = tail - data + data - head = tail - head
	unsigned int size = skb_headlen(skb) + headroom;// tail - head
	int flags = skb_alloc_rx_flag(skb) | (fclone ? SKB_ALLOC_FCLONE : 0);
	struct sk_buff *n = __alloc_skb(size, gfp_mask, flags, NUMA_NO_NODE); // n包括了sk_buff结构体一个（或者sk_buff_fclones）、size大小的sk_buff数据区、skb_shared_info分片结构体一个，不包括分片数据区(正好这个函数不clone分片数据区，共享)

	if (!n)
		goto out;

	/* Set the data pointer */
	skb_reserve(n, headroom);// headroom: skb->data - skb->head;
	/* Set the tail pointer and length */
	skb_put(n, skb_headlen(skb));// headlen: len - data_len
	/* Copy the bytes 
	这是个内存拷贝的封装函数，就是从被拷贝的skb结构中的data指针指向的地方开始，
	偏移len  （因为len = (data - tail) + data_len；所以这里本应该写成(data - tail)的，但考虑到此时分片结构数据区还没有数据，data_len为零。
	即是len = data-tail）    个字节内容都拷贝到新复制到的skb结构体中去。
	即是：用被拷贝的skb中的数据区内容来为新拷贝的skb结构的数据区填充。
	
	因为这一步不考虑分片区域，所以就将skb的sk_buff数据区数据拷贝到n的sk_buff数据区；
	本来这个区域长度是 n->len - n->data_len，因为这一步data_len为零，所以就是n->len
	*/    
	skb_copy_from_linear_data(skb, n->data, n->len);// memcpy(n->data, skb->data, n->len);

	n->truesize += skb->data_len;
	n->data_len  = skb->data_len;
	n->len	     = skb->len;

	if (skb_shinfo(skb)->nr_frags) {// 分片个数， 这里是数组形式组织的分片的拷贝
		int i;

		if (skb_orphan_frags(skb, gfp_mask) ||
		    skb_zerocopy_clone(n, skb, gfp_mask)) {
			kfree_skb(n);
			n = NULL;
			goto out;
		}
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			skb_shinfo(n)->frags[i] = skb_shinfo(skb)->frags[i];// skb_frag_t结构体的赋值，分片数据区是共享的，让新的skb中的分片结构指针指向共享的分片结构数据区。
			skb_frag_ref(skb, i);// get_page(frag->bv_page) ---> page_ref_inc(page);  分片数据区是共享的，所以增加该数据区所在page的引用次数
		}
		skb_shinfo(n)->nr_frags = i;// 新旧分片个数肯定一样
	}

	if (skb_has_frag_list(skb)) {// 这里是链表形式组织的分片的拷贝, skb_shinfo(skb)->frag_list != NULL;
		skb_shinfo(n)->frag_list = skb_shinfo(skb)->frag_list;
		skb_clone_fraglist(n);// skb_walk_frags遍历链表上的每个skb，执行skb_get，增加ref count，因为这种组织形式也是共享的
	}

	skb_copy_header(n, skb);
out:
	return n;
}
```

主要是分配skb及数据区内存----》对数据区拷贝赋值----》处理分片结构数据区内存----》为其他成员变量拷贝赋值。

![](D:\10000_works\zzztmp\截图\pskb_copy.png)



### skb_copy

```c

/**
 *	skb_copy	-	create private copy of an sk_buff
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data. This is used when the
 *	caller wishes to modify the data and needs a private copy of the
 *	data to alter. Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	As by-product this function converts non-linear &sk_buff to linear
 *	one, so that &sk_buff becomes completely private and caller is allowed
 *	to modify all the data of returned buffer. This means that this
 *	function is not recommended for use in circumstances when only
 *	header is going to be modified. Use pskb_copy() instead.
 */

struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp_mask)
{
	int headerlen = skb_headroom(skb);// headroom: skb->data - skb->head;
	unsigned int size = skb_end_offset(skb) + skb->data_len;// skb->end - skb->head + skb->data_len = skb->len
	struct sk_buff *n = __alloc_skb(size, gfp_mask,
					skb_alloc_rx_flag(skb), NUMA_NO_NODE);// 也包括了分片数据区的内存部分

	if (!n)
		return NULL;

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	BUG_ON(skb_copy_bits(skb, -headerlen, n->head, headerlen + skb->len));

	skb_copy_header(n, skb);
	return n;
}

/**	
 *	skb_copy_bits - copy bits from skb to kernel buffer
 *	@skb: source skb
 *	@offset: offset in source				-headroom
 *	@to: destination buffer					head指针
 *	@len: number of bytes to copy			headroom + skb->len
 *
 *	Copy the specified number of bytes from the source skb to the
 *	destination buffer.
 *
 *	CAUTION ! :
 *		If its prototype is ever changed,
 *		check arch/{*}/net/{*}.S files,
 *		since it is called from BPF assembly code.
 */
// http://blog.chinaunix.net/uid-26940719-id-3199680.html
int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int start = skb_headlen(skb);// skb->len - skb->data_len;	skb数据区长度
	struct sk_buff *frag_iter;
	int i, copy;

	if (offset > (int)skb->len - len)// skb->len - (skb->data - skb->head + skb->len) = -(skb->data - skb->head)
		goto fault;

	/* Copy header. */
	if ((copy = start - offset) > 0) {// skb数据区长度 + headroom长度
		if (copy > len)
			copy = len;
        // memcpy(to, skb->data - headroom, copy) ---> memcpy(to, skb->head, copy)
        // skb的headroom和数据区，都拷贝到新的对应区域
		skb_copy_from_linear_data_offset(skb, offset, to, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;// -headroom长度 + (skb数据区长度 + headroom长度) = skb数据区长度
		to     += copy;// head指针 +  (skb数据区长度 + headroom长度) = skb数据区结尾处
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;
		skb_frag_t *f = &skb_shinfo(skb)->frags[i];

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(f);// start + frag->bv_len
		if ((copy = end - offset) > 0) {// start + frag->bv_len - skb数据区长度 = frag->bv_len, copy就是当前分片的长度
			u32 p_off, p_len, copied;
			struct page *p;
			u8 *vaddr;

			if (copy > len)
				copy = len;

			skb_frag_foreach_page(f,
					      skb_frag_off(f) + offset - start,
					      copy, p, p_off, p_len, copied) {
				vaddr = kmap_atomic(p);
				memcpy(to + copied, vaddr + p_off, p_len);// 复制分片数据块中的内容，能看到to指向dest位置
				kunmap_atomic(vaddr);
			}

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to     += copy;// to移动到下一个分片存储位置，准备下一个分片的数据拷贝
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			if (copy > len)
				copy = len;
			if (skb_copy_bits(frag_iter, offset - start, to, copy))
				goto fault;
			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to     += copy;// to移动到下一个分片存储位置，准备下一个分片的数据拷贝
		}
		start = end;
	}

	if (!len)
		return 0;

fault:
	return -EFAULT;
}
```



![](D:\10000_works\zzztmp\截图\skb_copy.png)



首先还是来说下sk_buff结构及相关结构体，第一块是sk_buff自身结构体，第二块是sk_buff结构的数据区及分片结构体（他们始终在一起），第三块则是分片结构中的数据区。
然后来总结下各个函数的不同点：
        skb_clone（）函数仅仅是克隆个sk_buff结构体，其他数据都是共享；
        pskb_copy()函数克隆复制了sk_buff和其数据区(包括分片结构体)，其他数据共享；
        skb_copy()函数则是完全的复制拷贝函数了，把sk_buff结构体和其数据区（包括分片结构体）、分片结构的数据区都复制拷贝了一份。
为什么要定义这么多个复制拷贝函数呢？ 其真正的原因是：不能修改共享数据。所以如果想要修改共享数据，只能把这份共享数据拷贝一份，因此就有了这几个不同的复制拷贝函数了。 选择使用哪个复制拷贝函数时就根据你所要修改的哪块共享数据区来定。

------

## kfree_skb

（1）kfree_skb()函数首先是获取到skb->users成员字段，这是个引用计数器，表示有多少处正在引用这个结构体。
如果不为1的话，表示还有其他人在引用他，不能释放掉这个结构体，否则会让引用者出现野指针、非法操作内存等错误。那么kfree_skb()函数只是简单的skb->users减去个1而已，表明我不再引用这个结构体了。
如果skb->users == 1，则表明是最后一个引用该结构体的，所以可以调用_kfree_skb()函数直接释放掉了（也不是释放，而是放回到缓存池中）。

（2）当skb释放掉后，dst_release同样会被调用以减小相关dst_entry数据结构的引用计数。
如果destructor（skb的析构函数）被初始化过，相应的函数会在此时被调用。

（3）还有分片结构体（skb_shared_info）也会相应的被释放掉，然后把所有内存空间全部返还到***skbuff_head_cache***缓存池中，这些操作都是由kfree_skbmem()函数来完成的。
这里分片的释放涉及到了克隆问题：
如果skb没有被克隆，数据区也没有其他skb引用，则直接释放即可；
如果是克隆了skb结构，则当克隆数计数为1时，才能释放skb结构体；(**sk_buff_fclones.fclone_ref**??? )
如果分片结构被克隆了，那么也要等到分片克隆计数为1时，才能释放掉分片数据结构。（**skb_shared_info.dataref** ???）
如果skb是从***skbuff_fclone_cache***缓存池中申请的内存时，则要仔细销毁过程了，因为从这个缓存池中申请的内存，会返还2个skb结构体和一个引用计数器。
所以销毁时不仅要考虑克隆问题还要考虑2个skb的释放顺序。