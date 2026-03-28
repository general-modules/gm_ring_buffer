/**
 * @file      gm_ring_buffer.c
 * @brief     环形缓冲区模块源文件
 * @author    huenrong (sgyhy1028@outlook.com)
 * @date      2026-02-16 16:25:57
 *
 * @copyright Copyright (c) 2026 huenrong
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "gm_ring_buffer.h"

// 环形缓冲区对象
struct _gm_ring_buffer_t
{
    uint8_t *buffer;       // 指向缓冲区的指针
    size_t head;           // 头指针(指向头元素)
    size_t tail;           // 尾指针(指向尾元素的下一个位置)
    size_t total_size;     // 缓冲区的总大小
    size_t current_size;   // 当前大小
    pthread_mutex_t mutex; // 互斥锁
    pthread_cond_t cond;   // 条件变量
};

gm_ring_buffer_t *gm_ring_buffer_create(void)
{
    gm_ring_buffer_t *gm_ring_buffer = (gm_ring_buffer_t *)malloc(sizeof(gm_ring_buffer_t));
    if (gm_ring_buffer == NULL)
    {
        return NULL;
    }

    gm_ring_buffer->buffer = NULL;
    gm_ring_buffer->head = 0;
    gm_ring_buffer->tail = 0;
    gm_ring_buffer->total_size = 0;
    gm_ring_buffer->current_size = 0;
    pthread_mutex_init(&gm_ring_buffer->mutex, NULL);

    return gm_ring_buffer;
}

int gm_ring_buffer_init(gm_ring_buffer_t *gm_ring_buffer, const size_t size)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    if (size == 0)
    {
        return -2;
    }

    if (size >= RING_BUFFER_MAX_SIZE)
    {
        return -3;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    if (gm_ring_buffer->buffer != NULL)
    {
        free(gm_ring_buffer->buffer);
        gm_ring_buffer->buffer = NULL;
    }

    // 计算需要分配的内存空间。需要多加一个间隔元素，用于区分空/满
    size_t buffer_size = size + 1;
    gm_ring_buffer->buffer = (uint8_t *)malloc(buffer_size);
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -4;
    }
    gm_ring_buffer->head = 0;
    gm_ring_buffer->tail = 0;
    gm_ring_buffer->total_size = buffer_size;
    gm_ring_buffer->current_size = 0;
    if (pthread_cond_init(&gm_ring_buffer->cond, NULL) != 0)
    {
        free(gm_ring_buffer->buffer);
        gm_ring_buffer->buffer = NULL;
        gm_ring_buffer->total_size = 0;
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -5;
    }

    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return 0;
}

int gm_ring_buffer_destroy(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);

    bool inited = (gm_ring_buffer->buffer != NULL);
    if (inited)
    {
        // 广播条件变量，唤醒所有等待的线程
        pthread_cond_broadcast(&gm_ring_buffer->cond);

        free(gm_ring_buffer->buffer);
        gm_ring_buffer->buffer = NULL;
        gm_ring_buffer->head = 0;
        gm_ring_buffer->tail = 0;
        gm_ring_buffer->total_size = 0;
        gm_ring_buffer->current_size = 0;
    }

    pthread_mutex_unlock(&gm_ring_buffer->mutex);
    if (inited)
    {
        pthread_cond_destroy(&gm_ring_buffer->cond);
    }
    pthread_mutex_destroy(&gm_ring_buffer->mutex);
    free(gm_ring_buffer);

    return 0;
}

int gm_ring_buffer_clear(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -2;
    }

    gm_ring_buffer->head = 0;
    gm_ring_buffer->tail = 0;
    gm_ring_buffer->current_size = 0;
    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return 0;
}

ssize_t gm_ring_buffer_get_current_size(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    size_t current_size = gm_ring_buffer->current_size;
    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return (ssize_t)current_size;
}

ssize_t gm_ring_buffer_get_total_size(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    size_t total_size = gm_ring_buffer->total_size;
    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return (ssize_t)total_size;
}

bool gm_ring_buffer_is_empty(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    bool is_empty = (gm_ring_buffer->current_size == 0);
    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return is_empty;
}

bool gm_ring_buffer_is_full(gm_ring_buffer_t *gm_ring_buffer)
{
    if (gm_ring_buffer == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);

    // 确保缓冲区已初始化，以免 total_size 为 0 导致除零错误
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return false;
    }

    bool is_full = ((gm_ring_buffer->tail + 1) % gm_ring_buffer->total_size == gm_ring_buffer->head);
    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return is_full;
}

ssize_t gm_ring_buffer_write(gm_ring_buffer_t *gm_ring_buffer, const uint8_t *data, const size_t size)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    if (data == NULL)
    {
        return -2;
    }

    if (size == 0)
    {
        return -3;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -4;
    }

    size_t written_size = 0;
    for (; written_size < size; written_size++)
    {
        // 每次写入数据前，先判断缓冲区是否已满
        if ((gm_ring_buffer->tail + 1) % gm_ring_buffer->total_size == gm_ring_buffer->head)
        {
            break;
        }

        gm_ring_buffer->buffer[gm_ring_buffer->tail] = data[written_size];
        gm_ring_buffer->tail = (gm_ring_buffer->tail + 1) % gm_ring_buffer->total_size;
        gm_ring_buffer->current_size++;
    }

    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    // 写入数据后，通知读线程
    if (written_size > 0)
    {
        pthread_cond_signal(&gm_ring_buffer->cond);
    }

    return written_size;
}

ssize_t gm_ring_buffer_read(gm_ring_buffer_t *gm_ring_buffer, uint8_t *data, const size_t size)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    if (data == NULL)
    {
        return -2;
    }

    if (size == 0)
    {
        return -3;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -4;
    }

    memset(data, 0, size);

    // 使用 while 循环，防止虚假唤醒。具体原因可可参考《Linux/Unix 系统编程手册》第 30.2.3 节
    while (gm_ring_buffer->current_size == 0)
    {
        pthread_cond_wait(&gm_ring_buffer->cond, &gm_ring_buffer->mutex);
    }

    size_t read_size = 0;
    for (; read_size < size; read_size++)
    {
        // 没有数据了
        if (gm_ring_buffer->current_size == 0)
        {
            break;
        }

        data[read_size] = gm_ring_buffer->buffer[gm_ring_buffer->head];
        gm_ring_buffer->head = (gm_ring_buffer->head + 1) % gm_ring_buffer->total_size;
        gm_ring_buffer->current_size--;
    }

    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return read_size;
}

ssize_t gm_ring_buffer_read_with_timeout(gm_ring_buffer_t *gm_ring_buffer, uint8_t *data, const size_t size,
                                         const uint32_t timeout_msec)
{
    if (gm_ring_buffer == NULL)
    {
        return -1;
    }

    if (data == NULL)
    {
        return -2;
    }

    if (size == 0)
    {
        return -3;
    }

    pthread_mutex_lock(&gm_ring_buffer->mutex);
    if (gm_ring_buffer->buffer == NULL)
    {
        pthread_mutex_unlock(&gm_ring_buffer->mutex);

        return -4;
    }

    memset(data, 0, size);

    if (timeout_msec > 0)
    {
        struct timespec start_time = {0};
        int ret = clock_gettime(CLOCK_REALTIME, &start_time);
        if (ret != 0)
        {
            pthread_mutex_unlock(&gm_ring_buffer->mutex);

            return -5;
        }

        struct timespec end_time = {0};
        end_time.tv_sec = start_time.tv_sec + timeout_msec / 1000;
        end_time.tv_nsec = start_time.tv_nsec + (timeout_msec % 1000) * 1000000;

        // tv_nsec 必须小于 1s
        if (end_time.tv_nsec >= 1000000000)
        {
            end_time.tv_sec += 1;
            end_time.tv_nsec -= 1000000000;
        }

        // 没有数据时，才超时等待信号
        // 使用 while 循环，防止虚假唤醒。具体原因可可参考《Linux/Unix 系统编程手册》第 30.2.3 节
        while (gm_ring_buffer->current_size == 0)
        {
            // 超时等待信号
            ret = pthread_cond_timedwait(&gm_ring_buffer->cond, &gm_ring_buffer->mutex, &end_time);
            // 已超时，退出
            if (ret == ETIMEDOUT)
            {
                pthread_mutex_unlock(&gm_ring_buffer->mutex);

                return 0;
            }
        }
    }

    size_t read_size = 0;
    for (; read_size < size; read_size++)
    {
        // 没有数据了
        if (gm_ring_buffer->current_size == 0)
        {
            break;
        }

        data[read_size] = gm_ring_buffer->buffer[gm_ring_buffer->head];
        gm_ring_buffer->head = (gm_ring_buffer->head + 1) % gm_ring_buffer->total_size;
        gm_ring_buffer->current_size--;
    }

    pthread_mutex_unlock(&gm_ring_buffer->mutex);

    return read_size;
}
