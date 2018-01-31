//
//  SafeQueue.h
//  SafeContainer
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef SafeQueue_h
#define SafeQueue_h
#include <memory>
#include <exception>
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T> > data_queue;//队里存储的是shared_ptr这样可以保证push和pop操作时不会引起构造或析构异常，队列更加高效
    std::condition_variable data_cond;//采用条件变量同步入队和出队操作
public:
    threadsafe_queue(){}
    void wait_and_pop(T& value)//直至容器中有元素可以删除
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        value=std::move(*data_queue.front());
        data_queue.pop();
    }
    bool try_pop(T& value)//若队中无元素可以删除则直接返回false
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        std::shared_ptr<T> res=data_queue.front();
        data_queue.pop();
        return res;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res=data_queue.front();
        data_queue.pop();
        return res;
    }
    void push(T new_value)
    {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));//数据的构造在临界区外从而缩小临界区，并且不会在临界区抛出异常
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

#endif /* SafeQueue_h */
