Reference

* https://www.cs.usfca.edu/~galles/visualization/RedBlack.html

## 数据结构

#### 环形缓冲区(读完即丢)

算法描述(假设缓冲区大小为4)

```shell
		R=0
		||
		\/
buf[0]->[0][1][2][3]
        ||
        \/
		W=0
1、空, R==W
2、写, buf[W]=val, W = (W+1)%4; //到尽头就重新来
3、读, buf[R]=val, R = (R+1)%4; //到尽头就重新来
4、满, (W+1)%4 == R
```

编程

```c
#define len 4
int buf[len];
static int r = 0;
static int w = 0;
static int is_empty(void)
{
	return (r == w);
}
static int is_full(void)
{
	return ((w + 1) % len == r);
}
static void put(int x)
{
	if (is_full()) {
		r = (r + 1) % len; //如果满了还要写，就丢一个
	}
	buf[w] = x;
	w = (w + 1) % len;
}
static int get(int* p)
{
	if (is_empty) {
		return 0;
	}
	*p = buf[r];
	r = (r + 1) % len;
	
	return 1;
} 
```

#### 容器

```c
#define container_of(ptr, type, member) ({					\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\		/* 为了编译器检查类型 */
	(type *)( (char*)__mptr - offsetof(type, member) );})

#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)
```

#### 内核链表

* 内核链表采用了内嵌的设计理念，省去了大量重复的代码，使用时只需要调用容器，即可得到链表所对的结构体。

```c
/* 操作链表 */
#define LIST_HEAD(name);	// 返回名为name的list_head结构体，并将next,prev指针指向自己，即初始化
#define list_entry(ptr, type, member);// 返回父结构体
list_add(struct list_head *new, struct list_head *head);	// 向后插入节点
list_add_tail(struct list_head *new, struct list_head *head);	// 添加到链表尾部
list_del(struct list_head *entry);	// 删除节点，一般删除之后，还需要下面的操作
list_del_init(struct list_head *entry);	// 让被删掉的节点不乱指
list_move(struct list_head *list, struct list_head *head);	// 把节点从一个链表移动到另一个链表head节点后面
list_move_tail(struct list_head *list, struct list_head *head);	// 把节点从一个链表移动到另一个链表尾部
list_empty(const struct list_head *head);
list_splice(const struct list_head *list, struct list_head *head);	// 合并两个链表
list_splice_init(struct list_head *list, struct list_head *head);	// 合并两个链表并将list指向的链表初始化
/* 遍历链表 */
// 直接遍历链表项
struct list_head *p;
struct foo *f;
list_for_each(p, &foolist) {
    foo = list_entry(p, struct foo, list_membername);
}
// 直接遍历父对象
struct foo *f;
list_for_each_entry(f, &foolist, list_membername) {
    
}
// 反向则加_reverse后缀
/* 加safe代表遍历时删除，反向同理 */
list_for_each_entry_safe(pos, next, head, member);	//pos和next为同一类型，均为父对象类型
```

#### 内核队列

```c
struct kfifo fifo;
kfifo_alloc(&fifo, size, GFP_KERNEL);	/* return 0 means success */
#define	kfifo_in(fifo, buf, n);	/* return enqueue len, n is sizeof buf */
#define	kfifo_out(fifo, buf, n);	/* return dequeue len */
#define	kfifo_out_peek(fifo, buf, n);	/* just peek, not dequeue */
/* 获取长度 */
#define kfifo_size(fifo);	//返回fifo总长度
#define kfifo_len(fifo);	//返回fifo中已有数据的长度
#define	kfifo_avail(fifo);	//返回剩余空间
#define	kfifo_is_full(fifo);
#define	kfifo_is_empty(fifo);
#define kfifo_reset(fifo);	//重置
#define kfifo_free(fifo);	//释放
```

#### 二叉搜索树(BST)

* 任意节点左子树不为空，则值小于根节点
* 任意节点右子树不为空，则值大于根节点
* 任意节点的子树也为二叉查找树
* 没有键值相等的节点
* 遍历

```shell
         4                    前序遍历：根-左-右
        /  \                  4 2 1 3 2.5 6 5 5.5 7
      2     6          
     /  \  /  \               中序遍历：左-根-右
    1   3  5   7              1 2 2.5 3 4 5 5.5 6 7
       /    \
     2.5    5.5               后序遍历：左-右-根
                              1 2.5 3 2 5.5 5 7 6 4
```

* 二叉查找树，最左侧为最小值，最右侧为最大值
* 前驱节点：小于当前节点的最大值。后继节点：大于当前节点的最小值
* 删除节点：若节点同时具有左右节点，那么就用前驱或者后继去替代

#### 高度平衡树(AVL)

BST存在倾斜现象，不同的插入顺序也会使高度不同，AVL可以通过旋转操作将高度维持到logN，使左右子树高度差不超过1，但是AVL需要频繁旋转，比较麻烦，因此一种只需要平衡黑节点的结构被提出，即红黑树。

#### 2-3-4树

```shell
        5                    只能有2、3、4节点，多了就分裂，分裂中间节点会上升，少了就合并
       /  \                  2节点：包含1个元素，有2个子节点
      3   7&9                3节点：包含2个元素，有3个子节点
    / |   / | \              4节点：包含3个元素，有4个子节点
  1&2 4  6  8  10&11&12      每个叶子节点深度相同
```

2-3-4树避免了倾斜，但由于代码实现过于复杂，因此需要简单转换，转换结果，就是一个红黑树。

#### 红黑树(RBT)

```shell
2节点 5           '5'
3节点 7&9         '7'右倾         '9'左倾
                   \            /
                    9          7
4节点 10&11&12    '11'
                 /  \
                10   12
裂变 10&11&12&13  '11'       变色     11
                 /  \              /   \
               10   12&13        '10'  '12'
                                          \
                                           13
```

* 每个节点要么是黑色，要么是红色
* 根节点是黑色
* 每个叶子节点（NULL）是黑色
* 每个红色节点的两个子节点一定都是黑色
* 任意一节点到每个叶子节点的路径都包含数量相同的黑节点

```shell
左旋：  4                  6            右旋：  6                  4
     /   \              /   \               /   \              /   \
    3     6            4     7             4     7            3     6
        /   \        /   \               /  \                     /   \
      '5'     7      3   '5'            3   '5'                 '5'    7
左旋，右节点的左节点搭到旋转节点
右旋，左节点的右节点搭到旋转节点
如果旋转节点有父节点，则旋转过去要继承
```



## Arm Linux内存管理

#### 1. Arm LInux内存分区

* Linux以页为内存管理基本单元，以PAGESIZE定义，一般为0x1000，即4k

* 内存空间分为用户空间和内核空间，用户空间分布一般为0-3GB(PAGE_OFFSET)，以0xc0000000为分界，内核空间的虚拟地址被所有进程所共享

![memory_map](pic/memory/memory_map.png)

* 应用程序地址空间细分

![user_memory](pic/memory/user_memory.png)

#### 2. MMU二级页表映射关系

```mermaid
flowchart LR
	0x12345678--\n一级页表定位\n1M映射-->0x123
	0x12345678--二级页表定位\n4K映射-->0x45
	0x12345678--基地址偏移\n\n\n-->0x678
```

#### 3. cache数据传输机制

```mermaid
flowchart LR
	\n处理器\n内核\n\n<--字/字节访问\n高速-->Cache
	Cache--\n高速-->写缓冲器
	Cache<--块传输\n低速-->\n主存\n\n
	写缓冲器--\n低速-->\n主存\n\n
	\n主存\n\n<--字/字节访问\n低速-->\n处理器\n内核\n\n
```

根据是否使用**Cache**和**写缓冲器**可产生四种写数据的组合。

| enable Cache? | enable Write Buffer? | Explain                         |
| ------------- | -------------------- | ------------------------------- |
| 0             | 0                    | Non-cached, non-buffered (NCNB) |
| 0             | 1                    | write-combine(WC)               |
| 1             | 0                    | Cached, write-through mode (WT) |
| 1             | 1                    | Cached, write-back mode (WB)    |

方式1：读写直达硬件，适合寄存器的读写。

方式2：读写经写缓冲器写入内存，可能会发生写合并现象，适合显存等不需要读的操作。

方式3：适合于只读设备。

方式4：适合于一般的内存读写。

* 禁用Cache的场合：**Register**、**FrameBuffer**、**DMA**等需要数据一直是同步的情况。

