//
//  main.cpp
//  async
//
//  Created by Haitao on 1/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//async (Fn&& fn, Args&&... args);//自动选择线程执行任务fn，args是fn的参数，若fn是某个对象的非静态成员函数那么第一个args必须是对象的名字，后面的args是fn所需的参数
//
//async (launch policy, Fn&& fn, Args&&... args);//有三种方式policy执行任务fn
//policy=launch::async表示开启一个新的线程执行fn
//policy=launch::deferred 表示fn推迟到future::wait/get时才执行
//policy=launch::async|launch::deferred表示由库自动选择哪种机制执行fn，和第一种构造方式async(fn,args)策略相同

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <functional>
#include <thread>

template <typename RandomIt>
int parallel_sum(RandomIt beg, RandomIt end)
{
    //auto len = end - beg;
    int len = std::distance(beg, end);
    if (len < 1000)
        return std::accumulate(beg, end, 0);
    
    RandomIt mid = beg + len/2;
    auto handle = std::async(std::launch::async,
                             parallel_sum<RandomIt>, mid, end);
    int sum = parallel_sum(beg, mid);
    return sum + handle.get();
}


class X
{
public:
    void foo(int,std::string const& str)
    {
        std::cout << str << std::endl;
    }
    
    std::string bar(std::string const& str)
    {
        std::cout << str << std::endl;
        return std::string(str);
    }
};

struct Y
{
    double operator()(double num)
    {
        std::cout << num << std::endl;
        return num;
    }
};

class move_only
{
public:
    move_only()
    {
        std::cout << "default construct function ..." <<std::endl;
    }
    
    move_only(move_only&&)
    {
        std::cout << "move construct function ..." <<std::endl;
    }
    
    move_only(move_only const&) = delete;
    move_only& operator=(move_only&&);
    move_only& operator=(move_only const&) = delete;
    
    void operator()()
    {}
};


int main()
{
    std::vector<int> v(10000, 1);
    std::cout << "The sum is " << parallel_sum(v.begin(), v.end()) << '\n';
    
    X x;
    std::thread m_thread(&X::foo,&x,42,"hello");
    m_thread.join();
    auto f1=std::async(&X::foo,&x,42,"hello"); // 调用p->foo(42, "hello")，p是指向x的指针
    auto f2=std::async(&X::bar,x,"goodbye"); // 调用tmpx.bar("goodbye")， tmpx是x的拷贝副本
//    
    Y y;
    auto f3=std::async(Y(),3.141); // 调用tmpy(3.141)，tmpy通过Y的移动构造函数得到
    auto f4=std::async(std::ref(y),2.718); // 调用y(2.718)
//    X baz(X&);
//    std::async(baz,std::ref(x)); // 调用baz(x)
    auto f5=std::async(move_only()); // 调用tmp()，tmp是通过std::move(move_only())构造得到
    
    auto f6=std::async(std::launch::async,Y(),1.2); // 在新线程上执行
    auto f7=std::async(std::launch::deferred,Y(),3.141); // 在wait()或get()调用时执行
    auto f8=std::async(
                       std::launch::deferred | std::launch::async,
                       Y(),3.141); // 实现选择执行方式
    f7.wait(); // 调用延迟函数

}


