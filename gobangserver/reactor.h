#ifndef REACTOR_H
#define REACTOR_H
#include <queue>
#include <map>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include "log/log.h"
#include "sql/connection_pool.h"

#define BUF_SIZE 1024

/*
    GAME请求方法
    LOGIN：请求登录
    REGISTER：请求注册
    MATE：匹配对局
    MESSAGE：对局信息
    GAMERESULT：对局结果

*/
enum REQUEST_STATES {LOGIN = 0, REGISTER, MATE, MESSAGE, GAMERESULT, UNKNOWN};
enum REQUEST_RESULT {SUCCESS = 0, FAIL};


//负责事件的处理
class reactor
{

public:
    

private:
    static int m_user_count;//当前用户数量
    //static pthread_mutex_t mutex;//保护m_user_count


    /* data */
    int m_fd;
    int m_ept;
    int m_events;
    int re_events;//触发的事件
    int writebuf_len;//要发送的数据长度
    int bytes_have_send;//已经发送的数据长度

    int readbuf_len;//读到的数据长度
    //缓存数组地址
    char* writebuf;
    char* readbuf;
    //读写指针
    char *m_readbuf;

    //是否保持连接
    bool keepalive;

    //请求类型
    REQUEST_STATES m_method;
    //保存请求的主机地址
    char* m_host;

    //请求处理结果
    REQUEST_RESULT m_result; 
    //数据库保护锁
    static locker db_lock;
    //账号映射
    static std::map<string, string> map_users;
    //匹配队列保护锁
    static locker qu_lock;
    //匹配队列
    static std::queue<reactor *> mate_qt;
    //对手
    reactor *m_rival;
    //是否在匹配队列
    bool m_inqueue;
    //连接未关闭
    bool isconnect;
    //是否打开数据库
    bool isopendatabase;
public:
    reactor();
    ~reactor();


    //修改事件
    void modevent(int events);
    //初始化连接并将事件加入事件树
    void init(int ept, int fd, int events);
    //销毁连接并删除事件
    void destory();
    //设置监听到的事件
    void setevents(int events);


    //线程池执行的函数
    void process();
    //解析报文
    bool process_read();
    //回复响应报文
    bool process_write();
    //供敌方调用的写数据接口
    void rival_write(string str);
    //初始化账号表
    static bool initmysql_result(connection_pool *connpool);


private:
    //读写事件处理函数
    bool dealread();
    bool dealwrite();

    //查找数据库中账号是否存在
    bool userjudge(const char* user, const char* passwd);
    bool register_user(const char* user, const char* passwd);
    //从缓冲区中读取一行
    char* getline();
    //初始化各种成员
    void readinit();
    //解析请求头
    REQUEST_STATES  parse_request_line( char* text );
    //处理注册和登录
    void userdeal(char* userinfo);
    //匹配对局
    void mateplayer();
    //往缓冲区写入数据
    bool add_response( const char* format, ... );
};





#endif