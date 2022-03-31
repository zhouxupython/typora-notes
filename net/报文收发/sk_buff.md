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



| [Linux内核网络源码解析1——sk_buff结构](https://liu-jianhao.github.io/2019/05/linux内核网络源码解析1sk_buff结构/) |      |      |
| ------------------------------------------------------------ | ---- | ---- |
| [Linux内核网络源码解析2——sk_buff操作](https://liu-jianhao.github.io/2019/05/linux内核网络源码解析2sk_buff操作/) |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |
|                                                              |      |      |



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
 */
static inline void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
}
```

![](D:\10000_works\zzztmp\截图\sk_buff_2.6.20_put_push_pull_reserve.png)

reserve：

开始准备存储应用层下发过来的数据，通过调用函数 skb_reserve()来使data指针和tail指针同时向下移动，空出一部分空间来为后期添加==协议==信息。

put：

开始存储数据时，通过调用函数skb_put()来使tail指针向下移动空出空间来添加数据，此时skb->data和skb->tail之间存放的都是==数据==信息，无协议信息。

push、pull处理各层协议头的：

push是高层协议到低层协议，通常用于发送的数据包后在各层由上往下传递时，添加下层的协议首部；这时就开始调用函数skb_push()来使data指针向上移动，空出空间来添加各层==协议==信息。直到最后到达二层，添加完帧头然后就开始发包了。

pull是低层协议到高层协议，通常用于接收的数据包后在各层由下往上传递时，上层忽略下层的==协议==信息。

![](D:\10000_works\zzztmp\截图\sk_buff在各层协议之间传输.png)

------

```c
/*
克隆sk_buff
只复制sk_buff描述符，同时增加数据缓存区的引用计数
*/
skb_clone
```

![](D:\10000_works\zzztmp\截图\skb_clone.png)

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

```c
/*
添加尾部数据
将指定用户空间的数据添加到skb_buff的数据缓存区的尾部
*/
skb_add_data
```

![](D:\10000_works\zzztmp\截图\skb_add_data.png)

```c
/*
删除尾部数据：skb_trim()
与skb_add_data()操作相反
*/
```

![](D:\10000_works\zzztmp\截图\skb_trim.png)

------

```c
/*
拆分数据：skb_split()
根据指定长度拆分sk_buff，使得原sk_buff中的数据长度为指定长度，剩下的数据保存到拆分得到的sk_buff中
LEN：拆分长度，hlen：线性数据长度 + 当拆分数据的长度小于线性数据长度时，直接拆分线性数据区即可


拆分数据的长度大于线性数据长度时，则需要拆分非线性区域中的数据，拆分长度LEN大于hlen并且LEN小于hlen+S1
*/


/*
拆分数据：skb_split()
根据指定长度拆分sk_buff，使得原sk_buff中的数据长度为指定长度，剩下的数据保存到拆分得到的sk_buff中
指向984zx
LEN：拆分长度；hlen：线性数据长度
（1）当拆分数据的长度小于线性数据长度时，直接拆分线性数据区即可
（2）拆分数据的长度大于线性数据长度时，则需要拆分非线性区域中的数据，拆分长度LEN大于hlen并且LEN小于hlen+S1
*/
```

![](D:\10000_works\zzztmp\截图\skb_split-直接拆分.jpg) 



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