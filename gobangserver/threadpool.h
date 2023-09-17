#ifndef THREADPOOL_H
#define THREADPOLL_H

#include <pthread.h>
#include <exception>
#include <cstdio>
#include <queue>



template<typename T>
class threadpool
{

private:
    //任务队列锁
    pthread_mutex_t mutex;
    //任务存在,可取任务
    pthread_cond_t full;
    //存在空位,可以添加任务
    pthread_cond_t empty;

    //线程数量
    int pthread_number;
    //最大任务队列
    int max_tasks_number;
    //任务队列
    std::queue<T*> task_queue;
    //线程数组
    pthread_t* threads;

    //退出线程标志
    bool shutdown;


public:
    threadpool(int pth_number = 8, int mtasks_number = 10000);
    ~threadpool();

    //添加任务，相当于生产者
    bool addtask(bool isblock, T* task);
    //让所有线程退出
    void destory();
    //实际执行函数
    void run();

private:
    //线程回调函数, 相当于消费者
    static void* worker(void* arg);

};

//申请线程数组，初始化锁和信号
//gcc中参数不能和成员变量同名
template<typename T>
threadpool<T>::threadpool(int pth_number, int mtasks_number)
{

    if (pth_number <= 0 || mtasks_number <= 0)
    {
        throw std::exception();
    }
    

    shutdown = false;
    pthread_number = pth_number;
    max_tasks_number = mtasks_number;

    //注意：一定要在线程创建前初始化锁和信号量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&full, NULL);
    pthread_cond_init(&empty, NULL);


    threads = new pthread_t[pthread_number];

    for (int i = 0; i < pthread_number; i++)
    {
        //注意：回调函数只能传入静态成员函数, 将线程池作为参数传入
        if (pthread_create(threads+i, NULL, worker, this) < 0)
        {
            throw std::exception();
        }
        if (pthread_detach(threads[i]) < 0)
        {
            throw std::exception();
        }
        
        printf("create pthread%d\n", i+1);
    }
    



}

//销毁数组，锁和信号
template<typename T>
threadpool<T>::~threadpool()
{
    destory();
    delete [] threads;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&full);
    pthread_cond_destroy(&empty);
}

template<typename T>
bool threadpool<T>::addtask(bool isblock, T* task)
{
    //先抢锁
    pthread_mutex_lock(&mutex);
    
    while (task_queue.size() + 1 > max_tasks_number)
    {
        //非阻塞
        if (!isblock)
        {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        //等待空闲位置
        pthread_cond_wait(&empty, &mutex);
    }
    //printf("task number%lu\n", task_queue.size());
    task_queue.push(task);
    pthread_mutex_unlock(&mutex);
    //发送任务信号
    pthread_cond_signal(&full);

    return true;

}

template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = (threadpool*)arg;
    pool->run();
    printf("pthread exit\n");
}

template<typename T>
void threadpool<T>::destory()
{
    shutdown = true;

    //唤醒所有线程
    pthread_cond_broadcast(&full);

}

template<typename T>
void threadpool<T>::run()
{
    T* task = nullptr;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        //等待任务或者终止信号
        while(!shutdown && task_queue.size() == 0)
        {
            pthread_cond_wait(&full, &mutex);
        }
        
        //销毁时记得释放锁
        if (shutdown)
        {
            
            pthread_mutex_unlock(&mutex);
            break;
        }
        else
        {
            task = task_queue.front();
            task_queue.pop();
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&empty);
        //执行任务处理程序
        task->process();
        
    }

}







#endif