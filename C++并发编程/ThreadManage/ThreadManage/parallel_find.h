//
//  parallel_find.h
//  ThreadManage
//
//  Created by Haitao on 14/5/18.
//  Copyright © 2018年 mess. All rights reserved.
//

#ifndef parallel_find_h
#define parallel_find_h

#include "ThreadJoiner.h"

/*
 一种办法，中断其他线程的一个办法就是使用一个原子变量作为一个标识，在处理过每一个
 元素后就对这个标识进行检查。如果标识被设置，那么就有线程找到了匹配元素，所以算法
 就可以停止并返回了。用这种方式来中断线程，就可以将那些没有处理的数据保持原样，并
 且在更多的情况下，相较于串行方式，性能能提升很多。缺点就是，加载原子变量是一个很
 慢的操作，会阻碍每个线程的运行。
 如何返回值和传播异常呢？现在你有两个选择。你可以使用一个future数组，使
 用 std::packaged_task 来转移值和异常，在主线程上对返回值和异常进行处理；或者使
 用 std::promise 对工作线程上的最终结果直接进行设置。这完全依赖于你想怎么样处理工作
 线程上的异常。如果想停止第一个异常(即使还没有对所有元素进行处理)，就可以使
 用 std::promise 对异常和最终值进行设置。另外，如果想要让其他工作线程继续查找，可以
 使用 std::packaged_task 来存储所有的异常，当线程没有找到匹配元素时，异常将再次抛
 出。
 这种情况下，我会选择 std::promise ，因为其行为和 std::find 更为接近。这里需要注意一
 下搜索的元素是不是在提供的搜索范围内。因此，在所有线程结束前，获取future上的结果。
 如果被future阻塞住，所要查找的值不在范围内，就会持续的等待下去。实现代码如下。
 
*/


template<typename Iterator,typename MatchType>
Iterator parallel_find(Iterator first,Iterator last,MatchType match)
{
    struct find_element // 1
    {
        void operator()(Iterator begin,Iterator end,
                        MatchType match,
                        std::promise<Iterator>* result,
                        std::atomic<bool>* done_flag)
        {
            try
            {
                for(;(begin!=end) && !done_flag->load();++begin) // 2
                {
                    if(*begin==match)
                    {
                        result->set_value(begin); // 3
                        done_flag->store(true); // 4
                        return;
                    }
                }
            }
            catch(...) // 5
            {
                try
                {
                    result->set_exception(std::current_exception()); // 6
                    done_flag->store(true);
                }
                catch(...) // 7
                {}
            }
        }
    };
    
    unsigned long const length=std::distance(first,last);
    if(!length)
        return last;
    unsigned long const min_per_thread=25;
    unsigned long const max_threads=
    (length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads=
    std::thread::hardware_concurrency();
    unsigned long const num_threads=
    std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long const block_size=length/num_threads;
    std::promise<Iterator> result; // 8
    std::atomic<bool> done_flag(false); // 9
    std::vector<std::thread> threads(num_threads-1);
    { // 10
        join_threads joiner(threads);
        Iterator block_start=first;
        for(unsigned long i=0;i<(num_threads-1);++i)
        {
            Iterator block_end=block_start;
            std::advance(block_end,block_size);
            threads[i]=std::thread(find_element(), // 11
                                   block_start,block_end,match,
                                   &result,&done_flag);
            block_start=block_end;
        }
        find_element()(block_start,last,match,&result,&done_flag); // 12
    }
    if(!done_flag.load()) //13
    {
        return last;
    }
    return result.get_future().get(); // 14
}


#endif /* parallel_find_h */
