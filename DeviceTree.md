# 设备树相关常用函数

********

## 查找结点

**a. of_find_node_by_path		//根据路径查找结点**

```c
struct device_node *of_find_node_by_path(const char *path)
```

参数path取值"/"对应根节点，"/memory"对应memory节点，以此类推。

**b. of_find_compatible_node	//根据compatible找到节点**

```c
struct device_node *of_find_compatible_node(struct device_node *from, const char *type, const char *compat)
```

参数from表示从哪个节点开始找，传入NULL则为根节点；参数type一般取NULL（一般无type属性）

**c. of_get_parent		//找到节点的父节点**

```c
struct device_node *of_get_parent(const struct device_node *node)
```

注：调用该函数得到parent必须调用`of_node_get(parent)`以减少parent节点引用次数。

**d. of_get_next_parent		//找到节点的父节点**

```c
struct device_node *of_get_next_parent(struct device_node *node)
```

注：与c.不同的是，调用该函数无需调用`of_node_get(parent)`

**e. of_get_next_child		//取出上一个子节点**

```c
struct device_node *of_get_next_child(const struct device_node *node, struct device_node *prev)
```

参数prev表示上一个子节点，传入NULL默认第一个子节点。

**f. of_get_next_available_child	//取出上一个可用子节点**

```c
struct device_node *of_get_next_available_child(const struct device_node *node, struct device_node *prev)
```

与e.相比跳过了"disabled"状态节点。

********

## 查找某个属性

**a. of_find_property	//找到节点中的属性**

```c
struct property *of_find_property(const struct device_node *np, const char *name, int *lenp)
```

参数name代表属性名；参数lenp保存属性值的长度

*******

## 获取属性的值

**a. of_get_property	//根据名字找到节点的属性，并返回它的值**

```c
const void *of_get_property(const struct device_node *np, const char *name, int *lenp)
```

参数name代表属性名；参数lenp保存属性值的长度

**b. of_property_count_elems_of_size	//获取节点中元素的个数**

```c
int of_property_count_elems_of_size(const struct device_node *np, const char *propname, int elem_size)
```

参数propname表示需要统计的属性名；参数else_size代表元素的长度

**c. of_property_read_u32/u64	//读取u32/u64整数**

```c
int of_property_read_u32(const struct device_node *np, const char *propname, u32 *out_value)
int of_property_read_u64(const struct device_node *np, const char *propname, u64 *out_value)
```

参数propname表示需要读取的属性名；参数out_value保存读取的值

**d. of_property_read_u32	//读取指定位置u32**

```c
int of_property_read_u32_index(const struct device_node *np, const char *propname, u32 index, 
                               u32 *out_value);
```

参数propname表示需要读取的属性名；参数out_value保存读取的值；参数index代表读取位置

**e. of_property_read_variable_u8/u16/u32/u64_array	//读取数组**

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

**f. of_property_read_string//读取字符串**

```c
int of_property_read_string(const struct device_node *np, const char *propname, const char **out_string);
```

参数propname表示需要读取的属性名；参数out_string保存读取的字符串





