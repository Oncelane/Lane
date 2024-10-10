/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-02-20 11:16
 * LastModified : 2023-02-20 11:16
 * Filename : fdmanager.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_FDMANAGER_H__
#define __LANE_FDMANAGER_H__
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include "base/hook.h"
#include "base/log.h"
#include "base/macro.h"
#include "base/mutex.h"
namespace lane {
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;

public:
    FdCtx(int fd);
    ~FdCtx(){};

private:
    void init();

public:
    void setUserNonBlock(bool v) {
        m_isUserNonBlock = v;
    }
    bool getUserNonBlock() const {
        return m_isUserNonBlock;
    }

    void setSysNoBlock(bool v) {
        m_isSysNonBlock = v;
    }
    bool getSysNonBlock() const {
        return m_isSysNonBlock;
    }

    bool isClose() const {
        return m_isClosed;
    }
    bool isSocket() const {
        return m_isSocket;
    }

    uint64_t getTimeout(int type) const;
    void     setTimeout(int type, uint64_t v);

private:
    bool m_isClosed : 1;
    bool m_isUserNonBlock : 1;
    bool m_isSysNonBlock : 1;
    bool m_isSocket : 1;

    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
    int      m_fd;
};

class FdManager {
public:
    typedef std::vector<FdCtx::ptr>    FdCtxVector;
    typedef std::shared_ptr<FdManager> ptr;
    using MutexType = SpinMutex;

public:
    FdManager();
    // ~FdManager();
    FdCtx::ptr get(int fd, bool autoCreate = false);
    void       del(int fd);

private:
    FdCtxVector m_fdCtxs;
    MutexType   m_mutex;
};
typedef Singleton<FdManager> FdMgr;
}  // namespace lane

#endif