/**
 * @file      gm_ring_buffer.h
 * @brief     环形缓冲区模块头文件
 * @author    huenrong (sgyhy1028@outlook.com)
 * @date      2026-02-16 16:26:04
 *
 * @copyright Copyright (c) 2026 huenrong
 *
 */

#ifndef __GM_RING_BUFFER_H
#define __GM_RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GM_RING_BUFFER_VERSION_MAJOR 1
#define GM_RING_BUFFER_VERSION_MINOR 0
#define GM_RING_BUFFER_VERSION_PATCH 0

// 内部需要预留一个字节（间隔元素），用于区分缓冲区空/满
#define RING_BUFFER_MAX_SIZE (SIZE_MAX - 1)

// 环形缓冲区对象
typedef struct _gm_ring_buffer gm_ring_buffer_t;

/**
 * @brief 创建环形缓冲区对象
 *
 * @return 成功: 环形缓冲区对象
 * @return 失败: NULL
 */
gm_ring_buffer_t *gm_ring_buffer_create(void);

/**
 * @brief 初始化环形缓冲区对象
 *
 * @note 1. 该函数支持重复调用，重复调用时会释放之前分配的内存，再分配新的内存
 *       2. 调用该函数前必须确保没有其它线程在操作该环形缓冲区对象，否则可能导致未定义行为
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 * @param[in]     size          : 缓冲区大小（不能超过 RING_BUFFER_MAX_SIZE）
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int gm_ring_buffer_init(gm_ring_buffer_t *gm_ring_buffer, const size_t size);

/**
 * @brief 销毁环形缓冲区对象
 *
 * @note 1. 调用该函数前必须确保没有其它线程在操作该环形缓冲区对象，否则可能导致未定义行为
 *       2. 销毁后，该环形缓冲区对象将不再可用
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int gm_ring_buffer_destroy(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 清空环形缓冲区
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int gm_ring_buffer_clear(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 获取环形缓冲区当前元素个数
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return >=0: 环形缓冲区当前元素个数
 * @return <0 : 失败
 */
ssize_t gm_ring_buffer_get_current_size(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 获取环形缓冲区总大小
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return >=0: 环形缓冲区总大小
 * @return <0 : 失败
 */
ssize_t gm_ring_buffer_get_total_size(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 判断环形缓冲区是否为空
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return true : 为空
 * @return false: 不为空
 */
bool gm_ring_buffer_is_empty(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 判断环形缓冲区是否已满
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 *
 * @return true : 已满
 * @return false: 未满
 */
bool gm_ring_buffer_is_full(gm_ring_buffer_t *gm_ring_buffer);

/**
 * @brief 写入数据到环形缓冲区
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 * @param[in]     data          : 待写入的数据
 * @param[in]     size          : 待写入的数据长度
 *
 * @return >=0: 实际写入的数据长度
 * @return <0 : 失败
 */
ssize_t gm_ring_buffer_write(gm_ring_buffer_t *gm_ring_buffer, const uint8_t *data, const size_t size);

/**
 * @brief 阻塞方式从环形缓冲区读取数据
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 * @param[out]    data          : 读取到的数据
 * @param[in]     size          : 指定读取的数据长度
 *
 * @return >=0: 实际读取到的数据长度
 * @return <0 : 失败
 */
ssize_t gm_ring_buffer_read(gm_ring_buffer_t *gm_ring_buffer, uint8_t *data, const size_t size);

/**
 * @brief 超时方式从环形缓冲区读取数据
 *
 * @note 超时时间如果为0，则表示不等待，立即返回
 *
 * @param[in,out] gm_ring_buffer: 环形缓冲区对象
 * @param[out]    data          : 读取到的数据
 * @param[in]     size          : 指定读取的数据长度
 * @param[in]     timeout_msec  : 超时时间(单位: ms)
 *
 * @return >0: 实际读取到的数据长度
 * @return 0 : 超时未接收到数据
 * @return <0: 失败
 */
ssize_t gm_ring_buffer_read_with_timeout(gm_ring_buffer_t *gm_ring_buffer, uint8_t *data, const size_t size,
                                         const uint32_t timeout_msec);

#ifdef __cplusplus
}
#endif

#endif // __GM_RING_BUFFER_H
