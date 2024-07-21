#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "base/iomanager.h"
#include "base/log.h"

static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void test() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8090);
    inet_pton(AF_INET, "47.99.79.135", &addr.sin_addr.s_addr);

    int rt = connect(sockfd, (const sockaddr*)&addr, sizeof(addr));
    if (rt == -1 && EINPROGRESS == errno) {
        // iom->addEvent(sockfd, lane::IOManager::READ, [](){
        //     LANE_LOG_DEBUG(g_logger) << "read cb";
        // });

        lane::IOManager::GetThis()->addEvent(
            sockfd, lane::IOManager::WRITE, []() {
                LANE_LOG_DEBUG(g_logger) << "write cb";
            });
    } else {
        LANE_LOG_DEBUG(g_logger) << "error:" << errno << strerror(errno);
    }
}

void test_timer() {
    lane::IOManager iom(2, "tm");

    iom.addTimer(
        1000,
        []() {
            LANE_LOG_DEBUG(g_logger) << "timer...";
        },
        true);
}

int main() {
    lane::IOManager iom(2, "mmm");
    iom.addTask(test);
    // test_timer();
    LANE_LOG_DEBUG(g_logger) << "return";
    return 0;
}