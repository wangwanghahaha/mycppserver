#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
template <typename T>
class Threadpool
{
public:
    Threadpool(int actor_model, ConnectionPool *conn_pool, int thread_number = 8, int max_request = 10000);
    ~Threadpool();
    bool append(T *request, int state);
    bool appendP(T *request);

private:
    static void *worker(void *arg);
    void run();

private:
    int thread_number_;        //线程池中的线程数
    int max_requests_;         //请求队列中允许的最大请求数
    pthread_t *threads_;       //描述线程池的数组，大小为thread_number
    std::list<T *> workqueue_; //请求队列
    Locker queuelocker_;       //保护请求队列的互斥锁
    Sem queuestat_;            //是否有任务需要处理
    ConnectionPool *connPool_; //数据库
    int actor_model_;          //模型切换
};
template <typename T>
Threadpool<T>::Threadpool(int actor_model, ConnectionPool *conn_pool, int thread_number, int max_request)
{
    if (thread_number_ <= 0 || max_requests <= 0)
        throw std::exception();
    threads_ = new pthread_t[thread_number_];
    if (!threads_)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(threads_ + i, NULL, worker, this) != 0)
        {
            delete[] threads_;
            throw std::exception();
        }
        if (pthread_detach(threads_[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template <typename T>
Threadpool<T>::~Threadpool()
{
    delete[] threads_;
}
template <typename T>
bool Threadpool<T>::append(T *request, int state)
{
    queuelocker_.Lock();
    if (workqueue_.size() >= max_requests_)
    {
        queuelocker_.Unlock();
        return false;
    }
    request->state_ = state;
    workqueue_.push_back(request);
    queuelocker_.Unlock();
    queuestat_.post();
    return true;
}
template <typename T>
bool Threadpool<T>::appendP(T *request)
{
    queuelocker_.Lock();
    if (workqueue_.size() >= max_requests_)
    {
        queuelocker_.Unlock();
        return false;
    }
    workqueue_.push_back(request);
    queuelocker_.Unlock();
    queuestat_.post();
    return true;
}
template <typename T>
void *Threadpool<T>::worker(void *arg)
{
    Threadpool *pool = (Threadpool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void Threadpool<T>::run()
{
    while (true)
    {
        queuestat_.wait();
        queuelocker_.lock();
        if (workqueue_.empty())
        {
            queuelocker_.Unlock();
            continue;
        }
        T *request = workqueue_.front();
        workqueue_.pop_front();
        queuelocker_.Unlock();
        if (!request)
            continue;
        if (1 == actor_model_)
        {
            if (0 == request->state_)
            {
                if (0 == request->readOnce())
                {
                    request->improv = 1;
                    ConnectionRAII mysqlcon(&request->mysql, connPool_);
                    request->process();
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }else{
                if (request->Write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            ConnectionRAII mysqlcon(&request->mysql,connPool_);
            request->process();
        }
    }
}
#endif