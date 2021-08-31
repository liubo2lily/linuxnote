#include <linux/serial.h>
/* 用到这2个ioctl: TIOCGRS485, TIOCSRS485 */
#include <sys/ioctl.h>

struct serial_rs485 rs485conf;

/* 打开串口设备 */
int fd = open ("/dev/mydevice", O_RDWR);
if (fd < 0) {
	/* 失败则返回 */
    return -1;
}
/* 读取rs485conf */
if (ioctl (fd, TIOCGRS485, &rs485conf) < 0) {
	/* 处理错误 */
}

/* 使能RS485模式 */
rs485conf.flags |= SER_RS485_ENABLED;

/* 当发送数据时, RTS为1 */
rs485conf.flags |= SER_RS485_RTS_ON_SEND;

/* 或者: 当发送数据时, RTS为0 */
rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

/* 当发送完数据后, RTS为1 */
rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;

/* 或者: 当发送完数据后, RTS为0 */
rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

/* 还可以设置: 
 * 发送数据之前先设置RTS信号, 等待一会再发送数据
 * 等多久? delay_rts_before_send(单位ms)
 */
rs485conf.delay_rts_before_send = ...;

/* 还可以设置: 
 * 发送数据之后, 等待一会再清除RTS信号
 * 等多久? delay_rts_after_send(单位ms)
 */
rs485conf.delay_rts_after_send = ...;

/* 如果想在发送RS485数据的同时也接收数据, 还可以这样设置 */
rs485conf.flags |= SER_RS485_RX_DURING_TX;

if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
	/* 处理错误 */
}

/* 使用read()和write()就可以读、写数据了 */

/* 关闭设备 */
if (close (fd) < 0) {
	/* 处理错误 */
}