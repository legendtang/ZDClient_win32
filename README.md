## ZDClient_win32
原项目地址：https://code.google.com/p/zdcclient/

原开发者：Pentie、michael8090、zhhtc200、zhouJFu

Win32 版本已经由 Pentie 授权迁移至此

GNU/Unix 版 (Linux, Mac 用户) 请访问 [这里](https://github.com/isombyt/zdcclient)

### Environment

Compile and test on Windows 10 & Code::Blocks 13.12

已知在 武汉大学 等学校不会出现一段时间后自动掉线问题

### Guide

1. 安装最新的 [WinPcap](http://www.winpcap.org/install/default.htm) 
2. 下载预编译的二进制文件：[Link](https://github.com/legendtang/ZDClient_win32/releases)
3. Windows Vista 以上的操作系统请**右键管理员权限**运行，输入用户名和密码，选择**正确的网卡**连接即可

### Development

Code::Blocks 用户:

1. 安装最新的 [WinPcap](http://www.winpcap.org/install/default.htm)，并下载 [WinPcap Developer's Pack](https://www.winpcap.org/devel.htm) 并解压；
2. 克隆源码库至本地，打开项目，选择 Setting - Compiler settings - Global compiler settings - Linker Setting，添加以下库： 
  - WpdPack\Lib\wpcap.lib
  - X:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\WS2_32.lib
  - X:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\ComCtl32.lib
  - X:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\IPHlpApi.lib
3. 选择 Setting - Compiler settings - Global compiler settings - Search directories，添加 WpdPack\Include\；
4. 编译。
