#include "base/iomanager.h"
#include "net/address.h"
#include "net/socket.h"

static lane::Logger::ptr g_looger = LANE_LOG_ROOT();

void test_socket() {
    // std::vector<sylar::Address::ptr> addrs;
    // sylar::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    // sylar::IPAddress::ptr addr;
    // for(auto& i : addrs) {
    //     SYLAR_LOG_INFO(g_looger) << i->toString();
    //     addr = std::dynamic_pointer_cast<sylar::IPAddress>(i);
    //     if(addr) {
    //         break;
    //     }
    // }
    lane::IPAddress::ptr addr =
        lane::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr) {
        LANE_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        LANE_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    lane::Socket::ptr sock = lane::Socket::CreateTCP(addr);
    addr->setPort(80);
    LANE_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if (!sock->connect(addr)) {
        LANE_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        LANE_LOG_INFO(g_looger)
            << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0) {
        LANE_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt <= 0) {
        LANE_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    LANE_LOG_INFO(g_looger) << buffs;
}
int main(int argc, char** argv) {
    lane::IOManager iom(2, "mm");
    // iom.schedule(&test_socket);
    iom.addTask(test_socket);
    return 0;
}