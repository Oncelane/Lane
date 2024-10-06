#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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


const char* host = "127.0.0.1";         //主机名
const char* user = "debian-sys-maint";  //用户名
const char* pwd = "QTLVb6BaeeaJsFMT";   //密码
const char* database_name = "yourdb";   //数据库名称
int         port = 3306;                //端口号

MYSQL* ConnetcMysql() {
    MYSQL* sql = nullptr;
    sql = mysql_init(sql);
    if (!sql) {
        std::cout << "MySql init error!" << std::endl;
    }
    sql = mysql_real_connect(
        sql, host, user, pwd, database_name, port, nullptr, 0);

    if (!sql) {
        std::cout << "MySql Connect error!" << std::endl;
    }
    return sql;
}

void TestFiberMysql() {
    auto        sql = ConnetcMysql();
    std::string command = "select * from user;";

    int ret = mysql_query(sql, command.c_str());  // 查询结果
    std::cout << command << std::endl;
    if (ret) {
        std::cout << "error!" << std::endl;
        printf("修改表数据失败！失败原因：%s\n", mysql_error(sql));
    } else {
        std::cout << "success!" << std::endl;
    }

    MYSQL_RES* res_ptr = mysql_store_result(sql);

    MYSQL_ROW sqlrow = mysql_fetch_row(res_ptr);  // 读取一行

    if (sqlrow) {

        for (unsigned int i = 0; i < mysql_num_fields(res_ptr); i++) {
            std::cout << sqlrow[i] << " ";
        }
        std::cout << std::endl;
    }
    mysql_free_result(res_ptr);
    mysql_close(sql);
}

void TestMain() {

    lane::IOManager iom(2, "test", false);
    iom.addTask([=]() {
        for (int i = 0; i < 2; ++i) {
            TestFiberMysql();
        }
    });
    std::cin.get();
}

int main() {
    TestMain();
}
