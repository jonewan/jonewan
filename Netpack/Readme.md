# 项目终端NETPACK命令使用说明

## 一、描述

NETPACK是一款基于NETCAT与ZLIB库开发的一个用于打包压缩日志文件并传输的工具。将此项目clone到本地后执行`make`操作，在项目目录`/bin`下会生成可执行文件`np`。

## 二、使用说明

 ```c
 netpack: [v1.00]
 Usage:  np 192.xxx.xxx.xxx:0000 [file[s] || dir[s]]
  option:
     - 0      Store only
     - 1      Compress faster
     - 9      Compress better
     - a      Append to existing file.cvtelog.zip
     - f       Assign the zip fileName
     - h      Help
     - k      Password the zip file
     - n      Nmeric-only IP addresses, no DNS
     - o      Overwrite existing file.cvtelog.zip
     - s      Addr local source address
     - t      Answer TELNET negotiation
     - u     UDP mode
     - v     Verbose [use twice to be more verbose]
     - w     Secs    timeout for connects and final net reads
     - z      Zero-I/O mode [used for scanning] 
```

## 三、基本使用用例

### 3.1 接收端

* 若你使用的是windows系统，请使用`windws`+`R`, 打开命令提示符，找到你安装netcat的指定目录（安装NETCAT的方法在后文），例如：在桌面安装的 `netcat.exe`文件，使用：

```bash
cd C:\Users\user\Desktop
netcat.exe  -l -p 9090 > log.zip
```

* 若你使用的是任意的Linux发行版本，请安装netcat后，执行：

```bash
localhost$ nc -l -p 9090 > log.zip
```

其中`log.zip`可以换成你指定的zip文件, `9090`端口也可以换成你指定的端口号，但必须通知发送方你的IP地址以及端口号信息，否则对方不会知道给你传送到哪个地方的哪个端口。

### 3.2 发送端

在项目开发的终端上按 下`ctrl` + `alt` + `t`，进入crosh环境，执行如下代码。

```c
localhost$ np 192.168.1.2:9090 /var/log
```

等待连接成功并且发送完成后，Crosh会提示`Transmission finished!`，并在1秒后自动断开连接。

## 四、其他用例

NETPACK命令可以打包任意目录的任意文件，因此你如果有特殊的需求，例如：

* 对压缩文件大小有要求，则在发送时使用`- 0`，`- 1`，`- 9`参数，`- 0`表示仅打包整个目录或文件，并不做压缩，`- 1`表示快速压缩，但压缩效率不高，`- 9`表示高效压缩。
* 对文件安全性有要求，可以使用`- k` + `password`，对压缩文件进行加密。
* 对传输等待时间有要求，使用`- w`+`n`，表示若等待n秒没有启动传输则停止。

其他详细用法请使用`np -h`查看帮助信息。

## 五、NETCAT的安装方法

### 5.1 Windows

若您是windows用户，[点击此处下载](https://eternallybored.org/misc/netcat/netcat-win32-1.11.zip)提供的`netcat.exe`程序，将该程序复制到任意你可以找到的目录使用即可，使用方法见上文。

### 5.2 Linux

若您是Ubuntu用户，请使用下面方法安装：

```bash
apt-get install netcat
```

若您是Centos用户，请使用下面方法安装：

```bash
yum install netcat
```

若您是Gentoo用户，请使用下面方法安装：

```bash
emerge netcat
```
