#include "sql_connection_pool.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <string>
#include <list>
#include <iostream>

using namespace std;

ConnectionPool::ConnectionPool()
{
    curconn_ = 0;
    freeconn_ = 0;
}
ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool conn_pool;
    return &conn_pool;
}
void ConnectionPool::init(string url, string user, string password, string database_name, int port, int maxconn, int close_log)
{
    url_ = url;
    port_ = port;
    user_ = user;
    password_ = password;
    database_name_ = database_name;
    m_close_log = close_log;
    for(int i = 0; i<maxconn;i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);
        if (con == NULL)
        {
            LOG_ERROR("MYSQL Error");
            exit(1);
        }
        con = mysql_real_connect(con,url.c_str(),user.c_str(),password.c_str(),database_name.c_str(),port,NULL,0);
        if(con == NULL)
        {
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        conn_list.push_back(con);
        ++freeconn_;
    }
    reserve = Sem(freeconn_);
    maxconn_ = freeconn_;

}
MYSQL *ConnectionPool::GetConnection()
{
    MYSQL *con = NULL;
    if(0==conn_list.size())
        return NULL;
    reserve.wait();
    lock.Lock();
    con=conn_list.front();
    conn_list.pop_front();
    --freeconn_;
    ++curconn_;
    lock.Unlock();
    return con;
}
//释放当前使用的连接
bool ConnectionPool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;

	lock.Lock();

	conn_list.push_back(con);
	++freeconn_;
	--curconn_;

	lock.Unlock();

	reserve.post();
	return true;
}
int ConnectionPool::GetFreeConn()
{
    return this->freeconn_;
}
ConnectionPool::~ConnectionPool(){
    DestroyPool();
}
ConnectionRAII::ConnectionRAII(MYSQL **SQL, ConnectionPool *conn_pool)
{
    *SQL = conn_pool->GetConnection();
    conRAII_ = *SQL;
    poolRAII_ = conn_pool;

}
ConnectionRAII::~ConnectionRAII(){
    poolRAII_->ReleaseConnection(conRAII_);
}