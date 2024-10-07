/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-03-05 09:53
 * LastModified : 2023-03-05 09:53
 * Filename : worker.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_WORKER_H__
#define __LANE_WORKER_H__

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/singleton.h"

namespace lane {

class WorkerManager {
public:
    WorkerManager();
    void           add(Scheduler::ptr s);
    Scheduler::ptr get(const std::string& name);
    IOManager::ptr getAsIOManager(const std::string& name);

    template <class FiberOrCb>
    void schedule(const std::string& name, FiberOrCb fc, int thread = -1) {
        auto s = get(name);
        if (s) {
            s->schedule(fc, thread);
        } else {
            static lane::Logger::ptr s_logger = LANE_LOG_NAME("system");
            LANE_LOG_ERROR(LANE_LOG_NAME("system"))
                << "schedule name=" << name << " not exists";
        }
    }

    template <class Iter>
    void schedule(const std::string& name, Iter begin, Iter end) {
        auto s = get(name);
        if (s) {
            s->schedule(begin, end);
        } else {
            static lane::Logger::ptr s_logger = LANE_LOG_NAME("system");
            LANE_LOG_ERROR(LANE_LOG_NAME("system"))
                << "schedule name=" << name << " not exists";
        }
    }

    bool init();
    bool init(
        const std::map<std::string, std::map<std::string, std::string> >& v);
    void stop();

    bool isStoped() const {
        return m_stop;
    }
    std::ostream& dump(std::ostream& os);

    uint32_t getCount();

private:
    std::map<std::string, std::vector<Scheduler::ptr> > m_datas;
    bool                                                m_stop;
};

typedef lane::Singleton<WorkerManager> WorkerMgr;

}  // namespace lane

#endif
