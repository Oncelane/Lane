#include <base/mutex.h>
#include <mysql/mysql.h>

#include <iostream>
#include <queue>

#include "base/singleton.h"


namespace lane {
class MysqlManager {
public:
    const char* m_host = "127.0.0.1";         //主机名
    const char* m_user = "debian-sys-maint";  //用户名
    const char* m_pwd = "QTLVb6BaeeaJsFMT";   //密码
    const char* m_database_name = "yourdb";   //数据库名称
    int         m_port = 3306;                //端口号
private:
    std::queue<MYSQL*> m_freeSqls;
    Mutex              m_mu;


    MYSQL* createSql() {
        MYSQL* sql = nullptr;
        if (sql != nullptr) {
            return sql;
        }
        sql = mysql_init(sql);
        if (!sql) {
            std::cout << "MySql init error! str=" << mysql_error(sql)
                      << std::endl;
        }
        sql = mysql_real_connect(
            sql, m_host, m_user, m_pwd, m_database_name, m_port, nullptr, 0);

        if (!sql) {
            std::cout << "MySql Connect error!" << mysql_error(sql)
                      << std::endl;
        }
        return sql;
    }

public:
    MysqlManager() {
        int size = 10;
        for (int i = 0; i < size; ++i) {
            auto sql = createSql();
            m_freeSqls.emplace(sql);
        }
    }
    virtual ~MysqlManager() {
        while (!m_freeSqls.empty()) {
            auto sql = m_freeSqls.front();
            mysql_close(sql);
            m_freeSqls.pop();
        }
    }
    MYSQL* getSql() {
        Mutex::Lock lock(m_mu);
        if (m_freeSqls.empty()) {
            return nullptr;
        }
        auto ret = m_freeSqls.front();
        m_freeSqls.pop();
        return ret;
    }
    void putSql(MYSQL* sql) {
        Mutex::Lock lock(m_mu);
        m_freeSqls.push(sql);
    }
};
typedef lane::Singleton<MysqlManager> SqlMgr;
}  // namespace lane