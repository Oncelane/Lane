
#include <iostream>

#include "base/channel.h"
#include "base/iomanager.h"
lane::channel<int>       ch(1);
static int               g_int = 0;
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void                     producer() {
    while (true) {
        sleep(1);
        LANE_LOG_INFO(g_logger) << "in g_int=" << g_int;
        ch.input(g_int++);
    }
}

void comsumer() {
    while (true) {
        sleep(3);
        int tmp = ch.output();
        LANE_LOG_INFO(g_logger) << "out g_int=" << tmp;
    }
}

void waitSignl() {
    sleep(1);
    ch.output();
    LANE_LOG_INFO(g_logger) << "quit left size=" << ch.size();
}
void releaseSignl() {
    sleep(3);
    LANE_LOG_INFO(g_logger) << "start release";
    for (int i = 0; i < 3; i++) {
        ch.input(true);
    }
}


void test_channel() {
    lane::IOManager iom(1, "test", true);
    iom.addTask(waitSignl);
    iom.addTask(waitSignl);
    iom.addTask(waitSignl);
    iom.addTask(releaseSignl);
    iom.start();
    iom.stop();
}
int main() {
    char *buffer = (char *)malloc(1024);
    buffer[9] = (char)255;
    printf("buffer9=%d", buffer[9]);
    return 0;
}