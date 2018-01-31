//
//  FunctionWrap.h
//  ThreadPool
//
//  Created by Haitao on 25/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef FunctionWrap_h
#define FunctionWrap_h

/*
 
 清单9.2展示了对简单线程池的修改，通过修改就能等待任务完成，以
 及在工作线程完成后，返回一个结果到等待线程中去，不过 std::packaged_task<> 实例是不可
 拷贝的，仅是可移动的，所以不能再使用 std::function<> 来实现任务队列，因
 为 std::function<> 需要存储可复制构造的函数对象。包装一个自定义函数，用来处理只可移
 动的类型。这就是一个带有函数操作符的类型擦除类。只需要处理那些没有函数和无返回的
 函数，所以这是一个简单的虚函数调用。
*/

class function_wrapper
{
    struct impl_base {
        virtual void call()=0;
        virtual ~impl_base() {}
    };
    
    template<typename F>
    struct impl_type: impl_base
    {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call() { f(); }
    };
    
    std::unique_ptr<impl_base> impl;
public:
    template<typename F>
    function_wrapper(F&& f):
        impl(new impl_type<F>(std::move(f)))  //5
    {}
    
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other):
        impl(std::move(other.impl))
    {}
    
    void operator()() { impl->call(); }
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl=std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&)=delete;
    function_wrapper(function_wrapper&)=delete;
    function_wrapper& operator=(const function_wrapper&)=delete;
};

#endif /* FunctionWrap_h */
