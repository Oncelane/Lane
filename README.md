## Lane-ServerFramework
本项目基于sylar和lunar的协程服务器框架，以体验优化技术为主要目标，将重写整个服务器框架

优化： gmp调度，boost.fcontext协程切换
实现：协程锁、协程条件变量、channel

待实现： 协程池、select


### 项目依赖

~~~ bash
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
~~~

### 项目启动

1. 拉去仓库，切换分支
~~~ bash
git clone https://github.com/Oncelane/Lane.git
cd Lane
git checkout boost-context
~~~

2. 编译，运行

~~~ bash
mkdir build
cd build
cmake ..
make -j8
~~~

开发环境

https://blog.csdn.net/m0_72743841/article/details/139465617

clangd-17