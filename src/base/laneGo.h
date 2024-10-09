#ifndef __LANE_LANEGO_H__
#define __LANE_LANEGO_H__
#include <functional>

#include "base/iomanager.h"
#include "base/mutex.h"

#define co lane::_CO() -
#define defer lane::_DEFER() -
#define panic(str) lane::Fiber::GetThis()->_panic(str);
#define recovery() Fiber::GetBefore()->_recovery()

namespace lane {
struct _CO {
    void operator-(std::function<void()> cb) {
        lane::IOManager::GetThis()->addTask(cb);
    }
};

struct _DEFER {
    void operator-(std::function<void()> cb) {
        lane::Fiber::GetThis()->_defer(cb);
    }
};
}  // namespace lane


#endif
