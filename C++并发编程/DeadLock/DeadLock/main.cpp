//
//  main.cpp
//  DeadLock
//
//  Created by Haitao on 2/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//



#include <mutex>
#include<unistd.h>
#include<thread>
#include<iostream>
using namespace std;

//1 死锁：每个线程都希望锁住一些列锁以执行某个操作，且每个线程持有一个不同的锁，最终每个线程都需要其它线程的锁，导致所有线程都不能向前执行。
//
//1.1  顺序加锁lock
//1.1 死锁的一个解决方式：每个线程对锁的请求顺序一致。C++库提供了顺序加锁机制，可以一次性锁住多个mutex而不会出现死锁：
class big_object
{
public:
    big_object(int i=0):data(i){}
public:
    int data;
};

void swap(big_object& lhs,big_object& rhs)
{
    sleep(1);
    cout<<"swap()"<<endl;
}
class X
{
private:
    big_object some_detail;
    mutable std::mutex m;
public:
    X(big_object const& sd):some_detail(sd){}
    
    friend void swap(X& lhs, X& rhs)
    {
        if(&lhs==&rhs)
            return;
        std::lock(lhs.m,rhs.m);//C++库会自动生成加锁顺序，即使调用顺序不一致
        std::lock_guard<std::mutex> lock_a(lhs.m,std::adopt_lock);//adopt_lock是告诉lock_guard对象mutex已经被上锁，而lock_gurad对象将获得mutex的所有权，这样就可以保证在lock可能出现异常导致没有unlock的情形不会出现，栈对象会在异常抛出后自动析构
        std::lock_guard<std::mutex> lock_b(rhs.m,std::adopt_lock);
        swap(lhs.some_detail,rhs.some_detail);
    }
};

void threadFun(X& one,X& two)
{
    swap(one,two);
}


//1.2 可以通过比较多个mutex的地址大小实现顺序加锁，下面的程序是先锁地址小的mutex，再锁地址大的mutex。

class test{
public:
    friend void orderLock(test& one,test& two){
        if(&one.m>&two.m){
            lock_guard<mutex> smallGuard(two.m);
            sleep(1);
            cout<<"the second argument's mutex smaller"<<endl;
            lock_guard<mutex> bigGuard(one.m);
        }
        else if(&one.m==&two.m)
            cout<<"lock the same mutex"<<endl;
        else{
            lock_guard<mutex> smallGuard(one.m);
            sleep(1);
            cout<<"the first argument's mutex smaller"<<endl;
            lock_guard<mutex> bigGuard(two.m);
        }
    }
private:
    mutex m;
};
//
//2 避免死锁
//死锁不仅仅发生在mutex上，如多个线程相互之间调用join也会死锁。避免死锁的方式：don’t wait for another thread if there’s a chance it’s waiting for you//不要等待那些有机会等待你的线程。
//2.1 避免使用递归锁，don’t acquire a lock if you already hold one。
//2.2 避免持有锁期间执行用户代码，不清楚用户代码将会做什么就有可能出现死锁。有时这种情形难以避免的，如std::stack是个泛型类，内部存储用户指定的数据类型，stack<vector<string> >之类的vector本身就具有很多操作，更别说stack存储用户自定义的类万一又想存取stack本身呢？这种情形可以强加一些限制条件以保证加锁安全。
//2.3 若果一定要使用两个及以上的加锁，一定要确保加锁顺序，std::lock是个不错的选择。例如一个双向链表若一个线程从头开始遍历，一个线程从尾开始遍历那么两个线程必然会在中间某处出现死锁，这时放弃双向遍历而规定一个加锁顺序。
//2.4 lock hierarchy: 通常对对个mutex加锁会导致死锁，解决这个问题的最好办法就是保证加锁的顺序，当lock总是以一个规定的顺序进行时不会出现死锁。lock hierarchy指的是给每个mutex分配一个标号从而对mutex逻辑排序。限制条件是：当线程已经持有编号比n小的锁时不能再请求标号为n的mutex.






int main()
{
    //顺序加锁
    big_object ten(10),hundred(100);
    X one(ten),two(hundred);
    thread threadOne(threadFun,ref(one),ref(two));//不同线程有不同的参数调用顺序
    thread threadTwo(threadFun,ref(two),ref(one));
    threadOne.join();
    threadTwo.join();
    //先锁地址小的mutex，再锁地址大的mutex。
    test oneTest,twoTest;
    cout<<&oneTest<<" "<<&twoTest<<endl;
    orderLock(oneTest,twoTest);//#1#
    orderLock(twoTest,oneTest);//#1#
    //thread threadOne(threadFun,ref(one),ref(two));//#2#   注意这里要用ref将test对象转换为引用传递到子线程中去，否则thread会拷贝test对象从而拷贝mutex，而mutex::mutex(const mutex&)=delete从而会报错
    //thread threadTwo(threadFun,ref(two),ref(one));//#2#
    //threadOne.join();
    //threadTwo.join();//这里要说明的是多线程间共享锁不是什么好事情
    
    
    return 0;
}
