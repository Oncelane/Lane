#include <arpa/inet.h>
#include <base/MysqlManager.h>
#include <mysql/mysql.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"

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
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "221.204.56.95", &addr.sin_addr.s_addr);

    LANE_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    LANE_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno
                            << " str:" << strerror(errno);

    if (rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.1\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LANE_LOG_INFO(g_logger)
        << "send rt=" << rt << " errno=" << errno << " str:" << strerror(errno);

    if (rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);

    LANE_LOG_INFO(g_logger)
        << "recv rt=" << rt << " errno=" << errno << " str:" << strerror(errno);

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


void TestFiberMysql() {
    auto sql = lane::SqlMgr::GetInstance()->getSql();
    if (mysql_query(sql, "SELECT * FROM user")) {
        fprintf(stderr, "SELECT * failed. Error: %s\n", mysql_error(sql));
    }

    MYSQL_RES *res = mysql_store_result(sql);
    if (res) {
        // 处理结果
        //打印各行
        MYSQL_ROW row = NULL;
        row = mysql_fetch_row(res);
        while (NULL != row) {
            for (int i = 0; i < 2; i++) {
                std::cout << row[i] << "\t\t";
            }
            std::cout << std::endl;
            row = mysql_fetch_row(res);
        }

        mysql_free_result(res);
    }
    lane::SqlMgr::GetInstance()->putSql(sql);
}

void TestMain() {
    auto sqlmgr = lane::SqlMgr::GetInstance();
    LANE_LOG_INFO(g_logger) << sqlmgr;

    lane::IOManager iom(2, "system", false);
    iom.addTask([=]() {
        for (int i = 0; i < 24; ++i) {
            lane::IOManager::GetThis()->addTask([=]() { TestFiberMysql(); });
        }
    });

    std::cin.get();
}

int main() {
    // test_socket();
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::INFO);
    TestMain();
}
