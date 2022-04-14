#include "lst_timer.h"
#include "../http/http_conn.h"

SortTimerList::SortTimerList()
{
    head = NULL;
    tail = NULL;
}
SortTimerList::~SortTimerList()
{
    UtilTimer *tmp = head;
    while(tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}
void SortTimerList::AddTimer(UtilTimer *timer)
{
    if(!timer)
    {
        return;
    }
    if(!head)
    {
        head = tail = timer;
        return;
    }
    if(timer->expire > head->expire)
    {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    AddTimer(timer,head);
}
void SortTimerList::AdjustTimer(UtilTimer *timer)
{
    if(!timer)
    {
        return;
    }
    UtilTimer *tmp =timer->next;
    if(!tmp || (timer->expire < tmp->expire))
    {
        return;
    }
    if(timer == head)
    {
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        AddTimer(timer,head);

    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        AddTimer(timer, timer->next);
    }
}
void SortTimerList::DelTimer(UtilTimer *timer)
{
    if(!timer)
    {
        return;
    }
    if((timer == head) && (timer == tail))
    {
        delete timer;
        head == NULL;
        tail == NULL;
        return;
    }
    if(timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;

}
void SortTimerList::Tick()
{
    if(!head)
    {
        return;
    }
    time_t cur = time(NULL);
    UtilTimer *tmp = head;
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            break;
        }
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}
void Utils::init(int timeslot)
{
    TIMESLOT_ = timeslot;
}
//对文件描述符设置非阻塞
int Utils::SetNonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
void Utils::addfd(int epollfd,int fd,bool one_shot,int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;
    if(1==TRIGMode)
    {
       event.events = EPOLLIN | EPOLLET |EPOLLRDHUP;
       
    }
    else
    {
        event.events = EPOLLIN |EPOLLRDHUP;
    }
    if(one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    SetNonblocking(fd);
}
//信号处理函数
void Utils::SigHandler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(pipe_fd_[1],(char *)&msg,1,0);
    errno = save_errno;
}
//设置信号处理函数
void Utils::AddSig(int sig,void(handler)(int),bool restart)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL)!=-1);
}
//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::TimerHandler()
{
    timer_list_.Tick();
    alarm(TIMESLOT_);
}
void Utils::ShowError(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::pipe_fd_ = 0;
int Utils::epoll_fd_ = 0;

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::epoll_fd_, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    HttpConn::user_count_--;
}
