## 通用知识

*代表0个或多个任意字符，？代表一个任意字符

#### 元字符通配符

* [a-f]包含a-f之间的所有字符的匹配项
* [!a]剔除带a的字符匹配项
* [ab]包含ab字符匹配项

## 文件相关命令

#### touch

即可以创建空文件，若文件已存在，就更新文件的修改时间

#### cp

```shell
-i 若存在覆盖问题，则先通知
-R 递归复制，包括没被创建的文件夹
```

#### ls

```shell
-F 突出显示目录项
-R 递归显示
-i 显示inode
```

#### ln

* 创建链接文件-符号链接、硬链接

```shell
# 符号链接，实实在在的文件，inode号不同，可以跨存储媒介
# 硬链接，inode号相同，不可跨存储媒介
-s <srcfile> <dstfile> 创建符号链接
什么都不加，则创建硬链接
```

#### mv

```shell
-i 同cp，覆盖问题，征求意见
```

#### mkdir

```shell
-p 连同不存在的父目录一起创建
```

#### file

显示文件信息

#### cat

```shell
-n 显示行号
-b 只在有文本的地方排行号
-T 不显示制表符
```

#### less

与cat不同，它是分页浏览的

#### tail/head

```shell
#默认显示文件尾/头部十行数据
-n 重新指定显示的行数
```

#### wc

* 统计文件单词数，字节数，行数等信息

```shell
$ wc <filename>
-c 统计字节数。
-l 统计行数。
-m 统计字符数。这个标志不能与 -c 标志一起使用。
-w 统计字数。一个字被定义为由空白、跳格或换行字符分隔的字符串。
-L 打印最长行的长度。
-help 显示帮助信息
--version 显示版本信息
7     8     70     test.txt
行数   单词数 字节数  文件名
# 用来统计当前目录下的文件数
$ ls -l | wc -l
```

#### sort

* 和grep一样，可以使用管道|

```shell
#默认以字符排序
-n 以数字排序
-M 按月排序
-r 反向排序
$ sort -t ':' -k 3 -n /etc/passwd #根据某个字段进行排序
```

#### grep

```shell
$ grep [options] pattern [file]
-v 反向搜索
-c 统计多少行符合匹配
```



## 进程相关命令

#### ps

```shell
-e 显示所有进程
-f 显示完整格式的输出
用户		 进程ID  父进程ID CPU占用率  启动时间  终端设备  累计时间  程序名称
UID          PID    PPID  C 		STIME 	TTY      TIME     CMD
liubo     208992  208991  0 		09:27 	pts/2    00:00:00 -bash
```

#### top

```shell
-d <time> 设定刷新间隔，单位是秒
进程ID  属主名 进程优先级 谦让度值 所占虚存   所占物存 共享内存 进程状态(D可中断休眠，R运行，S休眠，T跟踪或停止跟踪，Z僵尸)
PID    USER   PR       NI      VIRT     RES     SHR     S        %CPU %MEM TIME+   COMMAND
247341 liubo  20       0       20800    4108    3324    R        1.0  0.1  0:01.35 top  
#运行时，d可动态修改时间，f可修改排序规则
```

#### kill(默认TERM)

| 信号 | 名称 | 描述                         |
| ---- | ---- | ---------------------------- |
| 1    | HUP  | 挂起                         |
| 2    | INT  | 中断                         |
| 3    | QUIT | 结束运行                     |
| 9    | KILL | 无条件终止                   |
| 11   | SEGV | 段错误                       |
| 15   | TERM | 尽可能终止                   |
| 17   | STOP | 无条件停止运行，但不终止     |
| 18   | TSTP | 停止或暂停，但继续在后台运行 |
| 19   | CONT | 在STOP或TSTP后恢复执行       |

#### killall

```shell
$ killall <commandname> 一次性杀掉符合命令名的所有进程，支持通配符
```

#### jobs

* 查看当前运行的后台作业

```shell
-l 显示更多信息
```

## 磁盘使用相关命令

#### mount/umount

```shell
$ mount 直接运行，则显示目前挂载信息
$ mount -t <type> <device> <directory>
-o 添加额外选项，一般-o nolock表示不锁定文件
   对于NFS文件系统，还要-o nolock,vers=3指定nfs协议版本
$ umount <directory | device>
```

#### df

```shell
#显示挂载情况
-h 以易读方式显示内存大小
```

#### du

* 显示目录下所有的文件、目录和子目录的磁盘使用情况，默认当前目录

```shell
-c 显示所有已列出文件总的大小
-h 按用户易读的格式输出大小
-s 显示每个输出参数的总计，与-c不同的是不会列出文件
```



## MISC

#### lsof

```shell
$ lsof <directory | device> 查看正在使用该设备的进程
```

#### history

* 查看历史输入命令

```shell
#可查看历史命令的时间
$ export HISTTIMEFORMAT='%F %T '
```

#### strace

* 跟踪应用程序调用过程

```shell
$ strace -o log.txt ./app
```

#### which

* 列出外部命令存放位置(只能是外部命令，cd等无效)

#### type

* 可通过type判断一个命令是否为内建命令
```shell
-a 列出所有可能性(一个命令可以有多种实现)
```

#### ！！

* 执行上一次的历史命令

#### env/printenv

* 打印全局环境变量
```shell
$ printenv PATH #打印某个变量，也可以用echo，而env无效
```

#### vmware-toolbox-cmd

* 虚拟机空间压缩

```shell
$ sudo vmware-toolbox-cmd disk list
/
/boot
/home
$ sudo vmware-toolbox-cmd disk shrink /
```

#### diff and patch

```shell
diff -Naur test/ test_new/ > test.patch
cd test
patch -p1 < ../test.patch	#p1代表忽略第一级目录
```


## Shell

#### 规则

* 赋值表达式等，不要加空格，空格会被当成单独的命令

#### 多条命令及进程列表

```shell
$ cmd1; cmd2 #分号分隔，一次执行多条命令
$ (cmd1; cmd2) #加上括号，生成进程列表，即在子shell中运行
```

#### 协程

```shell
#除了后台运行，还可以使用coproc创建一个协程
```

#### 环境变量

```shell
$ export <value> 将变量导出到全局环境变量(没$)
$ unset <value> 删除环境变量(没$)
```

