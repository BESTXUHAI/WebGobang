#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"
#include "../config.h"
#define NAMESIZE 512

using namespace std;

class Log
{
public:
    //C++11以后,使用局部变量懒汉不用加锁
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
    }
    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

private:
    Log();
    virtual ~Log();
    void *async_write_log()
    {
        string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
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
    locker m_mutex;
    int m_close_log; //关闭日志
};


//__VA_ARGS__为可变参数宏，用于指定可变参数...的位置，##代表可以忽略空参数
#define LOG_DEBUG(format, ...) if(0 == CLOSE_LOG) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();} \
else {printf(format, ##__VA_ARGS__);printf("\n");}
#define LOG_INFO(format, ...) if(0 == CLOSE_LOG) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();} \
else {printf(format, ##__VA_ARGS__);printf("\n");}
#define LOG_WARN(format, ...) if(0 == CLOSE_LOG) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();} \
else {printf(format, ##__VA_ARGS__);printf("\n");}
#define LOG_ERROR(format, ...) if(0 == CLOSE_LOG) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();} \
else {printf(format, ##__VA_ARGS__);printf("\n");}

#endif