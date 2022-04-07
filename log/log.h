#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"
using namespace std;
class Log
{

public:
    //懒汉模式
    static Log *GetInstance()
    {
        static Log instance;
        return &instance;
    }
    static void *FlushLogThread(void *args)
    {
        Log::GetInstance()->WriteLogAsync();
    }
    bool init(const char *file_name,int close_log,int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);
    void WriteLog(int level,const char *format,...);
    void Flush(void);
private:
    Log();
    virtual ~Log();
    void *WriteLogAsync()
    {
        string single_log;
        while(m_log_queue->Pop(single_log)){
            m_mutex.Lock();
            fputs(single_log.c_str(),m_fp);
            m_mutex.Unlock();
        }
    }
private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    block_queue<string> *m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位
    Locker m_mutex;
    int m_close_log; //关闭日志

};
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif