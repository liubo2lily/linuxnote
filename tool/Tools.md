## lrz

* 用途

  用于实现windows直接向linux进行文件传输，但速率较低

* 下载

```shell
sudo make lrzsz-rebuild
cd /home/liubo/work/buildroot/output/build/lrzsz-0.12.20
sudo ./configure --host=~/tool/ToolChain-6.2.1/bin/arm-linux-gnueabihf --prefix=$PWD/tmp
sudo make
sudo make install
cd tmp #bin目录就是工具

#ps
若存在
lrz: error while loading shared libraries: libnsl.so.1: cannot open shared object file: No such file or directory
则
cp ./output/target/lib/libnsl-2.23.so -> 开发板的bin
ln -s libnsl-2.23.so libnsl.so.1
```

* 使用

```shell
mobaxterm
ctrl + 右键
send file using Z-modem
```

## memtester

memtester主要是捕获内存错误和一直处于很高或者很低的坏位, 其测试的主要项目有随机值,异或比较,减法,乘法,除法,与或运算等等. 通过给定测试内存的大小和次数, 可以对系统现有的内存进行上面项目的测试。

```shell
# 参数
Usage: ./memtester [-p physaddrbase [-d device]] <mem>[B|K|M|G] [loops]
# mem:申请测试内存的大小，loops:次数
$ ./memtester 1M 1 #申请测试1M内存，测试1次
pagesize is 4096
pagesizemask is 0xfffff000
want 1MB (1048576 bytes)
got  1MB (1048576 bytes), trying mlock ...locked.
Loop 1/1:
  Stuck Address       : ok
  Random Value        : ok
  Compare XOR         : ok
  Compare SUB         : ok
  Compare MUL         : ok
  Compare DIV         : ok
  Compare OR          : ok
  Compare AND         : ok
  Sequential Increment: ok
  Solid Bits          : ok
  Block Sequential    : ok
  Checkerboard        : ok
  Bit Spread          : ok
  Bit Flip            : ok
  Walking Ones        : ok
  Walking Zeroes      : ok
  8-bit Writes        : ok
  16-bit Writes       : ok
Done.
```

## stress-ng

* 查看负载

```shell
$ uptime
02:43:06 up  2:43,  load average: 0.58, 0.52, 0.51
#load average表示1分5分15分钟内的平均负载
```

* stress-ng

```shell
$ ./stress-ng -c 40 -t 100 #开辟2个线程循环调用sqrt()，持续5s，
-c N: 				生成N个worker循环调用sqrt()产生cpu压力
-i N: 				生成N个worker循环调用sync()产生io压力
-m N: 				生成N个worker循环调用malloc()/free()产生内存压力
-d N: 				运行N worker HDD write/unlink测试
-t N:				测试持续N秒
-taskset N: 		将压力加到指定核心上
--cpu-method all:	worker从迭代使用30多种不同的压力算法，包括pi, crc16, fft等等
```

## openssh

```shell
$ sudo ./configure --host=/opt/ToolChain-6.2.1/bin/arm-linux-gnueabihf --prefix=$PWD/tmp --with-zlib=../libzlib-1.2.11 --with-ssl-dir=../libopenssl-1.1.1a --without-openssl-header-check
$ sudo make
$ sudo make install
```



