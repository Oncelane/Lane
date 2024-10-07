/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-03-05 10:32
 * LastModified : 2023-03-05 10:32
 * Filename : library.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_LIBRARY_H__
#define __LANE_LIBRARY_H__

#include <memory>

#include "init/module.h"

namespace lane {

class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};

}  // namespace lane

#endif
