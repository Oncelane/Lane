/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-01-27 11:20
 * LastModified : 2023-01-27 11:20
 * Filename : singleton.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_SINGLETON_H__

#define __LANE_SINGLETON_H__

namespace lane {
template <typename T>
class Singleton {
public:
    static T* GetInstance() {
        static T instance;
        return &instance;
    }
};

template <typename T>
class SingletonPtr {
public:
    static typename T::ptr GetInstance() {
        static typename T::ptr instance(new T());
        return instance;
    }
};
}  // namespace lane

#endif