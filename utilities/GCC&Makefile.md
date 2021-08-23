参考资料

* 百度搜 "gnu make 于凤昌"
* 查看官方文档: [http://www.gnu.org/software/make/manual/](http://www.gnu.org/software/make/manual/)
* Makefile生成依赖文件：https://blog.csdn.net/qq1452008/article/details/50855810

## GCC

编译选项补充

* -c	只编译不链接

* -I	指定头文件目录

* -v	展示编译完整过程

静态库制作.a

```shell
$ gcc -c -o main.o main.c
$ gcc -c -o sub.o sub.c
$ ar crs libsub.a sub.o
$ gcc -o test main.o libsub.a
```

动态库制作.so(shared object)

```shell
$ gcc -c -o main.o main.c
$ gcc -c -o sub.o sub.c
$ gcc -shared -o libsub.so sub.o
$ gcc -o test main.o libsub.so						#method1, 
$ gcc -o test main.o -lsub -L <libsub.so所在目录> 	#method2, lsub会省略掉lib前缀和.so后缀
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<自己的目录> #运行时指定动态库位置
```

重要补充

```shell
$ gcc -E -dM main.c > 1.txt					#把所有宏展开，放在文件里
$ gcc -Wp,-MD,abc.dep -c -o main.o main.c 	#生成依赖文件
$ echo 'main(){}' | gcc -E -v - #会列出涉及到的头文件目录以及库目录
```

## Makefile

### 1. 基本规则

makefie最基本的语法是规则，规则：

```bash
目标文件 : 依赖文件1 依赖文件2 ...
[TAB]命令
```

**当“依赖”比“目标”新**，就会重新执行依赖的命令，并重新生成目标。

文件：Makefile

```Makefile
test : a.o b.o
	gcc -o test a.o b.o	
a.o : a.c
	gcc -c -o a.o a.c
b.o : b.c
	gcc -c -o b.o b.c
```

#### 1.1 通配符

用于解决文件太多，逐个书写较麻烦的情况

```bash
test: a.o b.o 
	gcc -o test $^	
%.o : %.c
	gcc -c -o $@ $<
```

%：表示所有的

$@：表示目标

.$@：生成的目标会被隐藏

$<：表示第1个依赖文件

$^：表示所有依赖文件

#### 1.2 假想目标: .PHONY

```makefile
.PHONY: clean #把clean定义为假象目标。他就不会判断名为“clean”的文件是否存在，
	rm *.o test
```

但如果存在同名文件，则会报错，因此引入.PHONY关键字，可忽略掉同名文件

#### 1.3 变量

* 想使用变量的时候使用$(var)来引用

* 如果不想看到命令，可以在命令的前面加上@符号，就不会显示命令本身

* makefile变量定义位置随便，但后面的新值会覆盖旧值
* 可以在make后接变量名，给变量赋值，针对?=变量

```makefile
:=		#即时变量，即刻生效
= 		#延时变量，用到时才生效
?= 		#延时变量, 变量是否存在，若不存在则定义，若存在则不定义
+= 		#附加, 它是即时变量还是延时变量取决于前面的定义
?=: 	#如果这个变量在前面已经被定义了，这句话就会不会起效果
A := $(C)
B = $(C)
C = abc
D := 111
D ?= test
all:
	@echo A = $(A)
	@echo B = $(B)
	@echo D = $(D)
C += 123
```

结果：

```bash
A =
B = abc 123
D = 111
```

* 变量可以通过export关键字导出，使所有makefile都能看到

### 2. Makefile函数

* foreach(遍历)

```bash
$(foreach var,list,text) #对于list中的每一个var执行test
```

```bash
A = a b c
B = $(foreach f, $(A), $(f).o)
all:
	@echo B = $(B)
```

结果：

```bash
B = a.o b.o c.o
```

* filter/filter-out(过滤)

```bash
$(filter pattern...,text)     # 在text中取出符合patten格式的值
$(filter-out pattern...,text) # 在text中取出不符合patten格式的值
```

```bash
C = a b c d/
D = $(filter %/, $(C))
E = $(filter-out %/, $(C))
all:
    @echo D = $(D)
    @echo E = $(E)
```

结果：

```bash
D = d/
E = a b c
```

* Wildcard(判断存在)

```bash
$(wildcard pattern) # pattern若为文件名的格式，返回符合条件的所有文件名, 若为多个文件名，则返回真实存在的文件名
```

在该目录下创建三个文件：a.c b.c c.c

```bash
files2 = a.c b.c c.c d.c e.c  abc
files = $(wildcard *.c)
files3 = $(wildcard $(files2))
all:
        @echo files = $(files)
        @echo files3 = $(files3)
```

结果：

```bash
files = a.c b.c c.c
files3 = a.c b.c c.c
```

* patsubst(替换)

```bash
$(patsubst pattern,replacement,$(var)) #从 var 变量里面取出每一个值，如果这个符合pattern格式，把它替换成replacement格式
```

```bash
files2  = a.c b.c c.c d.c e.c abc
dep_files = $(patsubst %.c,%.d,$(files2))
all:
	@echo dep_files = $(dep_files)
```

结果：

```bash
dep_files = a.d b.d c.d d.d e.d abc
```

### 3. Makefile实例
makefile不会自动检测h文件更新，因此h文件也需要写进makefile，但给每个c文件都写上它的h太过复杂，因此需要自动生成头文件依赖

```bash
gcc -M <filename> # 打印出文件依赖
gcc -M -MF test.d test.c  # 把依赖写入文件d文件
gcc -c -o test.o test.c -MD -MF test.d  # 编译c.o, 把依赖写入文件c.d
```
修改Makefile如下：
```makefile
objs = a.o b.o c.o
dep_files := $(patsubst %,.%.d, $(objs))
dep_files := $(wildcard $(dep_files))
CFLAGS = -Werror -Iinclude
test: $(objs)
	gcc -o test $^
#判断dep_files不为空
ifneq ($(dep_files),) 
include $(dep_files)
endif
%.o : %.c
	gcc $(CFLAGS) -c -o $@ $< -MD -MF .$@.d
clean:
	rm *.o test
distclean:
	rm $(dep_files)	
.PHONY: clean	
```
CFLAGS是给编译加上额外选项Werror-警告也是错误，-Iinclude指定头文件路径

### 4. 知识点补充

* 执行make，可以重定向make文件，目录，以及生成目标

```bash
make -C dir/ -f othermakefile other_target
```

* 可先放置目标名，然后在后面再补充

### 5. 通用的Makefile

 [Makefile](..\code\makefile\Makefile)  [Makefile.build](..\code\makefile\Makefile.build) [readme](..\code\makefile\readme.txt) 



