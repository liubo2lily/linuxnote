[TOC]

# 0 前言

+ **驱动中有关设备树的函数为啥都以of开头？**

  答：因为设备树的起源就是OpenFirmware，简称OF。

******

# 1 设备树基本结构

## 1.1 节点结构

```shell
[label:] node-name[@unit-address] {
	[properties definitions]
	[child nodes]
};
```

其中label表示节点标签，方便其他节点将其引用或后期对节点进行修改，用&label实现 (若无标签，则只能引用路径了)；

&unit-address用于标注节点地址。

******

## 1.2 常用Property（属性）

|     属性名     |                             功能                             |
| :------------: | :----------------------------------------------------------: |
| #address-cells |                   address占用多少个32位数                    |
|  #size-cells   |                     size占用多少个32位数                     |
|   compatible   |         对应驱动程序中的compatible用于绑定设备与驱动         |
|     model      |            与compatible类似，可以具体指定兼容设备            |
|     status     | 定义设备状态，"okay"-正常运行, "disabled"-临时关闭,"fail"-发生严重错误,需要修复, "fail-sss"-跟错误信息 |
|      reg       |   用于描述一个寄存器或一段空间，格式ret = <address, size>    |

******

## 1.3 常用节点

**chosen节点**

可以通过dts向内核传入一些参数, 可在节点中设置bootargs属性

```shell
chosen {
	bootargs = "noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200";
};
```

**memory节点**

定义具体板子使用的内存基地址和大小

```shell
memory {
	reg = <0x80000000 0x20000000>;
};
```

**aliases节点**

给其他的节点起个别名

```shell
aliases {
	i2c0 = &xxxi2c2;
	i2c1 = &xxxi2c0;
};
```

*****

# 2 相关常用函数

********

## 2.1 查找结点

+ **of_find_node_by_path		//根据路径查找结点**

```c
struct device_node *of_find_node_by_path(const char *path);
```

参数path取值"/"对应根节点，"/memory"对应memory节点，以此类推。

+ **of_find_compatible_node	//根据compatible找到节点**

```c
struct device_node *of_find_compatible_node(struct device_node *from, const char *type, const char *compat);
```

参数from表示从哪个节点开始找，传入NULL则为根节点；参数type一般取NULL（一般无type属性）

+ **of_get_parent		//找到节点的父节点**

```c
struct device_node *of_get_parent(const struct device_node *node);
```

注：调用该函数得到parent必须调用`of_node_get(parent)`以减少parent节点引用次数。

+ **of_get_next_parent		//找到节点的父节点**

```c
struct device_node *of_get_next_parent(struct device_node *node);
```

注：与of_get_parent不同的是，调用该函数无需调用`of_node_get(parent)`

+ **of_get_next_child		//取出上一个子节点**

```c
struct device_node *of_get_next_child(const struct device_node *node, struct device_node *prev);
```

参数prev表示上一个子节点，传入NULL默认第一个子节点。

+ **of_get_next_available_child	//取出上一个可用子节点**

```c
struct device_node *of_get_next_available_child(const struct device_node *node, struct device_node *prev);
```

与of_get_next_child相比跳过了"disabled"状态节点。

********

## 2.2 查找某个属性

+ **of_find_property	//找到节点中的属性**

```c
struct property *of_find_property(const struct device_node *np, const char *name, int *lenp);
```

参数name代表属性名；参数lenp保存属性值的长度

*******

## 2.3 获取属性的值

+ **of_get_property	//根据名字找到节点的属性，并返回它的值**

```c
const void *of_get_property(const struct device_node *np, const char *name, int *lenp);
```

参数name代表属性名；参数lenp保存属性值的长度

+ **of_property_count_elems_of_size	//获取节点中元素的个数**

```c
int of_property_count_elems_of_size(const struct device_node *np, const char *propname, int elem_size);
```

参数propname表示需要统计的属性名；参数else_size代表元素的长度

+ **of_property_read_u32/u64	//读取u32/u64整数**

```c
int of_property_read_u32(const struct device_node *np, const char *propname, u32 *out_value);
int of_property_read_u64(const struct device_node *np, const char *propname, u64 *out_value);
```

参数propname表示需要读取的属性名；参数out_value保存读取的值

+ **of_property_read_u32	//读取指定位置u32**

```c
int of_property_read_u32_index(const struct device_node *np, const char *propname, u32 index, 
                               u32 *out_value);
```

参数propname表示需要读取的属性名；参数out_value保存读取的值；参数index代表读取位置

+ **of_property_read_variable_u8/u16/u32/u64_array	//读取数组**

```c
int of_property_read_variable_u8_array(const struct device_node *np,
                    const char *propname, u8 *out_values, size_t sz_min, size_t sz_max);
int of_property_read_variable_u16_array(const struct device_node *np,
                    const char *propname, u16 *out_values, size_t sz_min, size_t sz_max);
int of_property_read_variable_u32_array(const struct device_node *np,
                    const char *propname, u32 *out_values, size_t sz_min, size_t sz_max);
int of_property_read_variable_u64_array(const struct device_node *np,
                    const char *propname, u64 *out_values, size_t sz_min, size_t sz_max);
```

参数sz_min与sz_max用以限制值的长度，如果不满足两者，则一个数都读不到

+ **of_property_read_string	//读取字符串**

```c
int of_property_read_string(const struct device_node *np, const char *propname, const char **out_string);
```

参数propname表示需要读取的属性名；参数out_string保存读取的字符串

****

# 3 如何在驱动中获取设备节点

```c
struct device_node *node = pdev->dev.of_node;
```

pdev的dev成员中的of_node成员就代表了设备节点

******

# 4 GPIO设备树相关

+ **of_gpio_named_count	//根据名字获取节点定义的gpio个数**

```c
int of_gpio_named_count(struct device_node *np, const char* propname);
```

+ **of_gpio_count	//获取节点定义的gpio个数**

```c
int of_gpio_count(struct device_node *np);
```

使用此函数，那么定义gpio属性一定要以gpios来命名

+ **of_get_gpio_flags	//获取gpio的flags**

```c
int of_get_gpio_flags(struct device_node *np, int index, enum of_gpio_flags *flags);
```

flags取值`OF_GPIO_ACTIVE_LOW` or `OF_GPIO_SINGLE_ENDED`

+ **of_get_named_gpio	//根据名字获取GPIO资源号**

```c
int of_get_named_gpio(struct device_node *np, const char *propname, int index);
```

使用此函数，那么定义gpio属性一定要以gpios来命名

+ **of_get_gpio	//获取GPIO资源号**

```c
int of_get_gpio(struct device_node *np, int index);
```

使用此函数，那么定义gpio属性一定要以gpios来命名

*****

# 5 设备树中快速找到设备节点

```shell
dtc -I dtb -O dts xxx.dtb > target.dts
```

