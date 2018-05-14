//
//  parallel_foreach.h
//  ThreadManage
//
//  Created by Haitao on 14/5/18.
//  Copyright © 2018年 mess. All rights reserved.
//

#ifndef parallel_foreach_h
#define parallel_foreach_h
#include "ThreadJoiner.h"

template<typename Iterator,typename T>
struct accumulate_block
{
    T operator()(Iterator first,Iterator last) // 1
    {
        return std::accumulate(first,last,T()); // 2
    }
};

template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);
    if(!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long const block_size = length/num_threads;
    std::vector<std::future<T> > futures(num_threads-1); // 3
    std::vector<std::thread> threads(num_threads-1);
    join_threads joiner(threads);  //异常安全，保证线程退出
    
    
    Iterator block_start=first;
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        std::packaged_task<T(Iterator,Iterator)> task((accumulate_block<Iterator,T>()));// 4
        futures[i]=std::future<T>(task.get_future()); // 5
        threads[i]=std::thread(std::move(task),block_start,block_end); // 6
        block_start=block_end;
    }
    T last_result=accumulate_block<Iterator,T>()(block_start,last); // 7
    std::for_each(threads.begin(),threads.end(),
                  std::mem_fn(&std::thread::join));
    T result=init; // 8
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        result+=futures[i].get(); // 9
    }
    result += last_result; // 10
    return result;
}

#endif /* parallel_foreach_h */
