//
//  main.cpp
//  ProtectShareData
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//1  mutex粒度
//使用mutex的时候要尽量缩小临界区，若可能的话，对mutex加锁仅仅是为了获取共享数据，而对数据的计算放在临界区之外。
//
//2.4 std::call_once()用于多线程只执行一次
//[cpp] view plain copy
//template <class Fn, class... Args>
//void call_once (once_flag& flag, Fn&& fn, Args&&... args);
//若其它线程没有执行由flag标记的call_once，则本线程将调用fn执行创建工作。如果已经有线程(称为活动执行体)在执行flag标记的call_once，那么此后的其它线程调用此flag标记的call_once将会成为被动执行体，被动执行体不会调用fn函数，反而等待活动执行体从fn返回，从而保证fn只会被调用一次。如果活动执行体在call_once中抛出异常那么将会从被动执行体中选择一个称为新的活动执行体。值得注意的是一旦活动执行call_once成功返回，当前的被动执行体和以后的call_once都不会产生活动执行体即不会调用fn。
//若fn是一个对象的成员函数，那么第一个第一个args必须是这个成员函数所属的类。例如:call_once(flag,&X::fn,this)//X是个class
//std::once_flag同std::mutex一样是不能copy和move的，所有当把它们定义在一个类里面的时候要注意copy constructor和move constructor的设计。

//2.7  在某些读多写少的情形下可以使用读写锁，boost::shared_mutex，但是C++标准库没有提供读写锁机制，这也说明了：不要盲目相信读写锁能提升性能，copy on write可能更适合些。读写锁不是灵丹妙药，其性能依赖于读者和写者的数量，并且读写锁本身就增加了复杂性，所以是否获得性能提升还有待具体情形

//2.8  递归锁 std::recursive_mutex,可以在同一个线程中多次加锁，同样的也要相应的多次解锁后其它线程才能获得该锁。可以采用lock_guard<recursive_mutex> / unique_lock<recursive_mutex>管理递归锁。
//某些时候一个class需要多个线程访问，为了保证线程安全，class的每个成员函数都需要mutex加锁保护，这样若将来某个线程调用其中一个成员函数，该成员函数又去调用另外一个成员函数，这是recursive_mutex就可以实现多次加锁功能了。
//但无论如何，使用递归锁都不是个好办法，这说明设计上存在某些问题，第二个成员函数加锁mutex时第一个成员函数可能已经打破了某些invariants。可以重新创建一个函数内部调用这两个成员函数，总之使用递归锁时要反思自己的设计本身。


#include <iostream>
#include <memory>

struct some_resource {
    void do_something()
    {}
};

std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;
void init_resource()
{
    resource_ptr.reset(new some_resource);
}
void foo()
{
    std::call_once(resource_flag,init_resource);//不管多少个线程调用，只会有一个线程执行init_resource，注意call_once参数要求是函数指针
    resource_ptr->do_something();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
