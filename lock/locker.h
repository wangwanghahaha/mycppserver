#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
//信号量
class Sem
{
private:
    sem_t m_sem;

public:
    Sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    Sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~Sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem)==0;
    }
    bool post()
    {
        return sem_post(&m_sem)==0;
    }


};

class Locker
{
private:
    pthread_mutex_t m_mutex;
public:
    Locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~Locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool Lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool Unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get_mutex()
    {
        return &m_mutex;
    }
};
class Cond
{
private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
public:
    Cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~Cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool Wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool WaitUntilTime(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool Signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool Broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }


};
#endif
