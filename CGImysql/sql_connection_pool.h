#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <mysql/mysql.h>
#include <string>
#include <list>
#include <iostream>
#include "../lock/locker.h"
#include "../log/log.h"
using namespace std;
class ConnectionPool
{
public:
    MYSQL *GetConnection();              //获取数据库连接
    bool ReleaseConnection(MYSQL *conn); //释放连接
    int GetFreeConn();                   //获取连接
    void DestroyPool();                  //销毁所有连接
    //单例模式
    static ConnectionPool *GetInstance();
    void init(string url, string user, string password, string database_name, int port, int maxconn, int close_log);

private:
    ConnectionPool();
    ~ConnectionPool();
    int maxconn_;
    int curconn_;
    int freeconn_;
    Locker lock;
    list<MYSQL *> conn_list;
    Sem reserve;

public:
    string url_;           //主机地址
    string port_;          //数据库端口号
    string user_;          //登陆数据库用户名
    string password_;      //登陆数据库密码
    string database_name_; //使用数据库名
    int m_close_log;        //日志开关
};
class ConnectionRAII
{
public:
    ConnectionRAII(MYSQL **SQL, ConnectionPool *conn_pool);
    ~ConnectionRAII();

private:
    MYSQL *conRAII_;
    ConnectionPool *poolRAII_;
};
#endif