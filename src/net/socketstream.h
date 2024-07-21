/*******************************************
 * Author : Lane
 * Email: : 1981811204@qq.com
 * CreateTime : 2023-02-27 15:42
 * LastModified : 2023-02-27 15:42
 * Filename : socketstream.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_SOCKET_STREAM_H__
#define __LANE_SOCKET_STREAM_H__

#include "base/iomanager.h"
#include "base/mutex.h"
#include "net/socket.h"
#include "net/stream.h"

namespace lane {

    /**
     * @brief Socket流
     */
    class SocketStream : public Stream {
       public:
        typedef std::shared_ptr<SocketStream> ptr;

        /**
         * @brief 构造函数
         * @param[in] sock Socket类
         * @param[in] owner 是否完全控制
         */
        SocketStream(Socket::ptr sock, bool owner = true);

        /**
         * @brief 析构函数
         * @details 如果m_owner=true,则close
         */
        ~SocketStream();

        /**
         * @brief 读取数据
         * @param[out] buffer 待接收数据的内存
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int read(void* buffer, size_t length) override;

        /**
         * @brief 读取数据
         * @param[out] ba 接收数据的ByteArray
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int read(ByteArray::ptr ba, size_t length) override;

        /**
         * @brief 写入数据
         * @param[in] buffer 待发送数据的内存
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int write(const void* buffer, size_t length) override;

        /**
         * @brief 写入数据
         * @param[in] ba 待发送数据的ByteArray
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int write(ByteArray::ptr ba, size_t length) override;

        /**
         * @brief 关闭socket
         */
        virtual void close() override;

        /**
         * @brief 返回Socket类
         */
        Socket::ptr getSocket() const {
            return m_socket;
        }

        /**
         * @brief 返回是否连接
         */
        bool isConnected() const;

        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();
        std::string getRemoteAddressString();
        std::string getLocalAddressString();

       protected:
        /// Socket类
        Socket::ptr m_socket;
        /// 是否主控
        bool m_owner;
    };

}  // namespace lane

#endif
