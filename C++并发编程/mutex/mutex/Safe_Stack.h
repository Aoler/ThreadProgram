//
//  Safe_Stack.h
//  mutex
//
//  Created by Haitao on 2/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Safe_Stack_h
#define Safe_Stack_h


#include <iostream>
#include <stack>

class empty_stack: std::exception
{
    const char* what() const throw()//本函数不抛出任何异常
    {
        return "empty stack";
    }
    
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;//声明为mutable可以在const成员函数中也可以加锁,否则报错
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
        data.push(new_value);
    }
    std::shared_ptr<T> pop()//top和pop合并，采用shared_ptr返回栈顶元素防止元素构造时发生异常
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top())); //make_shared比shared_ptr直接构造效率高   
        data.pop();
        return res;
    }
    void pop(T& value)//通过函数参数返回
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value=data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

#endif /* Safe_Stack_h */
