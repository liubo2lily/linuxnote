### 1. 配置GPIO中断

每组GPIO中都有对应的GPIOx_ICR1、GPIOx_ICR2寄存器(interrupt configuration register )。
每个引脚都可以配置为中断引脚，并配置它的触发方式：

![](pic/gpio/01_imx6ull_gpiox_icr1.png)



### 2. 使能GPIO中断

![](pic/gpio/02_imx6ull_gpiox_imr.png)



### 3.  判断中断状态、清中断

![image-20201116001853748](pic/gpio/03_imx6ull_gpiox_isr.png)