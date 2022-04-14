#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"
/*
处理非活动连接的定时器
由于非活跃连接占用了连接资源，严重影响服务器的性能，
通过实现一个服务器定时器，处理这种非活跃连接，释放连
接资源。利用alarm函数周期性地触发SIGALRM信号,该信号
的信号处理函数利用管道通知主循环执行定时器链表上的定时任务.
*/
class UtilTimer;
struct client_data
{
    sockaddr_in address;
    int sockfd;
    UtilTimer *timer;
};
class UtilTimer
{
public:
    UtilTimer() : prev(NULL),next(NULL){};
public:
    time_t expire;
    void (* cb_func)(client_data *);
    client_data *user_data;
    UtilTimer *prev;
    UtilTimer *next;
};
class SortTimerList
{
public:
    SortTimerList();
    ~SortTimerList();

    void AddTimer(UtilTimer *timer);
    void AdjustTimer(UtilTimer *timer);
    void DelTimer(UtilTimer *timer);
    void Tick();
private:
    void AddTimer(UtilTimer *timer,UtilTimer *list_head);
    UtilTimer *head;
    UtilTimer *tail;
};
class Utils{
public:
    Utils(){};
    ~Utils(){};
    void init(int timslot);
    //对文件描述符设置非阻塞
    int SetNonblocking(int fd);
    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd,int fd,bool one_shot,int TRIGMode);
    //信号处理函数
    static void SigHandler(int sig);
     //设置信号函数
    void AddSig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void TimerHandler();

    void ShowError(int connfd, const char *info);
public:
    static int *pipe_fd_;
    SortTimerList timer_list_;
    static int epoll_fd_;
    int TIMESLOT_;
};
void CbFunc(client_data *user_data);
#endif