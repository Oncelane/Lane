# Lane

本项目基于 sylar 和 lunar 的协程服务器框架，以体验优化技术为主要目标,重点优化协程模块

已实现

- gmp 调度，worksteal 调度模式
- boost.fcontext 协程切换
- 协程互斥锁
- 协程信号量
- channel(无缓冲/有缓冲)
- waitGroup
- 协程池

# 压测数据

| 项目   | QPS   |
| ------ | ----- |
| lane   | 8.71W |
| golang | 7.00W |

测试页面：

```sh
oncelane@ServerTest:~/workspace/Lane$ curl -i -X GET http://127.0.0.1:8020/
HTTP/1.1 404 Not Found
Content-Type: text/html
Server: lane/1.0.0
connection: close
content-length: 135

<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>lane/1.0</center></body></html>
```

压测相同内容，均为 135 bytes：

![压测](./docs/压测lane2.png)

lane 压测页面内容不变为 135 bytes，golang 压测页面内容减少，为 36 bytes

![压测](./docs/压测lane和golang.png)

在另一配置较差笔记本上的压测数据：

| 项目         | QPS   |
| ------------ | ----- |
| lane O0 编译 | 3.39W |
| golang       | 3.97W |
| lane O2 编译 | 5.96W |

# 项目依赖

```bash
# boost
sudo apt install libboost-all-dev

# json
sudo apt install libjsoncpp-dev

# ragel
sudo apt install ragel

# yaml-cpp.git
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build
cd build
cmake ..
make -j8
sudo make install
```

# 项目启动

1. 拉取仓库

```bash
git clone https://github.com/Oncelane/Lane.git
cd Lane
```

切换分支

```bash
git checkout little-work
```

2. 编译

```bash
mkdir build
cd build
cmake ..
make -j8
```

3. 运行、压测

```sh
webbench 下载

#1.1 安装依赖 exuberant-ctags
sudo apt-get install exuberant-ctags
#1.2 下载源码并安装
wget http://blog.s135.com/soft/linux/webbench/webbench-1.5.tar.gz
tar zxvf webbench-1.5.tar.gz
cd webbench-1.5
make && sudo make install
# 如果出现 <rpc/types.h> 头文件错误
# 打开 webbench.c 编辑
# vim webbench.c
# 修改头文件#include <rpc/types.h> 为#include <sys/types.h>
```

```sh
../bin/test_httpserver
webbench -c 1000 -t 30 http://127.0.0.1:8020//
```

4. valgrind

安装

```sh
sudo apt-get install valgrind
```

运行

```sh
../bin/test_httpserver
```

压测

```sh
 webbench -c 8000 -t 30 http://127.0.0.1:8020/
```

# 开发环境

https://blog.csdn.net/m0_72743841/article/details/139465617

clangd-17
