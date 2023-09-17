#ifndef CONNECTION_POLL_H
#define CONNECTION_POLL_H
#include <stdio.h>
#include <iostream>
#include <mysql/mysql.h>
#include <string.h>
#include <list>
#include "../locker.h"
#include "../log/log.h"




class connection_pool
{

public:
    MYSQL *GetConnection(); //获取数据库连接
    bool ReleaseConnection(MYSQL *conn);//释放数据库连接
    void DestoryPool(); //销毁所有连接
    int GetFreenum();//获取空闲连接数
    static connection_pool * GetInstance();//单例模式，获取对象
    //初始化
    void init(const char* url, const char* user, const char* passwd, const char* databasename, int port, int maxconn);
    



private:
    connection_pool(/* args */);
    ~connection_pool();

    int m_MAXConn; //最大连接数
    int m_CurConn; //当前使用的连接数
    int m_FreeConn;//当前空闲连接处
    locker lock;
    std::list<MYSQL *> connList;//连接池
    sem reserve;

};

//创建一个RAII类，避免手动释放数据库连接
class connectionRAII
{
private:
    MYSQL *connRAII;
    connection_pool* poolRAII;
public:
    connectionRAII(MYSQL **SQL, connection_pool *connPool);
    ~connectionRAII();
};



#endif