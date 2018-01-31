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
