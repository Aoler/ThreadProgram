//
//  ThreadPool.h
//  ThreadPool
//
//  Created by Haitao on 25/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef ThreadPoolV1_h
#define ThreadPoolV1_h

#include <thread>
#include <future>
#include "SafeQueue.hpp"
#include "FunctionWrap.h"

class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
    threads(threads_)
    {}
    ~join_threads()
    {
        for(unsigned long i=0;i<threads.size();++i)
        {
            if(threads[i].joinable())
                threads[i].join();
        }
    }
};

/*
 当有异常抛出时，线程启动就会失败，所以需要保证任何已启动的线程都能停止，并且能在
 这种情况下清理干净。当有异常抛出时，通过使用try-catch来设置done标志⑩，还有
 join_threads类的实例(来自于第8章)③用来汇聚所有线程。当然也需要析构函数：仅设置done
 标志⑪，并且join_threads确保所有线程在线程池销毁前全部执行完成。注意成员声明的顺序
 很重要：done标志和worker_queue必须在threads数组之前声明，而数据必须在joiner前声
 明。这就能确保成员能以正确的顺序销毁；比如，所有线程都停止运行时，队列就可以安全
 的销毁了。
 
 worker_thread函数很简单：从任务队列上获取任务⑤，以及同时执行这些任务⑥，执行一个
 循环直到done标志被设置④。如果任务队列上没有任务，函数会调
 用 std::this_thread::yield() 让线程休息⑦，并且给予其他线程向任务队列上推送任务的机
 会。
*/

class thread_pool
{
private:
    std::atomic_bool done;
    thread_safe_queue<function_wrapper > work_queue; // 1
    std::vector<std::thread> threads; // 2
    join_threads joiner; // 3
    void worker_thread()
    {
        while(!done) // 4
        {
            function_wrapper task;
            if(work_queue.try_pop(task)) // 5
            {
                task(); // 6
            }
            else
            {
                std::this_thread::yield(); // 7
            }
        }
    }
public:
    thread_pool():
        done(false),joiner(threads)
    {
        unsigned int thread_count=std::thread::hardware_concurrency(); // 8
        try
        {
            for(unsigned i=0;i<thread_count;++i)
            {
                threads.push_back(std::thread(&thread_pool::worker_thread,this)); // 9
            }
        }
        catch(...)
        {
            done=true; // 10
            throw;
        }
    }
    ~thread_pool()
    {
        done=true; // 11
    }
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> // 1
     submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type; // 2
        std::packaged_task<result_type()> task(std::move(f)); // 3
        std::future<result_type> res(task.get_future()); // 4
        work_queue.push(std::move(task)); // 5 调用function_wrapper的模版移动构造函数
        return res; // 6
    }
};

#endif /* ThreadPoolV1_h */
