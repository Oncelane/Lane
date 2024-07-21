
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

#include "base/iomanager.h"
#include "base/log.h"
#include "net/bytearray.h"
#include "net/tcpserver.h"
static lane::Logger::ptr g_logger = LANE_LOG_ROOT();

class EchoServer : public lane::TcpServer {
   public:
    void handleClient(lane::Socket::ptr sock) override {
        lane::ByteArray::ptr buff = std::make_shared<lane::ByteArray>();
        while (true) {
            buff->clear();
            std::vector<iovec> iovs;
            int rt = sock->recv(&iovs[0], iovs.size());
            LANE_LOG_INFO(g_logger) << "recv re = " << rt;
            if (rt == 0) {
                LANE_LOG_INFO(g_logger) << " client close:" << *sock;
                break;
            } else if (rt < 0) {
                LANE_LOG_INFO(g_logger)
                    << "client error rt=" << rt << " errno=" << errno
                    << " errstr=" << strerror(errno);
                break;
            }
            buff->setPosition(buff->getPosition() + rt);
            buff->setPosition(0);
            std::cout << buff->toString();
        }
    }
};

void run() {
    auto addr = lane::Address::LookupAny("0.0.0.0:8020");
    // auto addr2 = sylar::UnixAddress::ptr(new
    // sylar::UnixAddress("/tmp/unix_addr"));
    std::vector<lane::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);

    lane::TcpServer::ptr tcp_server(new lane::TcpServer);
    std::vector<lane::Address::ptr> fails;
    while (!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

void test_echoserver() {
    lane::IOManager iom(2);
    iom.addTask(run);
}
int main(int argc, char** argv) {
    test_echoserver();
    return 0;
}
