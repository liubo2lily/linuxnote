### 1. 时钟框架简介

该框架用于管理设备的时钟节点。包括时钟选通、速率调整、多路复用等功能。通过宏`CONFIG_COMMON_CLK`启用。框架分为个两层级，层级间都相互分离。

第一层，首先是clk结构体，它抽象出各个硬件平台的公共部分。其次是在drivers/clk/clk.c中定义的API（clk.h中声明）。最后是clk_ops结构体，里面的成员函数由API间接调用。

第二层是clk_ops结构体中硬件相关的函数以及clk_foo结构体。有关clk_ops结构体中的成员函数如.enable、.set_rate等都涉及具体的硬件控制。类似的，对clk_foo结构体可以实现对foo硬件的抽象。

可通过clk_hw结构体将以上两部分连接到一起，它在clk_foo结构体中定义，并指向clk_core结构体。

### 2. 通用数据结构体及api

以下是drivers/clk/clk.c中的通用clk_core结构体定义，以下抽取了其主要部分：

```c
struct clk_core {
	const char		*name;
	const struct clk_ops	*ops;
	struct clk_hw		*hw;
	struct module		*owner;
	struct clk_core		*parent;
	const char		**parent_names;
	struct clk_core		**parents;
	u8			num_parents;
	u8			new_parent_index;
	...
};
```

该结构体是构成clk树的核心。API自身包含了面向驱动程序的函数，这些函数使用clk结构体进行操作，这些API定义在include/linux/clk.h，使用clk_core结构体的平台设备可利用clk_core结构体中的clk_ops结构体指针来执行clk-provider.h中定义的硬件相关操作函数。

```c
struct clk_ops {
	int		(*prepare)(struct clk_hw *hw);
	void		(*unprepare)(struct clk_hw *hw);
	int		(*is_prepared)(struct clk_hw *hw);
	void		(*unprepare_unused)(struct clk_hw *hw);
	int		(*enable)(struct clk_hw *hw);
	void		(*disable)(struct clk_hw *hw);
	int		(*is_enabled)(struct clk_hw *hw);
	void		(*disable_unused)(struct clk_hw *hw);
	unsigned long	(*recalc_rate)(struct clk_hw *hw,
					unsigned long parent_rate);
	long		(*round_rate)(struct clk_hw *hw,
					unsigned long rate,
					unsigned long *parent_rate);
	int		(*determine_rate)(struct clk_hw *hw,
					  struct clk_rate_request *req);
	int		(*set_parent)(struct clk_hw *hw, u8 index);
	u8		(*get_parent)(struct clk_hw *hw);
	int		(*set_rate)(struct clk_hw *hw,
				    unsigned long rate,
				    unsigned long parent_rate);
	int		(*set_rate_and_parent)(struct clk_hw *hw,
				    unsigned long rate,
				    unsigned long parent_rate,
				    u8 index);
	unsigned long	(*recalc_accuracy)(struct clk_hw *hw,
					unsigned long parent_accuracy);
	int		(*get_phase)(struct clk_hw *hw);
	int		(*set_phase)(struct clk_hw *hw, int degrees);
	void		(*init)(struct clk_hw *hw);
	int		(*debug_init)(struct clk_hw *hw,
				      struct dentry *dentry);
};
```

### 3. 硬件clk实现

clk_core结构体重点在于.ops和.hw成员指针，他们从具体的硬件特性抽象出clk结构体细节，例如在dirivers/clk/clk-gate.c中可以实现时钟的选通。

```c
struct clk_gate {
    struct clk_hw	hw;
    void __iomem    *reg;
    u8              bit_idx;
    ...
};
```

clk_gate结构体包含了clk_hw结构体成员以及相关选通寄存器，至于其他的细节，都交由clk_core结构体处理。

可通过以下方式启用一个clk：

```c
struct clk *clk;
clk = clk_get(NULL, "my_gateable_clk");

clk_prepare(clk);
clk_enable(clk);
```

clk_enable的调用过程如下：

```c
clk_enable(clk);
    clk->ops->enable(clk->hw);
    [resolves to...]
        clk_gate_enable(hw);
        [resolves struct clk gate with to_clk_gate(hw)]
        	clk_gate_set_bit(gate);
```

clk_gate_set_bit的定义如下：

```c
static void clk_gate_set_bit(struct clk_gate *gate)
{
	u32 reg;	
	reg = __raw_readl(gate->reg);
	reg |= BIT(gate->bit_idx);
	writel(reg, gate->reg);
}
```

to_clk_gate的定义如下：

	#define to_clk_gate(_hw) container_of(_hw, struct clk_gate, hw)

这种抽象方式可类比应用于每个硬件时钟。

### 4. 支持自己的clk

在实现自己的时钟支持时，只需包括以下头文件：`#include <linux/clk-provider.h>`

还需要定义以下内容：

```c
struct clk_foo {
	struct clk_hw hw;
	... hardware specific data goes here ...
};
```

至少要包含以下成员：

```c
struct clk_ops clk_foo_ops {
	.enable		= &clk_foo_enable;
	.disable	= &clk_foo_disable;
};
```

可通过容器实现上述功能：

```c
#define to_clk_foo(_hw) container_of(_hw, struct clk_foo, hw) //ptr,type,member，第一个参数一定要是个指针

int clk_foo_enable(struct clk_hw *hw)
{
	struct clk_foo *foo;

	foo = to_clk_foo(hw);

	... perform magic on foo ...

	return 0;
};
```

以下是时钟硬件特性表，其中"y"表示强制需要，"n"表示无效或不必要，“空”表示可选，可根据具体情况进行参考。

|                  | gate | change rate | single parent | multiplexer | root |
| ---------------- | ---- | ----------- | ------------- | ----------- | ---- |
| .prepare         |      |             |               |             |      |
| .unprepare       |      |             |               |             |      |
| .enable          | y    |             |               |             |      |
| .disable         | y    |             |               |             |      |
| .is_enabled      | y    |             |               |             |      |
| .recalc_rate     |      | y           |               |             |      |
| .round_rate      |      | 与下二选一  |               |             |      |
| .determine_rate  |      | 与上二选一  |               |             |      |
| .set_rate        |      | y           |               |             |      |
| .set_parent      |      |             | n             | y           | n    |
| .get_parent      |      |             | n             | y           | n    |
| .recalc_accuracy |      |             |               |             |      |
| .init            |      |             |               |             |      |

最后，通过以下函数注册时钟，该函数只需要传递clk_foo结构体变量，该函数会自动向框架进行注册。

```c
clk_register(...)
```

可参阅 ``drivers/clk/clk-*.c`` 中基本类型时钟例子.

### 5. 禁用未使用时钟的选通

如果驱动程序没有正确的开启时钟，但bootloader已经启动了，可通过设置bypass功能维持时钟功能，需要向bootargs参数中写入`clk_ignore_unused`。

### 6. 锁

该时钟框架包含prepare锁和enable锁两种锁。

enable锁是一种自旋锁，因此这些操作不允许休眠，在.enable、.disable和.is_enabled操作调用中使用，并且在原子上下文中允许调用clk_enable()、clk_disable()和clk_is_enabled() API。prepare锁是一种互斥锁，在其他的所有操作中使用，这些操作都允许休眠，并且在原子上下文中不允许调用API。通过锁的类型可将操作分为两组。

驱动程序无需关注同一组操作之间的公共资源，即使这些资源被多个时钟锁共享，但对于多组操作之间的共享就需要受到驱动程序的保护，如时钟的速率调整以及状态的改变。

该时钟框架是可重入的，可能出现一个时钟.set_rate调用另一个时钟的.set_rate的情况，这需要驱动程序去注意这种情况并控制代码流。

特别注意的是，当时钟框架之外的代码需要进行时钟相关操作时，也必须考虑使用锁。