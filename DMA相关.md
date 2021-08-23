参考资料

* /Documentation/devicetree/bindings/dma/snps-dma.txt
* /Documentation/devicetree/bindings/dma/dma.txt
* http://www.wowotech.net/linux_kenrel/dma_engine_overview.html
* http://www.wowotech.net/linux_kenrel/dma_controller_driver.html

Q&A

1.dma怎么加进去的？

驱动子系统做好了封装，

dma_request_slave_channel_compat	//申请通道

注意事项

* DMA和Cache一致性问题
* 一致性问题还存在于Cache开关时刻，在开启MMU之前要关Cache
* AHB总线的对应内存，APB总线的对应外设
* DMA分为写合并writecombine还有一致性写coherent
* 官方推荐使用dmaengine框架及API操作DMA

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

```c
bootspi0: bootspi0@f5001000 {
    compatible = "snps,dw-apb-ssi";
    #address-cells = <1>;
    #size-cells = <0>;
    reg = <0xf5001000 0x1000>;
    interrupts = <GIC_SPI 56 IRQ_TYPE_LEVEL_HIGH>;
    num-cs = <1>;
    status = "okay";
    dma-names = "rx", "tx";
    dmas      = <&dmac 25 0 0>, <&dmac 26 0 0>;
    cs-gpios =  <&gpio1 11 GPIO_ACTIVE_HIGH>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_bspi>;
    reg-io-width = <4>; /* 寄存器的宽度为32，4个字节 */

    flash0: s25fl128s@0 {
        reg = <0>;
        #address-cells = <1>;
        #size-cells = <1>;
        compatible = "s25fl128s";
        spi-max-frequency = <500000>;
    };

};
```



代码改了哪里

```c
static struct dw_dma_slave mid_dma_tx = { .dst_id = 26 };
static struct dw_dma_slave mid_dma_rx = { .src_id = 25 };


```

