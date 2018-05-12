//
//  main.cpp
//  future
//
//  Created by Haitao on 1/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//1）有一int型全局变量g_Flag初始值为0；
//2） 在主线称中起动线程1，打印“this is thread1”，并将g_Flag设置为1
//3） 在主线称中启动线程2，打印“this is thread2”，并将g_Flag设置为2
//4） 线程序1需要在线程2退出后才能退出
//5） 主线程在检测到g_Flag从1变为2，或者从2变为1的时候退出


#include<iostream>
#include<functional>
#include<thread>
#include<future>
#include<utility>
#include<cstdio>
#include<chrono>
#include<atomic>
#include <unistd.h>

/*
 因为大量的线程会消耗大量的系统资源，还有可能造成上下文频繁切换(当线程数量超出硬件可接受的并发数
 时)，这都会对性能有影响。
 
*/

using namespace std;
atomic<int> flag(0);//采用原子操作保护g_Flag的读写
void worker1(future<int> fut){//线程1
    printf("this is thread1\n");
    flag=1;
    int value = fut.get();//线程1阻塞至线程2设置共享状态
    std::cout << "value:" << value <<std::endl;
    printf("thread1 exit\n");
}
void worker2(promise<int> prom){//线程2
    printf("this is thread2\n");//C++11的线程输出cout没有boost的好，还是会出现乱序，所以采用printf，有点不爽
    flag=2;
    prom.set_value(20);//线程2设置了共享状态后，线程1才会被唤醒
    printf("thread2 exit\n");
}
int main(){
    promise<int> prom;
    future<int> fut=prom.get_future();
    thread one(worker1,move(fut));//注意future和promise不允许拷贝，但是具备move语义
    thread two(worker2,move(prom));
    while(flag.load()==0);
    one.detach();
    two.detach();
    
    sleep(2);
    printf("main thread exit\n");
    return 0;
}
