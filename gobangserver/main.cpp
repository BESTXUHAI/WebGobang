#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <stdlib.h>
#include "reactor.h"
#include "threadpool.h"
#include <signal.h>
#include "log/log.h"
#include "sql/connection_pool.h"


#define MAX_FD 65536   // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量
 
int stop_server = 0;

void addsig(int sig, void( handler )(int)){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    sigfillset( &sa.sa_mask );
    if(sigaction( sig, &sa, NULL ) == -1)
        exit(-1);
}

void shutdown(int sig)
{
    stop_server = 1;

}

void log_write(int m_close_log)
{
    //启动日志系统
    if (m_close_log == 0)
    {
        
        bool flag=Log::get_instance()->init("./ServerLog", 0, 2000, 800000, 800);
        if(!flag)
        {
            printf("log path not exist\n");
            exit(-1);
        }
        printf("open log system\n");
    }
    
}

void database_init(int open_database)
{
    if (open_database)
    {
        connection_pool *connpool=connection_pool::GetInstance();
        connpool->init("localhost", "root", "root", "webdb", 3306, 4);
        bool flag = reactor::initmysql_result(connpool);
        if (!flag)
        {
            printf("mysql connection error\n");
            exit(-1);
        }
        printf("create mysql connection pool\n");
    }


}

int main(int argc, char* argv[])
{

    if (argc <= 1)
    {
        printf("please input port\n");
        exit(-1);
    }
    
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    //设置端口复用
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //忽略SIGPIPE防止向被关闭的描述符写入数据，导致进程退出
    addsig(SIGPIPE, SIG_IGN);
    addsig(SIGINT, shutdown);


    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((unsigned short)atoi(argv[1]));
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(lfd, 5);

    int ept = epoll_create(5);
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENT_NUMBER];
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    epoll_ctl(ept, EPOLL_CTL_ADD, lfd, &ev);

    //创建反应堆
    reactor* reactors = new reactor[MAX_FD];
    //创建线程池
    threadpool<reactor>* pool = new threadpool<reactor>(8, 10000);

    //启动日志
    log_write(CLOSE_LOG);
    //启动数据库
    database_init(OPEN_LOGJUDGE);

    

    while (!stop_server)
    {
        int nready = epoll_wait(ept, events, MAX_EVENT_NUMBER, -1);
        if (nready < 0 && errno != EINTR)
        {
            printf("%s:errno is:%d", "epoll_wait", errno);
            exit(-1);
        }
        
        for (int i = 0; i < nready; i++)
        {
            if (events[i].data.fd == lfd)
            {
                if (events[i].events & EPOLLIN)
                {
                    struct sockaddr_in caddr;
                    socklen_t len = sizeof(caddr);
                    int cfd = accept(lfd, (struct sockaddr*)&caddr, &len);

                    if (cfd < 0)
                    {
                        LOG_ERROR("%s:errno is:%d", "accept", errno);
                        continue;
                    }
                    
                    //超过最大连接
                    if (cfd >= MAX_FD)
                    {
                        close(cfd);
                        LOG_WARN("Exceeding the maximum number of connections");
                       
                        continue;
                    }

                    //初始化连接
                    //EPOLLRDHUP可以检测对方是否关闭
                    //EPOLLONESHOT使事件只触发一次，防止多线程处理同一socket
                    //边沿触发,并设置非阻塞
                    reactors[cfd].init(ept, cfd, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT);
                }
                else
                {
                    printf("lfd events unknow\n");
                }
                
            }
            else
            {
                int cfd = events[i].data.fd;

                reactors[cfd].setevents(events[i].events);
                //加入线程池由线程处理
                if (events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ))
                {
                    //销毁连接
                    reactors[cfd].destory();
                }
                else
                {
                    pool->addtask(true, reactors+cfd);
                }

  
            }
            

        }
        

    }
    printf("stop server\n");
    close(lfd);
    close(ept);
    printf("close fd\n");
    delete [] reactors;
    printf("delete reactors\n");
    delete pool;
    printf("delete pool\n");
    return 0;
}

