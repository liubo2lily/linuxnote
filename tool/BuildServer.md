### Ubuntu设置静态ip

从ubuntu17之后，都采用netplan的方式管理网络。

```shell
$ sudo vi /etc/netplan/01-network-manager-all.yaml

#参考写法
# Let NetworkManager manage all devices on this system
network:
  ethernets:
    enxd89ef3f67246:
      addresses: [192.168.68.168/24]
      gateway4: 192.168.68.1
      nameservers:
        addresses: [114.114.114.114, 192.168.68.1]
      dhcp4: no
      optional: no
  version: 2
  renderer: NetworkManager

$ sudo netplan try #先看看写好的文件到底行不行
$ sudo netplan apply #如果行了，调用这个命令生效变更
```

### NFS

```shell
$ sudo apt-get install nfs-kernel-server  # 安装 NFS服务器端

$ sudo vim /etc/exports
#添加
/home/linuxer/nfs/root/ *(rw,sync,no_root_squash)

$ sudo /etc/init.d/nfs-kernel-server restart

#尝试自己挂载自己
$ sudo mount -t nfs 192.168.68.168:/home/linuxer/nfs/root/ /mnt
```

### TFTP

```shell
$ sudo apt-get install xinetd
$ sudo apt-get install tftp tftpd
$ sudo vi /etc/xinetd.d/tftp

service tftp
{
        socket_type    = dgram
        protocol       = udp
        wait           = yes
        user           = linuxer
        server         = /usr/sbin/in.tftpd
        server_args    = -s /home/linuxer/nfs/boot
        disable        = no
        per_source     = 11
        cps            = 100 2
        flags          = IPv4
}

$ sudo /etc/init.d/xinetd restart
```





### 网络启动

```shell
setenv netargs "setenv bootargs console=ttymxc0,115200 root=/dev/nfs ip=192.168.68.167:192.168.68.168:192.168.68.1:255.255.255.0::eth0:off nfsroot=192.168.68.168:/home/linuxer/nfs/root,v3,tcp"

setenv netargs "setenv bootargs console=ttymxc0,115200 root=/dev/nfs ip={ipaddr}:{serverip}:{gatewayip}:255.255.255.0::eth0:off nfsroot={serverip}:{nfsroot},v3,tcp"
```

