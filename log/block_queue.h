/*
循环数组实现的阻塞队列
线程安全，每个操作前加锁
*/
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"
using namespace std;

template <class T>
class block_queue
{
private:
    Locker m_mutex;
    Cond m_cond;
    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;

public:
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            throw std::exception();
        }
        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }
    void clear()
    {
        m_mutex.Lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.Unlock();
    }
    ~block_queue()
    {
        m_mutex.Lock();
        if (m_array != NULL)
            delete[] m_array;

        m_mutex.Unlock();
    }
    bool IsFull()
    {
        m_mutex.Lock();
        if (m_size >= m_max_size)
        {
            m_mutex.Unlock();
            return true;
        }
        m_mutex.Unlock();
        return false;
    }
    bool IsEmpty()
    {
        m_mutex.Lock();
        if (0 == m_size)
        {
            m_mutex.Unlock();
            return true;
        }
        m_mutex.Unlock();
        return false;
    }
    bool front(T &value)
    {
        m_mutex.Lock();
        if (0 == m_size)
        {
            m_mutex.Unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.Unlock();
        return true;
    }
    bool back(T &value)
    {
        m_mutex.Lock();
        if (0 == m_size)
        {
            m_mutex.Unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.Unlock();
        return true;
    }
    int size()
    {
        int tmp = 0;
        m_mutex.Lock();
        tmp = m_size;
        m_mutex.Unlock();
        return tmp;
    }
    int max_size()
    {
        int tmp = 0;
        m_mutex.Lock();
        tmp = m_max_size;
        m_mutex.Unlock();
        return tmp;
    }
    bool Push(const T &item)
    {
        m_mutex.Lock();
        if (m_size >= m_max_size)
        {
            m_cond.Broadcast();
            m_mutex.Unlock();
            return false;
        }
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;

        m_size++;

        m_cond.Broadcast();
        m_mutex.Unlock();
        return true;
    }
    //没有元素，将等待
    bool Pop(T &item)
    {
        m_mutex.Lock();
        while (m_size <= 0)
        {
            if(!m_cond.Wait(m_mutex.get()))
            {
                m_mutex.Unlock();
                return false;
            }
        }
                m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.Unlock();
        return true;
    }
    //超时处理
    bool Pop(T &item, int ms_timeout)
    {
        struct timespec t = {0,0};
        struct timeval now = {0,0};
        gettimeofday(&now,NULL);
        m_mutex.Lock();
        if(m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.WaitUntilTime(m_mutex.get(), t))
            {
                m_mutex.Unlock();
                return false;
            }
        }
        if (m_size <=0){
            m_mutex.Unlock();
            return false;
        }
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.Unlock();
        return true;
    }
};
#endif