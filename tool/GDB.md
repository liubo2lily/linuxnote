#### 准备工作

下载jlink软件

###### Linux版本：

```
gdb  JLink_Linux_V688b_x86_64.deb
gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
```

###### window版本：

JLink_Windows_V698.exe

#### 安装必备软件

1. 下载交叉编译器后，将该压缩包解压到/opt/目录下即可；

2. 安装Linux版本jlink命令如下

   ```
   sudo apt-get install libncurses5 libtinfo5
   sudo dpkg -i JLink_Linux_V688b_x86_64.deb`
   ```

#### 编译源码

1. 访问git服务器seehi_SW/1210-freeRTOS目录，下载代码压缩文件，也可以直接c克隆代码

    `http://192.168.10.42:10080/seehi_SW/1210-freeRTOS`

2. 修改Makefile

   修改Makefile，CROSS_COMPILE变量指向的是编译器的绝对路径；

   CROSS_COMPILE 	:= /opt/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-

3. 编译

   执行make命令，将在Build目录下生成目标文件。

#### 在线调试

板子上电后，在串口输出信息后，输入jtag，进入调试模式

运行JLinkGDBServerExe

输出以上信息，代表正常连接。

##### GDB连接命令

本地连接：target extended-remote:2331

通过网络连接：tar remote 192.168.31.250:2331  （gdb server 中localhost only不能选中）

加载文件：

load /home/yd/work/coding/1210-freertos/Build/A7_FreeRTOS.elf 

file /home/yd/work/coding/1210-freertos/Build/A7_FreeRTOS.elf 

##### GDB调试命令

开始运行

c

###### 断点：

|              描述              | 命令                                                        |
| :----------------------------: | ----------------------------------------------------------- |
|       删除指定的某个断点       | delete breakpoint 1                                         |
|          删除所有断点          | delete breakpoint                                           |
|     显示当前gdb的断点信息      | info break                                                  |
|        禁止使用某个断点        | disable breakpoint 1                                        |
|        允许使用某个断点        | enable breakpoint 1                                         |
| 清除源文件中某行代码的所有断点 | clean number                                                |
|            设置断点            | b line-number                                               |
|               b                | b function-name                                             |
|                                | b line-or-function if expr (such:break 46 if testsize==100) |

###### 执行程序

show regs 查看用户输入的参数

backtrace 堆栈提供向后追踪功能

next 不进入的单步执行

step 进入的单步执行

###### 显示数据

print p	查看变量的值

print h@10 显示在h后面的10个元素

whatis p 查看变量的类型

set varibale 设置变量的值

###### 机器语言工具

$pc 程序计数器

$fp 帧指针

$sp 栈指针

$ps 处理器状态

###### 常用的GDB命令

 backtrace  显示程序中的当前位置和表示如何到达当前位置的栈跟踪（同义词：where）  
 		breakpoint  在程序中设置一个断点  
 		cd  改变当前工作目录  
 		clear  删除刚才停止处的断点  
 		commands  命中断点时，列出将要执行的命令  
 		continue  从断点开始继续执行  
		delete  删除一个断点或监测点；也可与其他命令一起使用  
		display  程序停止时显示变量和表达时  
 		down  下移栈帧，使得另一个函数成为当前函数  
		 frame  选择下一条continue命令的帧  
		 info  显示与该程序有关的各种信息  
		 jump  在源程序中的另一点开始运行  
		 kill  异常终止在gdb  控制下运行的程序  
		 list  列出相应于正在执行的程序的原文件内容  
		 next  执行下一个源程序行，从而执行其整体中的一个函数  
		 print  显示变量或表达式的值  
		 pwd  显示当前工作目录  
		 pype  显示一个数据结构（如一个结构或C++类）的内容  
		 quit  退出gdb  
		 reverse-search  在源文件中反向搜索正规表达式  
		 run  执行该程序  
		 search  在源文件中搜索正规表达式  
		 set  variable  给变量赋值  
		 signal  将一个信号发送到正在运行的进程  
		 step  执行下一个源程序行，必要时进入下一个函数  
		 undisplay  display命令的反命令，不要显示表达式  
		 until  结束当前循环  
		 up  上移栈帧，使另一函数成为当前函数  
		 watch  在程序中设置一个监测点（即数据断点）  
		 whatis  显示变量或函数类型 

###### info命令

info命令可以用来在调试时查看寄存器、断点、观察点和信号等信息。其用法如下：

info registers:									  查看除了浮点寄存器以外的寄存器
		info all-registers: 								查看所有的寄存器包括浮点寄存器
		info registers < registersName>:	  查看指定寄存器
		info break: 											查看所有断点信息
		info watchpoints: 								查看当前设置的所有观察点
		info signals info handle:					 查看有哪些信号正在被gdb检测
		info line: 												查看源代码在内存中的地址
		info threads: 										可以查看多线程

###### watch命令

watch命令一般来观察某个表达式(变量也可视为一种表达式)的值是否发生了变化，如果由变化则程序立即停止运行，其具体用法如下：

watch < expr>为表达式(变量)expr设置一个观察点一旦其数值由变化，程序立即停止运行
		rwatch < expr>当表达式expr被读时，程序立即停止运行
		awatch < expr>当表达式expr的值被读或被写时程序立即停止运行
		info watchpoints列出当前所设置的所有观察点

###### 其他命令

bt 显示函数调用路径

##### 查看内存命令

x/<n/f/u> <addr>

###### x ：examine命令的简写

###### n 显示内存单元的个数

###### f 显示的格式

​	x 按十六进制格式显示变量。

​	d 按十进制格式显示变量。

​	u 按十六进制格式显示无符号整型。

​	o 按八进制格式显示变量。

​	t 按二进制格式显示变量。

​	a 按十六进制格式显示变量。

​	c 按字符格式显示变量。

​	f 按浮点数格式显示变量。

###### u 从当前地址往后请求的字节数

​	b:单字节

​	h:双字节

​	w:四字节

#### uboot调试命令
