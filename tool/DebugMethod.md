参考资料

* Documentation/admin-guide/kernel-parameters.txt //内含uboot参数的详细解释

## 1. 打印大法

### 1.0 需要了解

* 前提：uboot中set bootargs添加console=ttySAC0(可多个设备)

* 打印分很多等级，如下

| value | macro        | description                      |
| ----- | ------------ | -------------------------------- |
| 0     | KERN_EMERG   | system is unusable               |
| 1     | KERN_ALERT   | action must be taken immediately |
| 2     | KERN_CRIT    | critical conditions              |
| 3     | KERN_ERR     | error conditions                 |
| 4     | KERN_WARNING | warning conditions               |
| 5     | KERN_NOTICE  | normal but significant condition |
| 6     | KERN_INFO    | informational                    |
| 7     | KERN_DEBUG   | debug-level messages             |

### 1.1 调整控制台打印级别

* pirntk级别小于才打印console_loglevel
* 静态

可通过set bootargs添加loglevel=x指定当前控制台打印级别

* 动态

```shell
$ cat /proc/sys/kernel/printk
7		4		1		7
#console_loglevel	default_message_level	minimum_console_loglevel	default_console_loglevel
#控制台日志级别        默认的消息日志级别        最低的控制台日志级别            默认的控制台日志级别	
$ echo "8 4 1 7" > /proc/sys/kernel/printk
```

### 1.2 dmesg

dmesg是打印/proc/kmsg中的信息，它会输出所有模块的打印信息

## 2. 段错误分析

不ioremap直接使用地址

```shell
Unable to handle kernel paging request at virtual address 56000050#内核使用56000050来访问时发生了错误

pgd = c3eb0000
[56000050] *pgd=00000000
Internal error: Oops: 5 [#1]
Modules linked in: first_drv
CPU: 0    Not tainted  (2.6.22.6 #1)
PC is at first_drv_open+0x18(该指令的偏移)/0x3c(该函数的总大小) [first_drv]#PC就是发生错误的指令的地址
#大多时候，PC值只会给出一个地址，不会指示说是在哪个函数里
LR is at chrdev_open+0x14c/0x164	#LR寄存器的值
pc : [<bf000018>]    lr : [<c008d888>]    psr: a0000013	#执行这条导致错误的指令时各个寄存器的值
sp : c3c7be88  ip : c3c7be98  fp : c3c7be94
r10: 00000000  r9 : c3c7a000  r8 : c049abc0
r7 : 00000000  r6 : 00000000  r5 : c3e740c0  r4 : c06d41e0
r3 : bf000000  r2 : 56000050  r1 : bf000964  r0 : 00000000

Flags: NzCv  IRQs on  FIQs on  Mode SVC_32  Segment user
Control: c000717f  Table: 33eb0000  DAC: 00000015
Process firstdrvtest (pid: 777, stack limit = 0xc3c7a258)#发生错误时当前进程的名称是firstdrvtest

Stack: (0xc3c7be88 to 0xc3c7c000)#栈
be80:                   c3c7bebc c3c7be98 c008d888 bf000010 00000000 c049abc0 
bea0: c3e740c0 c008d73c c0474e20 c3e766a8 c3c7bee4 c3c7bec0 c0089e48 c008d74c 
bec0: c049abc0 c3c7bf04 00000003 ffffff9c c002c044 c3d10000 c3c7befc c3c7bee8 
bee0: c0089f64 c0089d58 00000000 00000002 c3c7bf68 c3c7bf00 c0089fb8 c0089f40 
bf00: c3c7bf04 c3e766a8 c0474e20 00000000 00000000 c3eb1000 00000101 00000001 
bf20: 00000000 c3c7a000 c04a7468 c04a7460 ffffffe8 c3d10000 c3c7bf68 c3c7bf48 
bf40: c008a16c c009fc70 00000003 00000000 c049abc0 00000002 bec1fee0 c3c7bf94 
bf60: c3c7bf6c c008a2f4 c0089f88 00008520 bec1fed4 0000860c 00008670 00000005 
bf80: c002c044 4013365c c3c7bfa4 c3c7bf98 c008a3a8 c008a2b0 00000000 c3c7bfa8 
bfa0: c002bea0 c008a394 bec1fed4 0000860c 00008720 00000002 bec1fee0 00000001 
bfc0: bec1fed4 0000860c 00008670 00000002 00008520 00000000 4013365c bec1fea8 
bfe0: 00000000 bec1fe84 0000266c 400c98e0 60000010 00008720 00000000 00000000 

Backtrace:#回溯，即函数的调用关系，要定义CONFIG_FRAME_POINTER=y才有
[<bf000000>] (first_drv_open+0x0/0x3c [first_drv]) from [<c008d888>] (chrdev_open+0x14c/0x164)
[<c008d73c>] (chrdev_open+0x0/0x164) from [<c0089e48>] (__dentry_open+0x100/0x1e8)
 r8:c3e766a8 r7:c0474e20 r6:c008d73c r5:c3e740c0 r4:c049abc0
[<c0089d48>] (__dentry_open+0x0/0x1e8) from [<c0089f64>] (nameidata_to_filp+0x34/0x48)
[<c0089f30>] (nameidata_to_filp+0x0/0x48) from [<c0089fb8>] (do_filp_open+0x40/0x48)
 r4:00000002
[<c0089f78>] (do_filp_open+0x0/0x48) from [<c008a2f4>] (do_sys_open+0x54/0xe4)
 r5:bec1fee0 r4:00000002
[<c008a2a0>] (do_sys_open+0x0/0xe4) from [<c008a3a8>] (sys_open+0x24/0x28)
[<c008a384>] (sys_open+0x0/0x28) from [<c002bea0>] (ret_fast_syscall+0x0/0x2c)
Code: e24cb004 e59f1024 e3a00000 e5912000 (e5923000) 
Segmentation fault
```

### 2.1 判断出错位置是否处于内核范围

查找编译生成文件System.map，该pc值文件项地址范围内，则为内核范围，否则不是。

### 2.2 段错误发生在模块

pc : [<bf000018>]

```shell
#1. 打印内核函数、加载函数的地址符号表，其中t代表全局函数，T代表静态函数
$ cat /proc/kallsyms > ~/kallsyms.txt
#2. 找小于且距离pc指针值最近的,如下就确定了具体模块
bf000000 t first_drv_open	[first_drv]
#3. 反汇编,文件地址从0开始，但偏移一致，可一一对应
arm-linux-objdump -D first_drv.ko > frist_drv.dis
#    first_drv.dis文件里              insmod后
#    00000000 <first_drv_open>:      bf000000 t first_drv_open	[first_drv]
#    00000018                  <=    pc = bf000018
#4. 找到对应指令，将寄存器值代入
18:		e5923000	ldr	r3,	[r2]	#出错位置在这 r2=56000050
```

### 2.3 段错误发生在内核

pc : [<c014e6c0>]

```shell
# 直接反汇编
$ arm-linux-objdump -D vmlinux > vmlinux.dis
# 搜索pc指针地址，并代入寄存器值
c014e6c0:       e5923000        ldr     r3, [r2] # 在此出错 r2=56000050
```

## 3. 根据栈信息分析函数调用流程(ko为例)

如果内核没有定义CONFIG_FRAME_POINTER=y，或是处理器不支持帧指针，则只能用此方法

* arm寄存器

| 寄存器 | 别名  | 作用                         |
| ------ | ----- | ---------------------------- |
| r0-r3  | a1-a4 |                              |
| r4-r9  | v1-v6 |                              |
| r10    | sl    | 栈限制                       |
| r11    | fp    | 帧指针                       |
| r12    | ip    | 内部过程调用寄存器           |
| r13    | sp    | 栈指针                       |
| r14    | lr    | 连接寄存器，用于存放返回地址 |
| r15    | pc    | 程序计数器                   |

* 压栈过程中，高地址放高标号

基本流程

* 根据打印的lr地址，在反汇编找到相应函数
* 在函数中找到压栈操作，计算下一个lr偏移
* 若发现sp自减，则进一步偏移
* 根据偏移找到下一个lr，重复以上流程

反汇编查找


```shell
---------------------------------------------------------------------------------------
c014e6a8 <first_drv_open>:
c014e6ac:       e92dd800        stmdb   sp!, {fp, ip, lr, pc}
---------------------------------------------------------------------------------------
c008c888 <chrdev_open>:
c008c740:		e92dd9f0	stmdb	sp!, {r4, r5, r6, r7, r8, fp, ip, lr, pc}
c008c748:		e24dd004	sub     sp, sp, #4		; 0x4    <==一定注意sp减法
---------------------------------------------------------------------------------------
c0088d47 <__dentry_open>:
c0088d4c:		e92dddf0	stmdb	sp!, {r4, r5, r6, r7, r8, sl, fp, ip, lr, pc}
---------------------------------------------------------------------------------------
c0088f30 <nameidata_to_filp>
c0088f34:		e92dd810	stmdb	sp!, {r4, fp, ip, lr, pc}
c0088f3c:		e24dd004	sub		sp, sp, #4		;0x4    <==一定注意sp减法
---------------------------------------------------------------------------------------
c0088f78 <do_flip_open>:
c0088f7c:		e92dd830	stmdb	sp!, {r4, r5, fp, ip, lr, pc}
c0088f84:		e24dd054	sub		sp, sp, #84		;0x54    <==一定注意sp减法, 21个空
---------------------------------------------------------------------------------------
c00892a0 <do_sys_open>:
c00892a4:		e92dddf0	stmdb	sp!, {r4, r5, r6, r7, r8, sl, fp, ip, lr, pc}
c00892ac:		e24dd004	sub		sp, sp, #4		;0x4    <==一定注意sp减法
---------------------------------------------------------------------------------------
c0089384 <sys_open>:
c0089388:		e92dd800	stmdb	sp!, {fp, ip, lr, pc}
---------------------------------------------------------------------------------------
c002aea0 <ret_fast_syscall>:
```

栈信息分析

```shell
Stack: (0xc3e69e88 to 0xc3e6a000)
be80:                   c3e69ebc c3e69e98 c008c888 bf000010 00000000 c0490620 
                        fp       ip       lr<-     pc                r4       
bea0: c3e320a0 c008c73c c0465e20 c3e36cb4 c3e69ee4 c3e69ec0 c0088e48 c008c74c 
      r5       r6       r7       r8       fp       ip       lr<-     pc    
bec0: c0490620 c3e69f04 00000003 ffffff9c c002b044 c06e0000 c3e69efc c3e69ee8 
      r4       r5       r6       r7       r8       sl       fp       ip
bee0: c0088f64 c0088d58 00000000 00000002 c3e69f68 c3e69f00 c0088fb8 c0088f40 
      lr<-     pc                r4       fp       ip       lr<-     pc
bf00: c3e69f04 c3e36cb4 c0465e20 00000000 00000000 c3e79000 00000101 00000001 
bf20: 00000000 c3e68000 c04c1468 c04c1460 ffffffe8 c06e0000 c3e69f68 c3e69f48 
bf40: c008916c c009ec70 00000003 00000000 c0490620 00000002 be94eee0 c3e69f94 
                                                   r4       r5       fp
bf60: c3e69f6c c00892f4 c0088f88 00008520 be94eed4 0000860c 00008670 00000005 
      ip       lr<-     pc                r4       r5       r6       r7
bf80: c002b044 4013365c c3e69fa4 c3e69f98 c00893a8 c00892b0 00000000 c3e69fa8 
      r8       sl       fp       ip       lr<-     pc       fp       ip
bfa0: c002aea0 c0089394 be94eed4 0000860c 00008720 00000002 be94eee0 00000001 
      lr<-     pc                   
bfc0: be94eed4 0000860c 00008670 00000002 00008520 00000000 4013365c be94eea8 
bfe0: 00000000 be94ee84 0000266c 400c98e0 60000010 00008720 00000000 00000000 
#推算出调用顺序
ret_fast_syscall->sys_open->do_sys_open->do_flip_open->nameidata_to_filp->__dentry_open->chrdev_open->first_drv_open
```

## 4. 寄存器直接读写

busybox自带devmem但也可以自制

 [regeditor.c](code\driver_debug\regeditor.c)     [ker_rw.c](code\driver_debug\ker_rw.c) 

## 5. 定位系统僵死

原理：系统时钟中断永不停歇，不断再保存恢复现场，现场中的pc值-4，就是一直卡死的位置

系统的总中断在arch/arm/kernel/irq.c-->asm_do_IRQ

```c
asmlinkage void __exception asm_do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	//..
	static pid_t pre_pid;
	static int cnt = 0;
	
	if (irq == 30) {//具体中断号根据内核来定，发生次数最多的一般就是cat /proc/interrupts
		if (pre_pid == current->pid) {	//current表示当前进程
			cnt ++;
		}
		else {
			cnt = 0;
			pre_pid = current->pid;
		}
		if (cnt == 10*HZ) {
			cnt = 0;
			printk("asm_do_IRQ : pid = %d, task name = %s\n", current->pid, current->comm);
			printk("pc = %08x", regs->ARM_pc); //regs结构体存放了现场各个寄存器值
		}	
	}
	//..
}
```

引入问题，在first_drv_write函数中引入while(1)

```shell
./firstdrvtest on 
asm_do_IRQ : pid = 752, task name = firstdrvtest
pc = bf000084   #对于中断, pc-4才是发生中断瞬间的地址，中断返回地址应该是pc-4
# 1. 查看符号表，确定发生的模块
$ cat /proc/kallsyms > ~/kallsyms.txt
# 2. 反汇编ko文件
arm-linux-objdump -D first_drv.ko > frist_drv.dis
00000000 <first_drv_open>:                     bf000000 t first_drv_open	[first_drv]         
0000003c <first_drv_write>:
80:	1bfffffe 	blne	80 <first_drv_write+0x44>      # 卡死的地方
```











