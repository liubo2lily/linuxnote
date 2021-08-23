# SPI的三种模式

标准SPI

标准SPI通常就称SPI，它是一种串行外设接口规范，有4根引脚信号：clk , cs, mosi, miso。

Dual SPI

它只是针对SPI Flash而言，不是针对所有SPI外设。对于SPI Flash，全双工并不常用，因此扩展了mosi和miso的用法，让它们工作在半双工，用以加倍数据传输。也就是对于Dual SPI Flash，可以发送一个命令字节进入dual mode，这样mosi变成SIO0（serial io 0），mosi变成SIO1（serial io 1）,这样一个时钟周期内就能传输2个bit数据，加倍了数据传输

Qual SPI

与Dual SPI类似，也是针对SPI Flash，Qual SPI Flash增加了两根I/O线（SIO2,SIO3），目的是一个时钟内传输4个bit，所以对于SPI Flash，有标准spi flash，dual spi , qual spi 三种类型，分别对应3-wire, 4-wire, 6-wire，在相同clock下，线数越多，传输速率越高。SPI Flash一般为NOR Flash。

