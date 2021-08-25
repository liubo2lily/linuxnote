参考资料

* 下载地址 https://buildroot.org/download.html
* 使用Buildroot构建根文件系统 https://www.linuxprobe.com/centos-buildroot.html

## 0. 基本命令

```shell
$ make menuconfig #buildroot的menuconfig
$ make linux-rebuild #单独编译内核
$ make linux-menuconfig #内核的menuconfig
$ make uboot-rebuild #单独编译uboot
$ make <pkg>-rebuild #单独编译某个软件包,e.g.:stress-ng-rebuild
$ make busybox-menuconfig #进入busybox配置界面
$ make sdk #生成系统sdk
$ make savedefconfig #用于保存配置生成xxx_defconfig
```

## 1. 架构选择

```shell
Target options  ---> #选择目标板的架构特性
  -> Target Architecture = ARM (little endian)  #指定架构类型及大小端
  -> Target Binary Format = ELF #指定二进制格式
  -> Target Architecture Variant = cortex-A7 #指定内核类型
  -> Target ABI = EABIhf #指定应用程序二进制接口
  -> Floating point strategy = NEON/VFPv4 #指定浮点数策略
  -> ARM instruction set = ARM #指定汇编指令集
```

## 2. 制作或指定交叉工具链

### 2.1自己生成工具链

```shell
Toolchain  ---> #配置交叉工具链
  -> Toolchain type (Buildroottoolchain)      ---> #指定为buildroot生成工具链
  -> Kernel Headers (Linux 4.14.x kernel headers)  ---> #指定内核类型，从内核Makefile中可以查
  -> C library (glibc)  ---> #选择c库为glibc        
  -> glibc version (2.20)  ---> #指定glibc版本       
     *** Binutils Options ***     
  -> Binutils Version (binutils 2.35.2)  ---> #指定二进制工具集版本 
  -> ()  Additional binutils options         
     *** GCC Options ***                         
  -> GCC compiler Version (gcc 9.x)  ---> #指定gcc版本
  -> ()  Additional gcc options                
  -> [*] Enable C++ support #使能c++支持                                   
  -> [*] Enable MMU support #使能MMU
```

最后会生成到output/host

### 2.2 使用现成的交叉编译工具链

```shell
Toolchain  ---> #配置交叉工具链
  -> Toolchain type = External toolchain #指定为外部工具链
  -> Toolchain = Custom toolchain #指定为用户自己的交叉编译器 
  -> Toolchain origin = Pre-installed toolchain #预装的编译器 
  -> Toolchain path =   #编译器绝对路径
  #/home/liubo/tool/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf 它会自己加/bin
  -> Toolchain prefix = $(ARCH)-linux-gnueabihf #指定前缀 
  -> External toolchain gcc version = 7.x #指定外部工具链gcc版本
  -> External toolchain kernel headers series = 4.10.x #指定外部工具链对应内核版本
  -> External toolchain C library = glibc/eglibc  #指定外部工具链c库类型
  -> [*] Toolchain has SSP support? (NEW)  ##
  -> [*] Toolchain has RPC support? (NEW)  
  -> [*] Toolchain has C++ support?
  -> [*] Enable MMU support (NEW)
```

## 3. 指定系统信息(rootfs)

```shell
System configuration  ---> #系统配置  
  -> System hostname = myname #主机名字
  -> System banner = Welcome to xxxxx #欢迎语
  -> Init system = BusyBox #初始化程序
  -> /dev management = Dynamic using devtmpfs + mdev #设备节点管理方式
  -> [*] Enable root login with password (NEW) #开机密码
  -> Root password = 123456

$ sudo make

```

## 4. 生成根文件系统

```shell
Filesystem images  ---> #配置文件系统 
  -> [*] ext2/3/4 root filesystem #如果是 EMMC 或 SD 卡的话就用 ext3/ext4 
  -> ext2/3/4 variant = ext4 #选择 ext4 格式 
  -> [*] ubi image containing an ubifs root filesystem #如果使用 NAND 的话就用 ubifs 
```

## 5. 编译应用包

### 5.1 编译已有的应用包

```shell
Target packages  ---> #目标应用配置 
#自己选，或者
$ sudo make <pkg>-rebuild #单独编译某个软件包,e.g.:stress-ng-rebuild
#可能直接编译出的包，还需要二次编译，如存在configure文件则执行下列万能命令
$ sudo ./configure --host=arm-buildroot-linux-gnueabihf --prefix=$PWD/tmp
$ sudo make
$ sudo make install
$ ls tmp
```

### 5.2 编译自己的应用包(TODO)

config.in和xxx.mk按照规范写，其他的遇到了再整理。

