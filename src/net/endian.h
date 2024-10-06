/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-02-25 11:42
 * LastModified : 2023-02-25 11:42
 * Filename : endian.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_ENDIAN_H__
#define __LANE_ENDIAN_H__

#define LANE_LITTLE_ENDIAN 1
#define LANE_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace lane {

/**
 * @brief 8字节类型的字节序转化
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(
    T value) {
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(
    T value) {
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(
    T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define LANE_BYTE_ORDER LANE_BIG_ENDIAN
#else
#define LANE_BYTE_ORDER LANE_LITTLE_ENDIAN
#endif

#if LANE_BYTE_ORDER == LANE_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}  // namespace lane

#endif