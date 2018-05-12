//
//  main.cpp
//  packaged_task
//
//  Created by Haitao on 1/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#include <string>
#include <iostream>       // std::cout
#include <functional>     // std::ref
#include <thread>         // std::thread
#include <future>         // std::promise, std::future
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

/*
 
 线程间传递任务
 很多图形架构需要特定的线程去更新界面，所以当一个线程需要界面的更新时，它需要发出
 一条信息给正确的线程，让特定的线程来做界面更新。 std::packaged_task 提供了完成这种
 功能的一种方法，且不需要发送一条自定义信息给图形界面相关线程。下面来看看代码。
 
*/

std::mutex m;
std::deque<std::packaged_task<void()> > tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread() // 1
{
    while(!gui_shutdown_message_received()) // 2
    {
        get_and_process_gui_message(); // 3
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m);
            if(tasks.empty()) // 4
                continue;
            task=std::move(tasks.front()); // 5
            tasks.pop_front();
        }
        task(); // 6
    }
}

std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f); // 7
    std::future<void> res=task.get_future(); // 8
    std::lock_guard<std::mutex> lk(m); // 9
    tasks.push_back(std::move(task)); // 10
    return res;
}




int task(std::string str)
{
    std::cout << str <<std::endl;
    return 0;
}

int main(int argc, const char * argv[])
{
    std::packaged_task<int(std::string)> myPackaged(task);//首先创建packaged_task对象myPackaged其内部创建一个函数task和一个共享状态(用于返回task的结果)
    std::future<int> myFuture=myPackaged.get_future();//通过packaged_task::get_future()返回一个future对象myFuture用于获取task的任务结果
    std::thread myThread(std::move(myPackaged),"hello world");//创建一个线程执行task任务，这里注意move语义强制将左值转为右值使用因为packaged_task禁止copy constructor，可以不创建线程，那么task任务的执行将和future结果的获取在同一个线程，这样就不叫异步了
    //这里主线程可以做其它的操作
    int x=myFuture.get();//线程还可以在执行一些其它操作，直到其想获取task的结果时调用此语句
    std::cout << "myFuture result :" << x << std::endl;
    myThread.join();
    return 0;
}
