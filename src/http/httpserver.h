/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-03-02 16:55
 * LastModified : 2023-03-02 16:55
 * Filename : httpserver.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_HTTP_SERVER_H__
#define __LANE_HTTP_SERVER_H__

#include "http/httpsession.h"
#include "http/servlet.h"
#include "net/tcpserver.h"

namespace lane {
namespace http {

/**
 * @brief HTTP服务器类
 */
class HttpServer : public TcpServer {
public:
    /// 智能指针类型
    typedef std::shared_ptr<HttpServer> ptr;

    /**
     * @brief 构造函数
     * @param[in] keepalive 是否长连接
     * @param[in] worker 工作调度器
     * @param[in] accept_worker 接收连接调度器
     */
    HttpServer(bool             keepalive = false,
               lane::IOManager* worker = lane::IOManager::GetThis(),
               lane::IOManager* io_worker = lane::IOManager::GetThis(),
               lane::IOManager* accept_worker = lane::IOManager::GetThis());

    /**
     * @brief 获取ServletDispatch
     */
    ServletDispatch::ptr getServletDispatch() const {
        return m_dispatch;
    }

    /**
     * @brief 设置ServletDispatch
     */
    void setServletDispatch(ServletDispatch::ptr v) {
        m_dispatch = v;
    }

    virtual void setName(const std::string& v) override;

protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    /// 是否支持长连接
    bool m_isKeepalive;
    /// Servlet分发器
    ServletDispatch::ptr m_dispatch;
};

}  // namespace http
}  // namespace lane

#endif
