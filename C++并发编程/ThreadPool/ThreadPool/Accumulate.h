//
//  Accumulate.h
//  ThreadPool
//
//  Created by Haitao on 25/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Accumulate_h
#define Accumulate_h

#include <numeric>
#include "ThreadPoolV1.h"

template<typename Iterator,typename T>
struct accumulate_block
{
//    T operator()(Iterator first,Iterator last) // 1
//    {
//        return std::accumulate(first,last,T()); // 2
//    }
    
    T operator()() // 1
    {
        //return std::accumulate(first,last,T()); // 2
        return T();
    }

};

template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);
    if(!length)
        return init;
    unsigned long const block_size=25;
    unsigned long const num_blocks=(length+block_size-1)/block_size; // 1
    std::vector<std::future<T> > futures(num_blocks-1);
    thread_pool pool;
    Iterator block_start=first;
    for(unsigned long i=0;i<(num_blocks-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        futures[i]=pool.submit(accumulate_block<Iterator,T>()); // 2
        block_start=block_end;
    }
    //T last_result=accumulate_block<Iterator,T>()(block_start,last);
    T last_result=accumulate_block<Iterator,T>()();
    T result=init;
    for(unsigned long i=0;i<(num_blocks-1);++i)
    {
        result+=futures[i].get();
    }
    result += last_result;
    return result;
}

#endif /* Accumulate_h */
