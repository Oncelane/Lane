#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/hook.h"
#include "base/iomanager.h"
#include "base/log.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");

void test_sleep() {
    lane::IOManager iom(2, "mm");
    iom.addTask([]() {
        LANE_LOG_INFO(g_logger) << "begin";
        sleep(2);
        LANE_LOG_INFO(g_logger) << "end";
    });
    iom.addTask([]() {
        LANE_LOG_INFO(g_logger) << "begin";
        sleep(3);
        LANE_LOG_INFO(g_logger) << "end";
    });
    LANE_LOG_INFO(g_logger) << "return 0";
}

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8090);
    inet_pton(AF_INET, "47.99.79.135", &addr.sin_addr.s_addr);

    LANE_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    LANE_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if (rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LANE_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if (rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);

    LANE_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if (rt <= 0) {
        return;
    }

    buff.resize(rt);
    LANE_LOG_INFO(g_logger) << buff;
}
void test_socket() {
    lane::IOManager iom(2, "mm");
    iom.addTask(test_sock);
}
int main() {
    // test_sleep();
    test_sock();
    return 0;
}