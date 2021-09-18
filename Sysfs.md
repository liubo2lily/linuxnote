## Reference

* Documentation/filesystems/sysfs.txt
* Documentation/kobject.txt
* samples/kobject/{kobject-example.c,kset-example.c}

## 1. Kobject

kobject是一种具有引用计数、uevent、上锁、层级嵌套等功能的顶层抽象类，常用于sysfs中，该结构主要由kset结构，kobj_type结构共同实现，该结构禁止在栈中生成。

#### 1.1 初始化及注册kobject

```c
void kobject_init(struct kobject *kobj, struct kobj_type *ktype);
/* 在注册前，一定要分配kset成员，如果kset还和其他kobject有关联，则需要将parent成员置NULL，注册后，parent将会是kset本身 */
int kobject_add(struct kobject *kobj, struct kobject *parent, const char *fmt, ...);
//将对象加入Linux设备层次。挂接该对象到kset的list链中，增加父目录各级的引用计数，在其parent指向的目录下创建文件节点，并启动该类型内核对象的hotplug函数。
int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype,
                             struct kobject *parent, const char *fmt, ...);	//该函数一气呵成
```

#### 1.2 kobject的名字

```c
//kobject的名称最好不要修改。如果一定要，则调用下列API，该操作
int kobject_rename(struct kobject *kobj, const char *new_name);
const char *kobject_name(const struct kobject * kobj);	//get name
```

#### 1.3 uevent

kobject_uevent会自动生成并发送相应的uevent

```c
int kobject_uevent(struct kobject *kobj, enum kobject_action action);
```

* action取KOBJ_ADD，公布添加，但是一定要确定所有初始化都完成之后才能做。
* action取KOBJ_REMOVE，公布移除

#### 1.4 引用计数

引用计数是正在引用该对象的个数，如果只想使用引用功能，那么直接使用`Documentation/kref.txt`即可，注意，在生成kobject过程中也会造成引用自增，因此需要手动释放。

```c
struct kobject *kobject_get(struct kobject *kobj);
void kobject_put(struct kobject *kobj);
```

#### 1.5 创建一个简单的kobject

如果想要创建一个无kset，show，store等功能的kobject(sysfs中的简单目录)，可以使用以下方法，该函数可在指定kobject节点下创建kobject

```c
struct kobject *kobject_create_and_add(char *name, struct kobject *parent);
//想要创建简单的属性，可使用以下函数
int sysfs_create_file(struct kobject *kobj, struct attribute *attr);
int sysfs_create_group(struct kobject *kobj, struct attribute_group *grp);
```

#### 1.6 struct kobj_type以及release的方法

由于计数是自动完成的，因此在释放时，因此在释放时要通知一下，release是每一个kobject都要有的一个方法，若不包含，则会发出警告，release函数中，绝不能修改kobject的name，release是由kobj_type结构体间接提供的，示例如下：

```c
struct kobj_type {
    void (*release)(struct kobject *kobj);
    const struct sysfs_ops *sysfs_ops;
    struct attribute **default_attrs;
    const struct kobj_ns_type_operations *(*child_ns_type)(struct kobject *kobj);
    const void *(*namespace)(struct kobject *kobj);
}; 
void my_object_release(struct kobject *kobj)
{
    struct my_object *mine = container_of(kobj, struct my_object, kobj);
    /* Perform any additional cleanup on this object, then... */
    kfree(mine);
}
```

#### 1.7 kset

kset顾名思义是kobject的一个集合，属于顶层容器类，其中的元素的ktype最好相同，它提供了以下功能。

* 内核可以使用它跟踪“所有块设备”或“所有PCI设备驱动程序”。
* 可以通过多层嵌套，构建层级结构，sysfs就是这么做的(kobj就是parent)。
* 支持热插拔，向用户空间上报uevent事件。

```c
struct kset {
	struct list_head list;
	spinlock_t list_lock;
	struct kobject kobj;	//所有列表项的父亲
	const struct kset_uevent_ops *uevent_ops;
}
struct kset *kset_create_and_add(const char *name, struct kset_uevent_ops *u, struct kobject *parent);/* 创建 */
//也可以通过，kobject_add()默认生成
void kset_unregister(struct kset *kset);	/* 注销 */
//kset_uevent_ops用于支持uevent操作
struct kset_uevent_ops {
        int (*filter)(struct kset *kset, struct kobject *kobj);
        const char *(*name)(struct kset *kset, struct kobject *kobj);
        int (*uevent)(struct kset *kset, struct kobject *kobj,
                      struct kobj_uevent_env *env);
};
```

* filter函数用于避免指定kobject向用户空间发送uevent。如果函数return 0，则不会发出uevent。
* name函数用于向用户空间覆盖kset的默认名称
* uevent函数将向真正的uevent函数提供更多的环境变量

#### 1.8 kobject的移除

一般调用`kobject_put()`，就将会自动删除，注意KOBJ_ADD和KOBJ_REMOVE uevent总是成对的，假设kobject暂时无法直接被移除，可以使用`kobject_del()`先将其隐身，之后再进行清理。`kobject_del()`可以删除对父节点的引用，以打破循环引用。

## 2. sysfs

sysfs是基于ram的文件系统，它的功能是向用户空间提供内核的一些数据结构、属性以及链接关系。

#### 2.1 目录的创建

每一个kobject都对应于sysfs的一个目录，多层级的kobject关系也就构成了向用户空间展示出的目录层级结构。sysfs中的顶层目录也就是这些对象的共同祖先，也就是子系统。之所以可以实现目录层次，是因为sysfs包含了struct kobject，而struct kobject包含了struct kernfs。

引用计数是通过`sysfs_schedule_callback()`函数操作。

#### 2.2 属性

属性可以导出到kobject，并以文件的形式在文件系统中展示，其提供了相应的文件IO操作去读写属性值。一个属性按道理讲只应有一个值，但可以定义为数组，但绝不要是多个不同类型的变量，属性的定义及操作很简单，如下所示，这些操作也是一个顶层抽象类。

```c
struct attribute {
        char                    * name;
        struct module       *owner;
        umode_t                 mode;
};
int sysfs_create_file(struct kobject * kobj, const struct attribute * attr);
void sysfs_remove_file(struct kobject * kobj, const struct attribute * attr);
```

而在实际应用中，都是继承此结构体，并加入`show()`和`store()`操作，以下是一个实例，源于device_attribute。

```c
struct device_attribute {
    struct attribute    attr;
    ssize_t (*show)(struct device *dev, struct device_attribute *attr,
            char *buf);
    ssize_t (*store)(struct device *dev, struct device_attribute *attr,
             const char *buf, size_t count);
};
int device_create_file(struct device *, const struct device_attribute *);
void device_remove_file(struct device *, const struct device_attribute *);
/* helper */
#define DEVICE_ATTR(_name, _mode, _show, _store) \
struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)
//e.g.
static DEVICE_ATTR(foo, S_IWUSR | S_IRUGO, show_foo, store_foo);
```

#### 2.3 子系统特定的回调

当子系统定义一个新的类型时，就需要实现ktype中的sysfs_ops接口，以间接调用属性中的show、store从而对自身属性进行修改。

```c
struct sysfs_ops {
        ssize_t (*show)(struct kobject *, struct attribute *, char *);
        ssize_t (*store)(struct kobject *, struct attribute *, const char *, size_t);
};
```

一个具体的实例，即device。

```c
#define to_dev(obj) container_of(obj, struct device, kobj)
#define to_dev_attr(_attr) container_of(_attr, struct device_attribute, attr)
static ssize_t dev_attr_show(struct kobject *kobj, struct attribute *attr,
                             char *buf)
{
        struct device_attribute *dev_attr = to_dev_attr(attr);
        struct device *dev = to_dev(kobj);
        ssize_t ret = -EIO;

        if (dev_attr->show)
                ret = dev_attr->show(dev, dev_attr, buf);
        if (ret >= (ssize_t)PAGE_SIZE) {
                print_symbol("dev_attr_show: %s returned bad count\n",
                                (unsigned long)dev_attr->show);
        }
        return ret;
}
```

#### 2.4 属性数据的读写

在声明属性时，`show()`和`store()`也要实现，sysfs会申请一个大小为PAGE_SIZE的buf用于交互，对于device属性也是如此，下面是一个简单的例子。

```c

// show返回的应该是scnprintf()的返回值，即打印到缓冲区的字节数
// 设置要返回到用户空间的值的格式时，show（）不能使用snprintf（）。如果可以保证永远不会发生溢出，则可以使用sprintf（），否则必须使用scnprintf（）。
// buf为一个页的大小

/* 简单实现 */
static ssize_t show_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", dev->name);
}

static ssize_t store_name(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    snprintf(dev->name, sizeof(dev->name), "%.*s",
             (int)min(count, sizeof(dev->name) - 1), buf);
    return count;
}
static DEVICE_ATTR(name, S_IRUGO, show_name, store_name);
```

* 在read(2)中，`show()`方法应该填充整个缓冲区，一个属性应该只导出一个值或一个数组。
* 在pthread(2)中，允许对整个文件的任意位置进行读取，如果偏移量指定为0，则会再次调用`show()`，重新填充。
* 在write(2)中，会在缓冲区的结尾添加NULL字符，这就可以安全的使用`sysfs_streq()`等字符串操作函数。
* `show()`返回的应该是scnprintf()的返回值，即打印到缓冲区的字节数
* `store()`应返回从缓冲区使用的字节数。如果使用了整个缓冲区，只需返回count参数。
* 设置值格式时，必须使用`scnprintf()`。

#### 2.5 sysfs顶层目录

sysfs揭漏了内核数据结构之间的关系`block/ bus/ class/ dev/ devices/ firmware/ net/ fs/`

* `devices/` 包含了文件系统所代表的设备树，是device结构体的层级结构
* `bus/` 包含内核各种总线类型的平面目录布局，每个总线目录都有两个子目录
  * `devices/` ，包含了每个设备的符号链接
  * `drivers/` ，包含了特定总线上已的加载驱动

* `fs/` 包含了一些文件系统，以及文件系统想要输出的信息
* `dev/` 包含两个目录char/和block/。在这两个目录中有名为<major>：<minor>的符号链接。这些符号链接指向具体device。

#### 2.6 sysfs已实现的接口

* devices (include/linux/device.h)

```c
struct device_attribute {
    struct attribute    attr;
    ssize_t (*show)(struct device *dev, struct device_attribute *attr,
            char *buf);
    ssize_t (*store)(struct device *dev, struct device_attribute *attr,
             const char *buf, size_t count);
};
DEVICE_ATTR(_name, _mode, _show, _store);

int device_create_file(struct device *dev, const struct device_attribute * attr);
void device_remove_file(struct device *dev, const struct device_attribute * attr);
```

* bus drivers (include/linux/device.h)

```c
struct bus_attribute {
        struct attribute        attr;
        ssize_t (*show)(struct bus_type *, char * buf);
        ssize_t (*store)(struct bus_type *, const char * buf, size_t count);
};
BUS_ATTR(_name, _mode, _show, _store)

int bus_create_file(struct bus_type *, struct bus_attribute *);
void bus_remove_file(struct bus_type *, struct bus_attribute *);
```

* device drivers (include/linux/device.h)

```c
struct driver_attribute {
        struct attribute        attr;
        ssize_t (*show)(struct device_driver *, char * buf);
        ssize_t (*store)(struct device_driver *, const char * buf,
                         size_t count);
};
DRIVER_ATTR_RO(_name)
DRIVER_ATTR_RW(_name)

int driver_create_file(struct device_driver *, const struct driver_attribute *);
void driver_remove_file(struct device_driver *, const struct driver_attribute *);
```

## 3. sysfs应用案例-基于leds子系统

核心函数

```c
/* 这些函数会自动生成名为dev_attr_name的struct device_attribute */
/* 并且自动包含名为name_show、name_store等函数，根据后缀自行理解 */
#define DEVICE_ATTR_RW(name);
#define DEVICE_ATTR_RO(name);
#define DEVICE_ATTR_WO(name);
#define DEVICE_ATTR(name, mode, show, store);
```

大致流程

```c
/* 1. 定义属性，归为组，归为组的数组 */
static ssize_t brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", "test");
}
static ssize_t brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long state;
	ssize_t ret;
	ret = kstrtoul(buf, 10, &state);	/* 进制数 */
	ret = size;
	pr_info("receive %lu\n", state);
	return ret;
}
static DEVICE_ATTR_RW(brightness);
static struct attribute *led_class_attrs[] = {
	&dev_attr_brightness.attr,
	NULL,
};
static const struct attribute_group led_group = {
	.attrs = led_class_attrs,
};
static const struct attribute_group *led_groups[] = {
	&led_group,
	NULL,
};
/* 2. 调用class_create, 在sys/class下创建目录 */
// in global
static struct class *leds_class;
// in init function
static int __init leds_init(void)
{
    leds_class = class_create(THIS_MODULE, "leds");
    if (IS_ERR(leds_class))
        return PTR_ERR(leds_class);
    leds_class->dev_groups = led_groups;
    return 0;
}

```





