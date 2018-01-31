//
//  main.cpp
//  uniqueLock
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//1 回顾采用RAII手法管理mutex的std::lock_guard其功能是在对象构造时将mutex加锁，析构时对mutex解锁，
//这样一个栈对象保证了在异常情形下mutex可以在lock_guard对象析构被解锁，lock_guard拥有mutex的所有权。
//explicit lock_guard (mutex_type& m);//必须要传递一个mutex作为构造参数
//lock_guard (mutex_type& m, adopt_lock_t tag);//tag=adopt_lock表示mutex已经在之前被上锁，这里lock_guard将拥有mutex的所有权
//lock_guard (const lock_guard&) = delete;//不允许copy constructor

// 2 再来看一个与std::lock_guard功能相似但功能更加灵活的管理mutex的对象 std::unique_lock，
//unique_lock内部持有mutex的状态：locked,unlocked。unique_lock比lock_guard占用空间和速度慢一些，
//因为其要维护mutex的状态。
//1 unique_lock() noexcept;   //可以构造一个空的unique_lock对象，此时并不拥有任何mutex
//
//2 explicit unique_lock (mutex_type& m);//拥有mutex，并调用mutex.lock()对其上锁
//
//3 unique_lock (mutex_type& m, try_to_lock_t tag);//tag=try_lock表示调用mutex.try_lock()尝试加锁
//
//4 unique_lock (mutex_type& m, defer_lock_t tag) noexcept;//tag=defer_lock表示不对mutex加锁，只管理mutex，此时mutex应该是没有加锁的
//
//5 unique_lock (mutex_type& m, adopt_lock_t tag);//tag=adopt_lock表示mutex在此之前已经被上锁，此时unique_locl管理mutex
//
//6 template <class Rep, class Period>
//unique_lock (mutex_type& m, const chrono::duration<Rep,Period>& rel_time);//在一段时间rel_time内尝试对mutex加锁,mutex.try_lock_for(rel_time)
//
//7 template <class Clock, class Duration>
//unique_lock (mutex_type& m, const chrono::time_point<Clock,Duration>& abs_time);//mutex.try_lock_until(abs_time)直到abs_time尝试加锁
//
//8 unique_lock (const unique_lock&) = delete;//禁止拷贝构造
//
//9 unique_lock (unique_lock&& x);//获得x管理的mutex，此后x不再和mutex相关，x此后相当于一个默认构造的unique_lock,移动构造函数，具备移动语义,movable but not copyable

#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
using namespace std;
struct bank_account//银行账户
{
    explicit bank_account(string name, int money)
    {
        sName = name;
        iMoney = money;
    }
    
    string sName;
    int iMoney;
    mutex mMutex;//账户都有一个锁mutex
};
void transfer( bank_account &from, bank_account &to, int amount )//这里缺少一个from==to的条件判断个人觉得
{
//    加锁的时候为什么不是如下这样的？在前面一篇博文中有讲到多个语句加锁可能导致deadlock，假设:同一时刻A向B转账，B也向A转账，
//    那么先持有自己的锁再相互请求对方的锁必然deadlock。
//    [cpp] view plain copy
//    lock_guard<mutex> lock1( from.mMutex );
//    lock_guard<mutex> lock2( to.mMutex );
    if (&from == &to)
        return ;
    unique_lock<mutex> lock1( from.mMutex, defer_lock );//defer_lock表示延迟加锁，此处只管理mutex
    unique_lock<mutex> lock2( to.mMutex, defer_lock );
    lock( lock1, lock2 );//lock一次性锁住多个mutex防止deadlock
    from.iMoney -= amount;
    to.iMoney += amount;
    cout << "Transfer " << amount << " from "<< from.sName << " to " << to.sName << endl;
}
int main()
{
    bank_account Account1( "User1", 100 );
    bank_account Account2( "User2", 50 );
    thread t1( [&](){ transfer( Account1, Account2, 10 ); } );//lambda表达式
    thread t2( [&](){ transfer( Account2, Account1, 5 ); } );
    t1.join();  
    t2.join();  
}
