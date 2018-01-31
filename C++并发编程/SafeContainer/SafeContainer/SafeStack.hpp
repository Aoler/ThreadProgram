//
//  SafeStack.h
//  SafeContainer
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef SafeStack_h
#define SafeStack_h

#include <memory>
#include <exception>
#include <mutex>
#include <stack>

struct empty_stack: std::exception
{
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data=other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()//top和pop合并，采用shared_ptr返回栈顶元素防止元素构造时发生异常
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));//make_shared比shared_ptr直接构造效率高
        data.pop();
        return res;
    }
    void pop(T& value)//采用参数引用返回栈顶元素
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value=std::move(data.top());
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
//这里还是有死锁的可能：栈内元素是用户代码，若该元素在构造或析构时修改栈则可能发生死锁，当然这种设计本身就有一定问题，应该从设计本身下手
#endif /* SafeStack_h */
