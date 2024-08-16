/*******************************************
 * Author : Lane
 * Email: : 1981811204@qq.com
 * CreateTime : 2023-02-01 10:22
 * LastModified : 2023-02-01 10:22
 * Filename : macro.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_MACRO_H__
#define __LANE_MACRO_H__

#include <assert.h>
#include <string.h>

#include "base/log.h"
#include "base/util.h"

/// 断言宏封装
#define LANE_ASSERT_ON false

#define LANE_ASSERT(x)                                      \
    if (LANE_ASSERT_ON) {                                   \
        if (!(x)) {                                         \
            LANE_LOG_ERROR(LANE_LOG_ROOT())                 \
                << "ASSERTION: " #x << "\nbacktrace:\n"     \
                << lane::BacktraceToString(100, 2, "    "); \
            assert(x);                                      \
        }                                                   \
    }


/// 断言宏封装

#define LANE_ASSERT2(x, w)                                  \
    if (LANE_ASSERT_ON) {                                   \
        if (!(x)) {                                         \
            LANE_LOG_ERROR(LANE_LOG_ROOT())                 \
                << "ASSERTION: " #x << "\n"                 \
                << w << "\nbacktrace:\n"                    \
                << lane::BacktraceToString(100, 2, "    "); \
            assert(x);                                      \
        }                                                   \
    }

#endif
