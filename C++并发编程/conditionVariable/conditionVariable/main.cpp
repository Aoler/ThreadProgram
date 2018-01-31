//
//  main.cpp
//  conditionVariable
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//1  线程睡眠函数
//[cpp] view plain copy
//std::this_thread::sleep_for(std::chrono::milliseconds(100));//头文件#include<chrono>,供选择的如seconds()等
//不要使用睡眠函数同步线程，睡眠函数可以用于复现线程的一些行为。
//线程重新调度函数：
//[cpp] view plain copy
//std::this_thread::yield()
//重新调度本线程，用于线程等待其它线程时而不阻塞本线程


//condition_variable();
//condition_variable (const condition_variable&) = delete;//没有copy constructor和move constructor
//
//~condition_variable();//阻塞在此条件变量上的线程将被唤醒，不会有线程再对此条件变量wait
//void wait (unique_lock<mutex>& lck);//等待条件发生，注意参数是unique_lock，可能发生虚假唤醒，即不是notify唤醒的
//
//template <class Predicate>
//void wait (unique_lock<mutex>& lck, Predicate pred);//pred是个函数对象返回bool，线程会在pred返回false的下阻塞，pred返回true会被唤醒，这样可以防止虚假唤醒等价于:while (!pred()) wait(lck);
//template <class Rep, class Period>
//
//cv_status wait_for (unique_lock<mutex>& lck,const chrono::duration<Rep,Period>& rel_time);//在指定rel_time时间段内等待，此期间可能被notify唤醒
//template <class Rep, class Period, class Predicate>
//bool wait_for (unique_lock<mutex>& lck,const chrono::duration<Rep,Period>& rel_time, Predicate pred);//为了防止虚假唤醒，加了一个pred，在rel_time内被notify且要pred返回true方可被唤醒
//
//template <class Clock, class Duration>
//cv_status wait_until (unique_lock<mutex>& lck,const chrono::time_point<Clock,Duration>& abs_time);//等待到一个指定的绝对时间点，语义和wait_for差不多
//template <class Clock, class Duration, class Predicate>
//bool wait_until (unique_lock<mutex>& lck,const chrono::time_point<Clock,Duration>& abs_time,Predicate pred);
//
//void notify_one() noexcept;//若有多个线程等待此条件变量则选择一个线程唤醒，若没有线程等待则此函数什么也不做
//void notify_all() noexcept;//唤醒所有阻塞在此条件变量上的线程，若没有阻塞线程此函数什么也不做

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
struct data_chunk{

};

bool more_data_to_prepare ()
{
    return true;
}

std::mutex mut;//该mutex会被condition_variable使用
std::queue<data_chunk> data_queue;//用于在生产者线程和消费者线程间传递数据
std::condition_variable data_cond;
void data_preparation_thread()//生产者
{
    while(more_data_to_prepare())
    {
        data_chunk const data=prepare_data();
        std::lock_guard<std::mutex> lk(mut);//这里可以用lock_guard或者unique_lock
        data_queue.push(data);//
        data_cond.notify_one();//唤醒一个阻塞在条件变量的线程，多个阻塞线程会选择一个唤醒，若没有阻塞的线程则什么也不做
    }
}
void data_processing_thread()//消费者
{
    while(true)//不断消费
    {
        std::unique_lock<std::mutex> lk(mut);//注意condition_varibale必须使用unique_lock，因为unique_lock提供了lock和unlock操作更加灵活
        data_cond.wait(lk,[]{return !data_queue.empty();});//##1##lambda表达式用于检测queue是否为空，这里可以是任意的callable object
        data_chunk data=data_queue.front();
        data_queue.pop();
        lk.unlock();//wait返回时mutex处于locked，为了提高并发应该立即显示解锁
        process(data);//处理数据
        if(is_last_chunk(data))
            break;  
    }  
}

//wait首先检查lambda表示是否为真（queue是否为空），若为假(queue中有数据)wait直接返回;若为假则表明条件不满足，
//那么wait会将mutex解锁，并使线程进入阻塞状态，这里看出为什么要用unique_lock了吧因为其提供了比lock_guard更灵活的lock和unlock操作。
//当阻塞的线程被notify_one()/notify_all()唤醒时，首先对mutex上锁，并检查lambda表达式(queue是否为空的条件)。
//若条件满足（queue非空）则wait立即返回，mutex处于locked。若条件仍不满足(queue为空)，wait将对mutex解锁，并继续阻塞线程在waiting状态。

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
