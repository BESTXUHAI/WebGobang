#include "reactor.h"

int reactor::m_user_count = 0;

//数据库保护锁
locker reactor::db_lock;
//账号映射
std::map<string, string> reactor::map_users;
//匹配队列保护锁
locker reactor::qu_lock;
//匹配队列
std::queue<reactor *> reactor::mate_qt;
//程序运行时，将用户表预先读取到map中，提高效率
bool reactor::initmysql_result(connection_pool *connpool)
{
    MYSQL *mysql = NULL;
    //从线程池中取出一个连接
    connectionRAII conraii(&mysql, connpool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //返回结果
    MYSQL_RES *result = mysql_store_result(mysql);

    //将每一行数据保存到map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string user(row[0]);
        string passwd(row[1]);
        map_users[user] = passwd;
    }
    
}



void reactor::modevent(int events)
{
    epoll_event ev;
    ev.data.fd = m_fd;
    ev.events = events | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    m_events = ev.events;
    epoll_ctl(m_ept, EPOLL_CTL_MOD, m_fd, &ev);
}


reactor::reactor()
{
    m_fd = -1;
    m_ept = -1;
    m_events = 0;
    re_events = 0;
    writebuf_len = 0;
    readbuf_len = 0;
    bytes_have_send = 0;
    keepalive = false;

    writebuf = new char[BUF_SIZE];
    readbuf = new char[BUF_SIZE];

}

reactor::~reactor()
{
    delete [] writebuf;
    delete [] readbuf;
}


void reactor::init(int ept, int fd, int events)
{
    //已经被初始化
    if (m_fd != -1)
    {
        
        LOG_WARN("Already initialized");
        close(fd);
        return;
    }
    
    m_fd = fd;
    m_ept = ept;
    m_events = events;
    writebuf_len = 0;
    readbuf_len = 0;
    bytes_have_send = 0;
    keepalive = true;
    m_readbuf = readbuf;
    //初始化业务相关变量
    m_method = UNKNOWN;
    m_result = SUCCESS;
    m_host = NULL;
    m_rival = NULL;
    m_inqueue = false;
    isconnect = true;
    isopendatabase = OPEN_LOGJUDGE;

    //设置文件描述符为非阻塞
    int flag = fcntl(fd, F_GETFL);
    flag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);

    //添加事件
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    epoll_ctl(ept, EPOLL_CTL_ADD, fd, &ev);

 
    
}

void reactor::destory()
{

    //已经被删除
    if (m_fd == -1)
    {
        LOG_WARN("The connection has been closed");
        return;
    }

    //删除事件
    epoll_ctl(m_ept, EPOLL_CTL_DEL, m_fd, 0);
    
    //close一旦关闭,accept可能会提取相同的文件描述符
    int tfd = m_fd;
    m_fd = -1;
    close(tfd);

    isconnect = false;
    LOG_INFO("关闭连接:%d",tfd);

    //如果还在匹配对局中，则告知对手认输
    if (m_rival != NULL)
    {
        m_rival->rival_write(string("GAMERESULT\r\nSUCCESS\r\n"));
        m_rival = NULL;
    }
    
}

void reactor::setevents(int events)
{
    re_events = events;
}

bool reactor::dealread()
{
    //缓冲区已经满了
    if (readbuf_len >= BUF_SIZE)
    {
        LOG_WARN("Read buffer full!!!");
        return false;
    }
    
    //非阻塞读
    while (1)
    {
        int n = recv(m_fd, readbuf+readbuf_len, BUF_SIZE-readbuf_len, 0);
        if (n == -1)
        {
            //缓冲区已经没有数据
            if (errno == EAGAIN || errno == EWOULDBLOCK )
            {
                break;
            }
            //发生错误
            return false;
        }
        else if (n == 0)
        {
            //对方关闭连接
            return false;
        }
        readbuf_len += n;    
    }
    
    if (readbuf_len < BUF_SIZE)
    {
        readbuf[readbuf_len] = '\0';
    }
    LOG_DEBUG("get message:%s", readbuf);

    return true;
}

bool reactor::dealwrite()
{

    while (1)
    {
        if (writebuf_len  <= bytes_have_send)
        {
            //写事件响应完毕
            writebuf_len = 0;
            readbuf_len = 0;
            bytes_have_send = 0;
            if (keepalive)
            {
                //继续监听
                modevent(EPOLLIN);
            }
            else
            {
                //关闭连接
                destory();
            }

            LOG_INFO("response success");

            return true;
        }

        int n;
        //发送数据
        if (bytes_have_send < writebuf_len)
        {   
            n = send(m_fd, writebuf+bytes_have_send, writebuf_len-bytes_have_send, 0);
        }
         
        if (n <= -1)
        {
            //非阻塞
            if (errno == EAGAIN)
            {
                //继续监听读事件
                modevent(EPOLLOUT);
                return true;
            }
            
            return false;
        }
        LOG_INFO("send date bytes=%d", n);

        bytes_have_send += n;

    }
        

    return true;
}


//从缓冲区中获取一行
char* reactor::getline()
{
    char* linest = m_readbuf;
    if (m_readbuf >= readbuf + readbuf_len)
    {
        return NULL;
    }
    int len = readbuf + readbuf_len - m_readbuf;

    for (int i = 0;  i < len+readbuf_len; i++)
    {
        if (linest[i]=='\r'&&linest[i+1]=='\n')
        {
            //设置结束标志位
            linest[i] = '\0';
            m_readbuf += i+2;
            return linest;
        }
    }

    return NULL;
}


// 往写缓冲中写入待发送的数据
bool reactor::add_response( const char* format, ... ) {
    
    if( writebuf_len >= BUF_SIZE ) {
        return false;
    }
    va_list arg_list;
    va_start( arg_list, format );
    int len = vsnprintf( writebuf + writebuf_len, BUF_SIZE - 1 - writebuf_len, format, arg_list );
    if( len >= ( BUF_SIZE - 1 - writebuf_len ) ) {
        return false;
    }
    writebuf_len += len;
    va_end( arg_list );
    return true;
}



//解析读到的数据
bool reactor::process_read()
{
    m_readbuf = readbuf;
    char* text = getline();
    //数据不完整, 继续读取数据
    if (text == NULL )
    {
        return false;
    }
    m_method = parse_request_line(text);

    if (m_method == LOGIN)
    {
        char *userinfo = getline();
        if (!userinfo)
        {
            return false;
        }
        userdeal(userinfo);
    }
    else if (m_method == REGISTER)
    {
        char *userinfo = getline();
        if (!userinfo)
        {
            return false;
        }
        userdeal(userinfo);
    }
    else if (m_method == MATE)
    {
        mateplayer();
    }
    else if (m_method == MESSAGE)
    {
        if (m_rival == NULL)
        {
            LOG_ERROR("m_rival is null");
            return true;
        }
        
        char *message = getline();
        //落子信息转发给对方
        m_rival->rival_write(string("MESSAGE\r\n")+string(message)+"\r\n");
    }
    else if (m_method == GAMERESULT)
    {
        if (m_rival == NULL)
        {
            LOG_ERROR("m_rival is null");
            return true;
        }
        char *gameresult = getline();
        //由获胜的一方发出
        m_rival->rival_write(string("GAMERESULT\r\nFAIL\r\n"));
        m_rival = NULL;
    }


    return true;
}

//生成写数据
bool reactor::process_write()
{
    bool flag = true;
    writebuf_len = 0;
    switch (m_method)
    {
    case LOGIN:
        if (m_result == SUCCESS)
        {
            flag = add_response("LOGIN\r\nSUCCESS\r\n");
        }
        else
        {
            flag = add_response("LOGIN\r\nFAIL\r\n");
        }
        
        break;
    case REGISTER:
        if (m_result == SUCCESS)
        {
            flag = add_response("REGISTER\r\nSUCCESS\r\n");
        }
        else
        {
            flag = add_response("REGISTER\r\nFAIL\r\n");
        }
        break;
    case MATE:
        if (m_result == SUCCESS)
        {
            flag = add_response("MATE\r\nSECOND\r\n");
        }
        else
        {
            flag = add_response("MATE\r\nFAIL\r\n");
        }
        break;

    default:
        break;
    }
    LOG_INFO("Listening for write events, writebuflen=%d", writebuf_len);
    return flag;
}

void reactor::rival_write(string str)
{
    if (isconnect == false)
    {
        return;
    }
    
    writebuf_len = 0;
    bytes_have_send = 0;
    add_response(str.c_str());

    modevent(EPOLLOUT);
}

REQUEST_STATES  reactor::parse_request_line( char* text )
{
    if(strcasecmp(text, "LOGIN") == 0)
    {
        return LOGIN;
    }
    else if(strcasecmp(text, "REGISTER") == 0)
    {
        return REGISTER;
    }
    else if(strcasecmp(text, "MATE") == 0)
    {
        return MATE;
    }
    else if(strcasecmp(text, "MESSAGE") == 0)
    {
        return MESSAGE;
    }
    else if(strcasecmp(text, "GAMERESULT") == 0)
    {
        return GAMERESULT;
    }
    return UNKNOWN;
}

void reactor::mateplayer()
{
    //已经被配对
    if (m_rival != NULL)
    {
        m_result = SUCCESS;
        m_inqueue = false;
        return;    
    }
    //还在队列里面
    else if(m_inqueue)
    {
        m_result = FAIL;
        return;
    }

    qu_lock.lock();
    //队列没有等待玩家
    if (mate_qt.size() == 0)
    {
        mate_qt.push(this);
        m_inqueue = true;
        
    }
    else
    {
        while (!mate_qt.empty() && m_rival == NULL)
        {
            //匹配成功
            if (mate_qt.front()->isconnect)
            {
                m_rival = mate_qt.front();
                m_rival->m_rival = this;
            }
            
            mate_qt.pop();
        }
    }
    qu_lock.unlock();

    if (m_rival == NULL)
    {
        m_result = FAIL;
    }
    else
    {
        m_result = SUCCESS;
        //通知对手匹配成功,被唤醒的先手
        m_rival->rival_write("MATE\r\nFIRST\r\n");
    }
}

void reactor::process()
{

    if (re_events & EPOLLIN)
    {
        if(!dealread())
        {
            destory();
            LOG_ERROR("%s:errno is:%d", "dealread", errno);
 
            return;
        }
        if (process_read())
        {
            //解析完成
            readbuf_len = 0;
            m_readbuf = readbuf;
            writebuf_len = 0;
            if (process_write())
            {
                //有数据才写
                if (writebuf_len > 0)
                {
                    bytes_have_send = 0;
                    modevent(EPOLLOUT);
                }
            }
            else
            {
                LOG_ERROR("write out bufsize");
                destory();
            }
            
        }
        else
        {
            LOG_INFO("data not complete");
            //数据不完整继续等待数据
            modevent(EPOLLIN);
        }
    }
    else if (re_events & EPOLLOUT)
    {
        if(!dealwrite())
        {
            destory();
            LOG_ERROR("%s:errno is:%d", "dealwrite", errno);
            return;
        }

    }
}


bool reactor::userjudge(const char* user, const char* passwd)
{
    string ur(user);
    string pd(passwd);
    return map_users.count(ur)!=0 && map_users[ur] == pd;
}

bool reactor::register_user(const char* user, const char* passwd)
{
    string ur(user);
    if(map_users.count(ur) != 0)
        return false;

    MYSQL *mysql = NULL;
    //获取数据库连接实例
    connection_pool *connpool = connection_pool::GetInstance();
    //获取连接
    connectionRAII connraii(&mysql, connpool);
    char sql[200] = "";
    sprintf(sql, "INSERT INTO user(username, passwd) VALUES(\'%s\',\'%s\')", user, passwd);
    LOG_INFO("sql query:%s", sql);
    db_lock.lock();
    int res = mysql_query(mysql, sql);
    if (!res)
    {
        map_users[string(user)] = string(passwd);
    }
    db_lock.unlock();
    if (res != 0)
    {
        LOG_ERROR("insert into user failed");    
        return false;
    }
    
    return true;

}

void reactor::userdeal(char *userinfo)
{
    char *user = userinfo;
    char *passwd = strchr(userinfo, ' ')+1;
    *(passwd-1) = '\0';
    
    //未开启登录检验
    if (!isopendatabase)
    {
        m_result = SUCCESS;
        return;
    }
    

    if (m_method == LOGIN)
    {
        //账号密码正确
        if (userjudge(user, passwd))
        {
            m_result = SUCCESS;
        }
        else
        {
            m_result = FAIL;
        }
    }
    else
    {
        if (register_user(user, passwd))
        {
            m_result = SUCCESS;
        }
        else
        {
            m_result = FAIL;
            
        }
        
    }
}
    