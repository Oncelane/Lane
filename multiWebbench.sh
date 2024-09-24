#!/bin/bash

# 设置并发数和测试时间
concurrent_requests=10
test_time=20
url="http://127.0.0.1:8020//"

# 启动多个 webbench 实例
for i in {1..10}; do  # 修改 5 为你需要的实例数量
    webbench -c $concurrent_requests -t $test_time $url &
done

wait  # 等待所有后台进程完成
