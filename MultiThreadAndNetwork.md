## 暂存VIM

命令模式
* o O o下新行，O上新行
* a i a后追加，i前追加
* cw  删除到单词尾
* 0 ^ $ g_ 0行头，^非空行头，$行尾，g_非空的行尾
* p P 下一行粘贴，上一行粘贴
* u ctrl+r 取消，反取消
* . 重复上一次操作
* gg G NG gg跳转开头 G跳转末尾 跳转N行
* w e w最后一个单词后，e最后一个单词的最后一个字母
* %匹配括号大括号

底行模式：

* :e 打开其他文件
* :bn :bp切换上下文件

## 一、网络编程

TCP: 可靠，重传 =>控制命令等，重要数据，有连接的传输

UDP: 不可靠     =>视频等，实时数据，无连接的传输

```shell
服务器，TCP传输                  客户端，TCP
fd = socket();                 fd = socket();
bind();//把fd和ip、端口绑定起来
listen();//开始监测数据
accept();//接受一条连接          connect();//指明目的，建立连接
send();//发                    send();//发
recv();//收

服务器，UDP                       客户端，UDP
fd = socket();                  fd = socket();
bind();//把fd和ip、端口绑定起来     connect();//可用，可不用
send(),sendto();//发             send(),sendto();//发 
recv(),recvfrom();//收           recv(),recvfrom();//收
```

* 示例代码 tcp:  [client.c](..\code\socket\tcp\client.c)   [server.c](..\code\socket\tcp\server.c) udp:  [client.c](..\code\socket\udp\client.c)  [server.c](..\code\socket\udp\server.c) 

## 二、多线程编程
* ps -T打印线程
* 资源的分配是以线程为单位的
* 多线程要配合休眠唤醒以及互斥量一起使用，如果不前者会导致CPU占用率过高，后者会导致数据冲突
* 互斥量的占用时间不宜过长，容易造成一直霸占锁的情况
* 可使用条件变量，代替休眠唤醒
* 示例代码  [pthread_sleep.c](..\code\pthread\pthread_sleep.c)   [pthread_cond.c](..\code\pthread\pthread_cond.c) 

