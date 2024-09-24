# Lane

本项目基于 sylar 和 lunar 的协程服务器框架，以体验优化技术为主要目标，将重写整个服务器框架

优化： gmp 调度，boost.fcontext 协程切换

实现：协程锁、协程条件变量、channel(未完全测试)

待实现： 协程池、select

# 压测数据

![压测](./docs/压测lane和golang.png)

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

1. 拉取仓库，切换分支

```bash
git clone https://github.com/Oncelane/Lane.git
cd Lane
git checkout boost-context
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

# 开发环境

https://blog.csdn.net/m0_72743841/article/details/139465617

clangd-17
