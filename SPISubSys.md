参考资料

* Documentation/spi/spi-summary
* Documentation/spi/spidev

## 1. SPI概念

“Serial Peripheral Interface”（SPI）是一种同步四线串行接口，有主从两种模式，信号线包含SCK(通常约为10 MHz)，以及MOSI、MISO，通信一般工作在0或3模式，并非所有数据位都有用，也并非需要全双工，一些设备除了信号线和片选，还有对主机的中断信号。一般除SPI Flash外，设备间是无法直接通讯的。

SPI只是一个代称，它包括了MicroWire（半双工SPI，用于请求/响应协议）、SSP（“同步串行协议”）、PSP（“可编程串行协议”）以及其他相关协议。其中半双工的引脚会被这样描述SCK、data、nCSx，其中data有时称为MOMI或SISO。

SPI可用来控制MMC、SD卡、Flash等，一些硬件的bios代码就存储在spi flash中，其应用十分广泛，在片内无SPI控制器时，可以用GPIO模拟，但SPI不支持热插拔，如果看中了SPI的低引脚数，又想要热插拔功能的话，不妨直接用USB。

* 时钟模式

根据时钟的极性以及相位，可将时钟分为四种模式，何为极性和相位。

CPOL(polarity)表示极性，取0意味着时钟从low开始，也就是前沿上升，后沿下降，取1与之相反。

CPHA(phase)表示相位，即时钟的哪个边沿开始采集数据，取0为前沿，否则为后沿。

需要注意的是，若CPHA=0，则片选引脚应该提前生效(至少半周期)。而根据CPOL和CPHA，就可以编排模式号。

由于一些工艺或习惯的影响，SPI设备往往只在上升沿读写数据，因此工作模式也就工作在了0或3模式。

SPI请求涉及I/O队列。设备的请求总是以FIFO顺序执行，并通过回调异步完成。linux还包含了一些同步操作的简单封装，比如查看设备是否有响应。在内核中，提供了两种SPI驱动，controller驱动和protocol驱动，其中controller支持主从。`struct spi_device`封装了这两种驱动程序之间的控制器端接口。

## 2. SPI的sysfs接口

其中CTLR代表控制模式，spi_master或spi_slave，B(Bus)，C(Chip)，D(Driver)

| 位置                                 | 描述                                              |
| ------------------------------------ | ------------------------------------------------- |
| /sys/devices/.../CTLR                | 给定SPI控制器的物理节点                           |
| /sys/devices/.../CTLR/spiB.C         | 总线“B”上的spi_device，芯片选择C，通过CTLR访问。  |
| /sys/bus/spi/devices/spiB.C          | spiB.C的符号链接                                  |
| /sys/devices/../CTLR/spiB.C/modalias | 表示该设备应该使用的驱动                          |
| /sys/bus/spi/drivers/D               | 多个spi的驱动程序                                 |
| /sys/class/spi_master/spiB           | 符号链接（或实际设备节点），管理一些总线状态      |
| /sys/devices/../CTLR/slave           | 用于注册(注销)驱动，写空注销，写入驱动名，则注册  |
| /sys/class/spi_slave/spiB            | 符号链接（或实际设备节点）,管理从控制器的一些状态 |

## 3. 编写protocol驱动

SPI protocol驱动类似于platform驱动：

```c
static struct spi_driver CHIP_driver = {
    .driver = {
        .name       = "CHIP",
        .owner      = THIS_MODULE,
        .pm     = &CHIP_pm_ops,
    },
    .probe      = CHIP_probe,
    .remove     = CHIP_remove,
};
```

驱动核心层将自动尝试将此驱动程序绑定到任何SPI设备，一般来说probe遵守一下来成

```c
static int CHIP_probe(struct spi_device *spi)
{
    struct CHIP         *chip;
    struct CHIP_platform_data   *pdata;
    /* assuming the driver requires board-specific data: */
    pdata = &spi->dev.platform_data;
    if (!pdata)
        return -ENODEV;
    /* get memory for driver's per-chip state */
    chip = kzalloc(sizeof *chip, GFP_KERNEL);
    if (!chip)
        return -ENOMEM;
    spi_set_drvdata(spi, chip);  
    /* ... etc */
    return 0;
}
```

一旦进入probe()，驱动就可用`struct spi_message`向SPI设备发出I/O请求。

* spi_message用于处理一系列原子序列，包括：
  * 当双向读写开始时，如何安排spi_transfer请求的顺序；
  * 使用了哪些I/O缓冲区。spi_transfer为每个传输方向分配一个缓冲区，若单向，则一个为空；
  * 可选择定义传输后的短延迟。使用spi_transfer.delay_usecs设置；
  * 在传输和任何延迟后，片选是否失能。通过使用spi_transfer.cs_change标志；
  * 若下一条消息可能会发送到同一设备，可通过spi_transfer.cs_change标志，减少片选切换。
* 可使用spi_message.is_dma_mapped告诉控制器驱动，已经自提供了dma地址。
* spi_async()。异步请求可以在任何上下文（irq处理程序、任务等）中发出，并使用消息提供的回调。若发生错误，则取消片选并终止处理。
* spi_sync()，以及像spi_read()、spi_write()和spi_write_then_read()这样的封装。可在休眠的上下文中发布。
* spi_write_then_read()，用于支持常见的RPC请求，如spi_w8r16()是它的进一步封装。

一些驱动程序可能要修改spi_device的属性，如传输模式、字大小或时钟速率。这是通过spi_setup()完成的，在对设备进行第一次I/O之前，通常会从probe()调用spi_setup()。但也可以在没有消息挂起的任何时候调用。

* 可以使用spi_message_alloc()和spi_message_free()，通过多次传输来分配和初始化spi_message。

## 4. 编写SPI Master Controller驱动

驱动程序的主要任务是提供`spi_master`。使用`spi_alloc_master()`分配，使用`spi_master_get_devdata()`获取设备驱动私有数据。

```c
struct spi_master   *master;
struct CONTROLLER   *c;
master = spi_alloc_master(dev, sizeof *c);
if (!master)
    return -ENODEV;
c = spi_master_get_devdata(master);
```

初始化`spi_master`的字段，包括总线号，方法等。它还将初始化自己的内部状态。初始化`spi_master`后，使用`spi_register_master()`将其注册。核心层会自动完成绑定操作。`spi_unregister_master()`用于注销。其中总线号一般与具体外设一致，若赋值为-1则自动分配。

* `master->setup(struct spi_device *spi)`

设置设备时钟速率、SPI模式和字大小。

* `master->cleanup(struct spi_device *spi)`

可用`spi_device.controller_state`来保持互动。如果这样做，请确保提供`cleanup()`方法来释放该状态。

* `master->prepare_transfer_hardware(struct spi_master *master)`

这将由队列机制调用，以向驱动程序发出消息即将到来的信号，因此子系统通过发出此调用请求驱动程序准备硬件传输。

* `master->unprepare_transfer_hardware(struct spi_master *master)`

这将由队列机制调用，以向驱动程序发出信号，表明队列中不再有待处理的消息。

* `master->transfer_one_message(struct spi_master *master, struct spi_message *mesg)`

子系统调用驱动程序传输单个消息，对同时到达的传输进行排队。完成此消息时，它必须调用`spi_finalize_current_message()`，以便发出下一条消息。

* `master->transfer_one(struct spi_master *master, struct spi_device *spi, struct spi_transfer *transfer)`

子系统调用驱动程序传输单个传输，对同时到达的传输进行排队。完成此消息时，它必须调用`spi_finalize_current_message()`，以便发出下一条消息。

以上两个操作是互斥的，返回值，负数表示失败，0成功，1正在传输。

## 5. 用户层

SPI设备的用户层API有限，支持对SPI从设备的基本半双工`read()`和`write()`访问。使用`ioctl()`请求，还可以进行全双工传输和参数配置。将SPI协议层开发在用户态主要是针对调试场景，因为用户空间开发，程序崩溃无需重启，加载灵活，适合频繁改动，但用户空间无法使用中断等接口。

```c
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
```

#### 5.1 设备创建与驱动程序的绑定

可以在设备树中直接包含，`modalias`条目为`spidev`。执行此操作时，SPI设备的sysfs节点将包括一个子设备节点

`/dev/spidev.C`字符设备，主设备号为153和次设备号动态分配。

`/sys/devices/../spiB.C`通常，SPI设备节点将是其SPI主控制器的子节点。

`/sys/class/spidev/spidev.C`当`spidev`驱动程序绑定到该设备时创建。

#### 5.2 基本的字符设备API

`read()`和`write()`是半双工的，在这些操作的之间，片选取消。使用`SPI_IOC_MESSAGE(N) `请求，可以进行全双工操作而无需改变片选。

`ioctl()`用于参数的设置读取：

| CMD                        | 说明                                               |
| -------------------------- | -------------------------------------------------- |
| `SPI_IOC_RD_MODE`          | (1btye)模式配置，SPI_MODE_0..3                     |
| `SPI_IOC_RD_MODE_32`       | (4btyes)更完整的模式配置                           |
| `SPI_IOC_RD_LSB_FIRST`     | (1btye)0表示MSB                                    |
| `SPI_IOC_RD_BITS_PER_WORD` | (1btye)位宽，0表示8位                              |
| `SPI_IOC_RD_MAX_SPEED_HZ`  | (4bytes)最大传输速率，这还取决于controller中的设置 |

* 无法验证设备的存在性。
* 不支持异步I/O。
* 无法知道实际的传输速率。
* 每个I/O请求传输到SPI设备的字节数默认为一页，但可以更改。

#### 5.3 全双工字符设备API

请参阅`spidev_fdx.c`(但它不执行全双工传输)，该模型与内核`spi_sync()`相同；该示例显示了一条半双工RPC的请求和响应消息，若要全双工传输，要为同一传输提供`rx_buf`和`tx_buf`他们可以相同。

