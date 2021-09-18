## 0. Header

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/platform_device.h>
```

## 1. Common Functions

#### 1.1 IO Remap

```c
void __iomem *ioremap(resource_size_t res_cookie, size_t size);
void iounmap(volatile void __iomem *cookie);
void __iomem *devm_ioremap(struct device *dev, resource_size_t offset, resource_size_t size);
```

#### 1.2 IO Read or Write

```c
void writel(u32 value, volatile void __iomem *addr);	//b w l 8 16 32
u32 readl(const volatile void __iomem *addr);	//b w l 8 16 32
```

#### 1.3 Bit Set or Clear

```c
set_bit(nr,p);
clear_bit(nr,p);
change_bit(nr,p);/* bit reverse */
```

#### 1.4 Request Memory

```c
void * devm_kzalloc(struct device *dev, size_t size, gfp_t gfp); /* z means memset 0 */
void *vzalloc(unsigned long size);	/* v means discontinuous, used only request for huge of memory and device independent*/
void vfree(const void * addr);
// gfp is usually as GFP_KERNEL, but when in Interrupt context or spin lock, should as GFP_ATOMIC
```

#### 1.5 Memory Conversion

```c
/* Only applicable to kernel memory and DMA memory */
unsigned long virt_to_phys(volatile void *address);
void *phys_to_virt(unsigned long address);
```

## 2. Frame-Platform Bus

 [Frame Reference](code\char_device\frame.c) 

* muti-minor support

```c
struct cdev xxx_cdev;
static int __init xxx_init(void)
{
    int rc;
    dev_t devid;
    int major;
    /* exist major: register_chrdev_region else: alloc_chrdev_region */
    rc = alloc_chrdev_region(&devid, 0, 1, "devname");	/* assign cdev */
    major = MAJOR(devid);
    cdev_init(&xxx_cdev, &xxx_fpos);	/* init cdev */
    cdev_add(&xxx_cdev, devid, 1);		/* register cdev */
}
static void __exit xxx_exit(void)
{
    cdev_del(&xxx_cdev);
    unregister_chrdev_region(MKDEV(major,0), 1);
}
```

## 3. file operations

```c
static struct file_operations xxx_fops = {
	.owner	 = THIS_MODULE,
	//...
};
```

#### 3.1 open and release

* Kernel

```c
static int xxx_open(struct inode *inode, struct file *file)
{
    return 0;
}
static int xxx_release(struct inode *inode, struct file *file)
{
    return 0;
}
```

* User

```c
int fd;
fd = open("devpath", O_RDWR);
close(fd);
```

#### 3.2 write

* Kernel

```c
static ssize_t xxx_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long err;
	struct xxx xxx;
	err = copy_from_user(&xxx, buf, sizeof(struct xxx));
	return err;
}
```

* User

```c
write(fd, &data, size);
/* return the number of bytes write or -1 error */
```

#### 3.3 read

* Kernel

```c
static ssize_t xxx_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long err;
    struct xxx xxx;
    err = copy_to_user(buf, &xxx, sizeof(struct xxx));
    return err;
}
```

* User

```c
read(fd, &data, size);
/* return the number of bytes read or -1 error */
```

#### 3.4 unlocked_ioctl

* Kernel

```c
#define CMD_WRITE	    _IOW('p', 0x01, struct xxx)
#define CMD_READ	    _IOR('p', 0x02, struct xxx)
#define CMD_CTRL		_IO('p', 0x03)	
static long xxx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
		case CMD_WRITE:
		{
		    struct xxx xxx;
		    if (copy_from_user(&xxx, (struct xxx*)arg, sizeof(struct xxx)))
				return -EFAULT;
		    return 0;
		}
        case CMD_READ:
		{
		    struct xxx xxx;
		    if (copy_to_user((struct xxx*)arg, &xxx, sizeof(struct xxx)))
				return -EFAULT;
		    return 0;
		}
		case CMD_CTRL:
		{
		    //control operations...
		    return 0;
		}
		default:
			return -EINVAL;
   }
}
```

* User

```c
#define CMD_WRITE	    _IOW('p', 0x01, struct xxx)
#define CMD_READ	    _IOR('p', 0x02, struct xxx)
#define CMD_CTRL		_IO('p', 0x03)	

struct xxx ws;
struct xxx rs;
ioctl(fd, CMD_WRITE, &ws);
ioctl(fd, CMD_READ, &rs);
ioctl(fd, CMD_CTRL);
```

#### 3.5 poll

* Kernel

```c
static DECLARE_WAIT_QUEUE_HEAD(read_wait);
static DECLARE_WAIT_QUEUE_HEAD(write_wait);
static unsigned int xxx_poll(struct file *fp, poll_table * wait)
{
    unsigned int mask = 0;
	poll_wait(fp, &read_wait, wait);
	poll_wait(fp, &write_wait, wait);
    if (...)		/* readable */
        mask |= (POLLIN | POLLRDNORM);
    if (...)		/* writeable */
        mask |= (POLLOUT | POLLWRNORM);
    return mask;
}
/* in ISR */
wake_up_interruptible(&read_wait); /* || */ wake_up_interruptible(&write_wait);
```

* User

```c
int ret;
struct pollfd fds[1];
fds[0].fd = fd;
fds[0].events = POLLIN;
ret = poll(fds,1, 5000);
if(ret == 0) {
    //timeout
}
else {
    //read operation
}
```

#### 3.6 fasync

* Kernel

```c
struct fasync_struct *xxx_fasync;
static int xxx_fasync(int fd, struct file *file, int on)
{
	if (fasync_helper(fd, file, on, &xxx_fasync) >= 0)
		return 0;
	else
		return -EIO;
}
/* in ISR */
kill_fasync(&p_fasync, SIGIO, POLL_IN);	/*Or POLL_OUT*/
```

* User

```c
static void sig_handler(int sig)
{
    //...
}
/* in main or other */
int	flags;
signal(SIGIO, sig_handler);
fcntl(fd, F_SETOWN, getpid());
flags = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, flags | FASYNC);
```

#### 3.7 mmap

* Kernel

```c
static char *kernel_buf;
static int bufsiz = 1024*8;
static int xxx_drv_mmap(struct file *file, struct vm_area_struct *vma)
{
	/* get physic address */
	unsigned long phy = virt_to_phys(kernel_buf);
	/* set property: cache, buffer */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	/* map */
	if (remap_pfn_range(vma, vma->vm_start, phy >> PAGE_SHIFT,		/* divide page size(0x1000) */
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		printk("mmap remap_pfn_range failed\n");
		return -ENOBUFS;
	}
	return 0;
}
/* In probe */
kernel_buf = devm_kzalloc(&pdev->dev, bufsiz, GFP_KERNEL);
```

* User

```c
static char *buf;
static int bufsiz = 1024*8;
/* In functions */
buf =  mmap(NULL, bufsiz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (buf == MAP_FAILED) {
    return -1;
}
munmap(buf, bufsiz);
```

## 4. Sleep and Wakeup

```c
static DECLARE_WAIT_QUEUE_HEAD(xxx_wait);
static int event = 0;
/* in function such as drv_read */
wait_event_interruptible(xxx_wait, event);
// normal operations
// change event value

/* in ISR */
// change event value
wake_up_interruptible(&xxx_wait);
```

## 5. Block and Non-block

* Kernel

```c
int flags = file->f_flags;
int is_nonblock = f_flags & O_NONBLOCK;
/* Implement code according to is_nonblock  */
```

* User

```c
fd = open(argv[1], O_RDWR | O_NONBLOCK);	/* Use from the beginning,or */
flags = fcntl(fd, F_GETFL);					/* Use in code */
fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);	/* Block */
fcntl(fd, F_SETFL, flags | O_NONBLOCK);		/* Non-block */
```

## 6. Concurrency Control

* SMP-Symmetric Multi-Processors, UP-Uni-Processor(Divided by ARMv6)

* ldrex r0, [r1] #r1 marked as exclusive
* strex r2, r0, [r1] #r2 returns whether the opeation was successful

#### 6.1 Atomic variable

```c
static atomic_t valid = ATOMIC_INIT(1);
static ssize_t xxx_open (struct inode *node, struct file *file)
{
    if (atomic_dec_and_test(&valid)) {
        return 0;
    }
    atomic_inc(&valid);
    return -EBUSY;
}
static int xxx_release (struct inode *node, struct file *file)
{
    atomic_inc(&valid);
    return 0;
}
```

#### 6.2 Spin Lock

```c
void spin_lock(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
int spin_is_locked(spinlock_t *lock);
/* _bh(Interrupt lower half), _irq, _irqsave/_irqrestore(save irq status)*/
/* for example */
static DEFINE_SPINLOCK(lock);
spin_lock_bh(&lock);
//critical code
spin_unlock_bh(&lock);
```

#### 6.3 Semaphore

```c
void down(struct semaphore *sem); /* _interruptible _killable(fatal signal) _trylock #return 0 means success */
int down_timeout(struct semaphore *sem, long jiffies);
void up(struct semaphore *sem);	/* release semaphore */
/* for example */
static DEFINE_SPINLOCK(clock_lock);
if (down_interruptible(&sem)) {
    return -ERESTARTSYS;
}
/* normal operations */
up(&sem);
```

#### 6.4 mutex

```c
int mutex_is_locked(struct mutex *lock); /* 1 means locked */
void mutex_lock(struct mutex *lock); /* _interruptible _killable(fatal signal) _trylock #return 0 means success */
void mutex_unlock(struct mutex *lock);	/* wake up other thread waiting for this mutex */
int atomic_dec_and_mutex_lock(atomic_t *cnt, struct mutex *lock); /* if cnt-- == 0, get mutex */
/* for example */
static DEFINE_MUTEX(mutex);
mutex_lock(&mutex);
//critical code
mutex_unlock(&mutex);
```

