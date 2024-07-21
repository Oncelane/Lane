#include <map>
#include <string>
#include <vector>

#include "base/log.h"
#include "net/address.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void testLookUp() {
    std::vector<lane::Address::ptr> addrs;

    lane::Address::LookUp(addrs, "www.baidu.com", AF_INET, SOCK_STREAM);

    for (int i = 0; i < addrs.size(); i++) {
        LANE_LOG_DEBUG(g_logger) << addrs[i]->toString();
    }

    LANE_LOG_DEBUG(g_logger) << "read net card";

    std::multimap<std::string, std::pair<lane::Address::ptr, uint32_t>> results;

    if (!lane::Address::GetInterfaceAddresses(results)) {
        LANE_LOG_DEBUG(g_logger) << "read net card fail";
        return;
    }

    for (auto& it : results) {
        LANE_LOG_DEBUG(g_logger) << it.first + "===" << *it.second.first;
    }
}

void testGetInterfaceAddresses() {
    std::multimap<std::string, std::pair<lane::Address::ptr, uint32_t>>
        addrInfo;

    lane::Address::GetInterfaceAddresses(addrInfo);

    for (auto item : addrInfo) {
        LANE_LOG_DEBUG(g_logger)
            << item.first << "--" << item.second.first->toString() << "--"
            << item.second.second << "--"
            << (std::dynamic_pointer_cast<lane::IPv4Address>(item.second.first))
                   ->getBroadcastAddress(item.second.second)
                   ->toString()
            << "--"
            << (std::dynamic_pointer_cast<lane::IPv4Address>(item.second.first))
                   ->getNetWordAddress(item.second.second)
                   ->toString()
            << "--"
            << (std::dynamic_pointer_cast<lane::IPv4Address>(item.second.first))
                   ->getSubnetMask(item.second.second)
                   ->toString();
    }
}

int main() {
    testLookUp();
    // testGetInterfaceAddresses();
    return 0;
}