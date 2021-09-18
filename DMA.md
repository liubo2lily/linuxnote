参考资料

* /Documentation/devicetree/bindings/dma/snps-dma.txt
* /Documentation/devicetree/bindings/dma/dma.txt
* /Documentation/dmaengine/client.txt
* /Documentation/dmaengine/provider.txt

## 1. DMAengine controller

#### 1.1 硬件介绍

大多数Slave DMA控制器具有相同的一般操作原理。

他们有一定数量的信道用于DMA传输，以及一定数量的请求线路。

请求和通道几乎是正交的。通道可用于为任何请求提供服务。为了简化，通道是进行复制的实体，并请求涉及到的端点。

request line实际上对应于从DMA设备与主控器的物理连线。每当设备想要开始传输时，它都会通过assert该请求行来assert DMA请求（DRQ）。

一个非常简单的DMA控制器只考虑一个参数：传输大小。在每个时钟周期，它将一个字节的数据从一个缓冲区传输到另一个缓冲区，直到达到传输大小。

这在现实世界中不会很好地工作，因为从设备可能需要在单个周期中传输特定数量的位。例如，在执行简单的内存复制操作时，我们可能希望传输物理总线允许的尽可能多的数据，以最大限度地提高性能，但我们的音频设备可能具有更窄的FIFO，要求一次只写入16或24位数据。这就是为什么大多数（如果不是所有的话）DMA控制器都可以使用一个称为transfer width的参数来调整这一点。

此外，一些DMA控制器，无论何时将RAM用作源或目标，都可以将内存中的读取或写入分组到缓冲区中，因此，与其进行大量小内存访问（这不是真正有效的），不如进行几次更大的传输。这是使用一个名为“burst size”的参数来完成的，该参数定义了在控制器不将传输拆分为更小的子传输的情况下允许执行的单个读/写次数。

我们理论上的DMA控制器将只能进行涉及单个连续数据块的传输。然而，我们通常进行的一些传输是不连续的，并且希望将数据从非连续缓冲区复制到连续缓冲区，这称为scatter-gather。

至少对于mem2dev传输，DMA引擎需要对scatter-gather的支持。所以我们这里只剩下两种情况：要么我们有一个非常简单的DMA控制器不支持它，我们必须在软件中实现它，要么我们有一个更高级的DMA控制器，在硬件中实现scatter-gather。

后者通常使用要传输的块集合进行编程，并且每当传输开始时，控制器将检查该集合，执行我们在那里编程的任何操作。

此集合通常是表或链接列表。然后，您将表的地址及其元素数，或列表的第一项推送到DMA控制器的一个通道，并且每当断言DRQ时，它将遍历集合以知道从何处获取数据。

无论如何，此集合的格式完全取决于您的硬件。每个DMA控制器都需要不同的结构，但对于每个数据块，所有这些控制器都需要至少源地址和目标地址，是否应该增加这些地址，以及我们前面看到的三个参数：突发大小、传输宽度和传输大小。

最后一件事是，通常情况下，从设备在默认情况下不会发出DRQ，只要您愿意使用DMA，就必须首先在从设备驱动程序中启用DRQ。

这些只是一般的内存到内存（也称为mem2mem）或内存到设备（mem2dev）的传输。大多数设备通常支持dmaengine支持的其他类型的传输或内存操作，本文档稍后将详细介绍。

#### 1.2 linux对DMA的支持

从历史上看，DMA控制器驱动程序是使用异步TX API实现的，用于offloads诸如内存复制、XOR、加密等操作，基本上是任何内存到内存的操作。

随着时间的推移，出现了内存到设备传输的需求，dmaengine也得到了扩展。如今，异步TX API作为dmaengine之上的一层编写，并充当客户机。尽管如此，dmaengine在某些情况下还是适应了该API，并做出了一些设计选择以确保其保持兼容。

有关Async TX API的更多信息，请查看documentation/crypto/Async-TX-API.txt中的相关文档文件。

#### 1.3 DMAEngine 注册

##### 1.3.1 struct dma_device初始化

与任何其他内核框架一样，整个DMAEngine注册依赖于驱动程序填充结构体并根据框架进行注册。在我们的例子中，这种结构就是dma_device。

第一件需要做的事就是取分配这个结构体，任何常用的内存分配器都可以，但是还需要初始化一些字段

* `channels`：例如，应使用INIT_LIST_HEAD宏初始化为列表

* `src_addr_widths`：应包含支持的源传输宽度的位掩码

* `dst_addr_widths`：应包含支持的目的传输宽度的位掩码

* `directions`：应包含所支持的slave方向的位掩码（除了mem2mem传输）

* `residue_granularity`：报告给`dma_set_residue`的传输残余的粒度，这可以是
  * Descriptor,您的设备不支持任何类型的残留物报告。框架将只知道特定的事务描述符已经完成。
  
  * Segment,您的设备能够报告已传输的数据块
  * Burst,您的设备能够报告已传输的Burst
  
* `dev`：应该保留指向与当前驱动关联的struct device指针。

##### 1.3.2 支持的处理类型

接下来需要设置设备（和驱动程序）支持的处理类型。

struct dma_device有一个名为cap_mask的字段，该字段保存支持的各种处理类型，使用`dma_cap_set`函数修改此掩码，并根据实际应用使用各种标志。

所有这些功能都在include/linux/dmaengine.h中的`dma_transaction_type enum`中定义

* `DMA_MEMCPY`，设备能够内存到内存拷贝
* `DMA_XOR`，设备能够对内存区域执行异或操作，用于加速XOR密集型任务，如RAID5
* `DMA_XOR_VAL`，该设备能够使用XOR算法对内存缓冲区执行奇偶校验。
* `DMA_PQ`，该设备能够执行RAID6 P+Q计算，P是简单的异或运算，Q是Reed-Solomon算法。
* `DMA_PQ_VAL`，该设备能够使用RAID6P+Q算法对内存缓冲区执行奇偶校验。
* `DMA_INTERRUPT`，该设备能够触发将产生周期性中断的虚拟传输。客户端驱动程序用来注册回调，该回调将通过DMA控制器中断定期调用。
* `DMA_PRIVATE`，这些设备仅支持slave传输，因此不可用于异步传输。
* `DMA_ASYNC_TX`，不能由设备设置，如果需要，将由框架设置
* `DMA_SLAVE`，该设备可以处理设备到内存的传输，包括scatter-gather传输。在mem2mem案例中，我们有两种不同的类型来处理要复制的单个块或它们的集合，而在这里，我们只有一种事务类型来处理这两种类型。如果要传输单个连续内存缓冲区，只需构建一个仅包含一项的scatter列表。
* `DMA_CYCLIC`，该设备可以处理循环传输。CYCLIC传输是块集合将在其自身上循环的传输，最后一项指向第一项。它通常用于音频传输，您需要在一个环形缓冲区上操作，该缓冲区将填充音频数据。
* `DMA_INTERLEAVE`，该设备支持交错传输。这些传输可以将数据从非连续缓冲区传输到非连续缓冲区，而`DMA_SLAVE`可以将数据从非连续数据集传输到连续目标缓冲区。它通常用于2d内容传输，在这种情况下，您需要将一部分未压缩的数据直接传输到显示器以进行打印

这些不同的类型还将影响源地址和目标地址随时间的变化。

指向RAM的地址通常在每次传输后递增（或递减）。在环形缓冲区的情况下，它们可能会循环（DMA_CYCLIC）。指向设备寄存器（如FIFO）的地址通常是固定的。

##### 1.3.3 device的一些操作

我们的struct dma_device还需要一些函数指针来实现实际逻辑，现在我们已经描述了我们能够执行的操作。具体实现哪些函数，取决于定义的处理类型。

* `device_alloc_chan_resources` and `device_free_chan_resources`

每当驱动程序在与该驱动程序关联的通道上第一次/最后一次调用dma_request_channel或dma_release_channel时，就会调用这些函数。他们负责分配/释放所有需要的资源，以便该channel对您的驱动可用。

* `device_prep_dma_*`
  * 这些函数与您以前注册的功能相匹配。
  * 这些函数都获取与正在准备的传输相关的缓冲区或散列表，并且应该从中创建硬件描述符或硬件描述符列表
  * 可以从中断上下文调用这些函数
  * 你可能做的任何分配都应该使用GFP_NOWAIT标志，以避免潜在的睡眠，但也不会耗尽应急池。
  * 驱动程序应在probe time尝试预先分配传输设置期间可能需要的任何内存，以避免对nowait分配器施加太大压力。
  * 它应该返回dma_async_tx_描述符结构的唯一实例，该结构进一步表示此特定传输。
  * 可使用函数`dma_async_tx_descriptor_init`初始化该结构
  * 你也需要在这个结构中设置两个字段
    * flags：TODO它可以由驱动程序本身修改，还是应该始终是参数中传递的标志
    * tx_submit：指向必须实现的函数的指针，该函数应将当前事务描述符推送到挂起队列，等待调用issue_pending。
  * 在此结构中，可以初始化函数指针回调结果，以便通知提交者事务已完成。在前面的代码中使用了函数指针回调。但是，它不为事务提供任何状态，将被弃用。传递到callback_result的定义为dmaengine_result的结果结构有两个字段：
    * result: 这将提供由dmaengine_tx_result定义的传输结果。要么成功，要么出现error condition。
    * residue: 为支持剩余字节的传输提供剩余字节。
* `device_issue_pending`

获取挂起队列中的第一个事务描述符，并开始传输。无论何时完成该传输，它都应该移动到列表中的下一个事务。这个函数可在中断上下文使用

* `device_tx_status`
  * 应该报告给定通道上剩余的字节
  * 应该只关心作为参数传递的事务描述符，而不是给定通道上当前活动的事务描述符
  * tx_state参数可能为NULL
  * 应该使用dma_set_residue来报告它
  * 在循环传输的情况下，应仅考虑当前period。
  * 这个函数可以在中断上下文中调用
* `device_config`
  * 使用作为参数给定的配置重新配置通道
  * 此命令不应同步执行，也不应在任何当前排队的传输上执行，而应仅在后续传输上执行
  * 在本例中，函数将接收一个dma_slave_config结构指针作为参数，该指针将详细说明要使用的配置。
  * 尽管该结构包含一个方向字段，但不推荐使用该字段，而赞成使用指定给prep_*函数的方向参数
  * 此调用仅在从属操作中是必需的。不应为memcpy操作设置或预期为memcpy操作设置此选项。如果驱动程序同时支持这两种操作，那么它应该只对从属操作使用此调用，而不对memcpy操作使用此调用。
* `device_pause`

暂停在这个通道的传输，此命令应在通道上同步运行，立即暂停给定通道的工作

* `device_resume`

继续这个通道的传输，此命令应在通道上同步运行，立即恢复给定通道的工作

* `device_terminate_all`
  * 中止通道上所有挂起和正在进行的传输
  * 对于中止的传输，不应调用完整回调
  * 可以从原子上下文或描述符的完整回调中调用。千万不要休眠。驱动程序必须能够正确处理此问题。
  * 终止可能是异步的。驱动程序不必等待当前活动的传输完全停止。请参阅设备同步。
* `device_synchronize`
  * 必须将通道的终止与当前上下文同步。
  * 必须确保DMA控制器不再访问先前提交的描述符的内存。
  * 必须确保以前提交的描述符的所有完整回调都已完成运行，并且没有计划运行任何回调。
  * 可休眠

#### 1.4 补充

* `dma_run_dependencies`
  * 应在异步TX transfer时调用，在slave传输情况下可忽略。
  * 确保相关操作在标记为完成之前已运行。
* `dma_cookie_t`
  * 它是一个DMA事务ID，将随时间增加。
  * 自从virt-dma的引入将其抽象掉之后，它就不再真正相关了。
* `DMA_CTRL_ACK`
  * 如果清除，则在客户端确认收到之前，即有机会建立任何依赖链之前，提供者无法重用描述符
  * 这可以通过调用async_tx_ack（）进行确认
  * 若设置了，并不意味着描述符可以重用
* `DMA_CTRL_REUSE`
  * 如果设置了，描述符可以在完成后重用。如果设置了此标志，则提供者不应释放它。
  * 应该通过调用dmaengine_desc_set_reuse（）为重用准备描述符，该函数将设置DMA_CTRL_REUSE。
  * dmaengine_desc_set_reuse（）只有在通道支持可重用描述符（如capabilities所示）时才会成功
  * 因此，如果设备驱动程序希望在两次传输之间跳过dma_map_sg（）和dma_unmap_sg（），因为没有使用dma的数据，那么它可以在传输完成后立即重新提交传输。
  * 可以用几种方法释放描述符
  * 通过调用dmaengine_desc_clear_reuse（）并提交上一个txn来清除DMA_CTRL_REUSE
  * 显式调用dmaengine_desc_free（），只有在已设置DMA_CTRL_REUSE时才能成功
  * 终止通道
* `DMA_PREP_CMD`
  * 如果设置了，客户端驱动程序会告诉DMA控制器，DMA API中传递的数据是命令数据。
  * 命令数据的解释是DMA控制器特有的。它可用于向其他外围设备/寄存器读取/寄存器写入发出命令，对于这些外围设备，描述符的格式应与普通数据描述符的格式不同。

#### 1.5 一般设计说明

您将看到的大多数DMAEngine驱动程序都基于类似的设计，该设计在处理程序中处理传输结束中断，但将大部分工作推迟到tasklet，包括在上一次传输结束时启动新的传输。

但是，这是一种效率相当低的设计，因为传输间延迟不仅是中断延迟，而且是tasklet的调度延迟，这将使通道在其间空闲，从而降低全局传输速率。

您应该避免这种做法，而不是在您的tasklet中选择新的传输，而是将该部分移动到中断处理程序以缩短空闲窗口（这是我们无论如何都无法避免的）。

#### 1.6 词汇表

Burst：在刷新到内存之前可以排队到缓冲区的一系列连续读或写操作。

Chunk：一个连续的Burst集合

Transfer：Chunk的集合（无论是否连续）



## 2. client

有关在async_tx中使用dmaengine，请参阅：Documentation/crypto/async-tx-api.txt

下面是设备驱动程序编写者如何使用DMA Engine的Slave-DMA API的指南。这仅适用于slave DMA使用。

我们必须在那里填写的功能，以及因此必须实现的功能，显然取决于您报告为支持的事务类型。

## 设备树

**DMA Controller**

需要的属性

- compatible: "snps,dma-spear1340"

- reg: DMAC 寄存器地址范围

- interrupt: DMAC中断号

- dma-channels: 硬件支持的通道数

- dma-requests: 支持的DMA请求行数，最多16条

- dma-masters: 控制器支持的AHB Master数量

- #dma-cells: 必须是<3>

- chan_allocation_order: 通道分配顺序，0（默认）：升序，1：降序

- chan_priority: 通道的优先级。0（默认）：从通道0->n，1：从通道n->0

- block_size: 控制器支持的最大块大小

- data-width: 每个AHB Master硬件支持的最大数据宽度（字节，2的幂）

```c
dmahost: dma@fc000000 {
    compatible = "snps,dma-spear1340";
    reg = <0xfc000000 0x1000>;
    interrupt-parent = <&vic1>;
    interrupts = <12>;
    dma-channels = <8>;
    dma-requests = <16>;
    dma-masters = <2>;
    #dma-cells = <3>;
    chan_allocation_order = <1>;
    chan_priority = <1>;
    block_size = <0xfff>;
    data-width = <8 8>;
};
```

**DMA Client**

需要的属性

- dmas：一个或多个DMA说明符的列表（1+#dma-cells），每个说明符包括
  - DMA Controller的引用
  - DMA请求行号
  - 用于在分配的通道上传输的Memory Master
  - 用于在分配通道上传输的Peripheral Master

- dma-names：每个DMA的别名

```c
serial@e0000000 {
    compatible = "arm,pl011", "arm,primecell";
    reg = <0xe0000000 0x1000>;
    interrupts = <0 35 0x4>;
    dmas = <&dmahost 12 0 1>,
    <&dmahost 13 1 0>;
    dma-names = "rx", "rx";
};
```

## 怎么设置DMA

* 设置源地址
* 设置目的地址
* 设置搬运长度
* 其他参数（地址递增顺序，触发方式，一次连续搬多少个数据）
* 启动DMA
* DMA传输完，会产生DMA中断





