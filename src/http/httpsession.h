/*******************************************
 * Author : Lane
 * Email: : 1981811204@qq.com
 * CreateTime : 2023-02-28 17:06
 * LastModified : 2023-02-28 17:06
 * Filename : httpsession.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_HTTP_SESSION_H__
#define __LANE_HTTP_SESSION_H__

#include "http/http.h"
#include "net/socketstream.h"

namespace lane {
    namespace http {

        /**
         * @brief HTTPSession封装
         */
        class HttpSession : public SocketStream {
           public:
            /// 智能指针类型定义
            typedef std::shared_ptr<HttpSession> ptr;

            /**
             * @brief 构造函数
             * @param[in] sock Socket类型
             * @param[in] owner 是否托管
             */
            HttpSession(Socket::ptr sock, bool owner = true);

            /**
             * @brief 接收HTTP请求
             */
            HttpRequest::ptr recvRequest();

            /**
             * @brief 发送HTTP响应
             * @param[in] rsp HTTP响应
             * @return >0 发送成功
             *         =0 对方关闭
             *         <0 Socket异常
             */
            int sendResponse(HttpResponse::ptr rsp);
        };

    }  // namespace http
}  // namespace lane

#endif
