#include "connection_pool.h"

connection_pool::connection_pool()
{
    m_CurConn = 0;
    m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(const char* url, const char* user, const char* passwd, const char* databasename, int port, int maxconn)
{
    m_MAXConn = maxconn;

    for (int i = 0; i < maxconn; i++)
    {
        MYSQL *conn = NULL;
        conn = mysql_init(conn);

        if (conn == NULL)
        {
            LOG_ERROR("MYSQL init error");
            exit(-1);
        }
        conn = mysql_real_connect(conn, url, user, passwd, databasename, port, NULL, 0);    

        if (conn == NULL)
        {
            LOG_ERROR("MYSQL connect error");
            exit(-1);
        }
        
        connList.push_back(conn);
        m_FreeConn++;
    }

    reserve = sem(m_FreeConn);    
    m_MAXConn = m_FreeConn;

}

MYSQL *connection_pool::GetConnection()
{
    MYSQL *conn = NULL;

    reserve.wait();    
    lock.lock();

    conn = connList.front();
    connList.pop_front();
    
    m_FreeConn--;
    m_CurConn++;

    lock.unlock();
    return conn;
}

bool connection_pool::ReleaseConnection(MYSQL *conn)
{
    if (conn == NULL)
    {
        return false;
    }

    lock.lock();
    connList.push_back(conn);
    m_FreeConn++;
    m_CurConn--;

    lock.unlock();
    reserve.post();    
    return true;
}

void connection_pool::DestoryPool()
{
    lock.lock();
    if (connList.size() > 0)
    {
        for (auto it = connList.begin();it != connList.end(); it++)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        m_CurConn = 0;
        m_FreeConn = 0;        
        connList.clear();
    }
    lock.unlock();    
}

int connection_pool::GetFreenum()
{
    return this->m_FreeConn;
}

connection_pool::~connection_pool()
{
    DestoryPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool)
{
    *SQL = connPool->GetConnection();
    connRAII = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(connRAII);
}
