//
//  SortChunk.h
//  Concurrency
//
//  Created by Haitao on 22/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef SortChunk_h
#define SortChunk_h


//
//  Safe_Stack.h
//  mutex
//
//  Created by Haitao on 2/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//



/*
 重要的是：对一个很大的数据集进行排序时，当每层递归都产生一个新线程，最后就会产生
 大量的线程。你会看到其对性能的影响，如果有太多的线程存在，那么你的应用将会运行的
 很慢。如果数据集过于庞大，会将线程耗尽
 */

/*
 另一种选择是使用 std::thread::hardware_concurrency() 函数来确定线程的数量，就像在清单
 2.8中的并行版accumulate()一样。然后，你可以将已排序的数据推到线程安全的栈上(如第
 6、7章中提及的栈)。当线程无所事事，不是已经完成对自己数据块的梳理，就是在等待一组
 排序数据的产生；线程可以从栈上获取这组数据，并且对其排序。
 */

#include <list>
#include <future>
#include <vector>
#include <thread>
#include <memory>
#include "Safe_Stack.h"

//清单8.1 使用栈的并行快速排序算法——等待数据块排序
template<typename T>
class sorter // 1
{
    struct chunk_to_sort
    {
        std::list<T> data;
        std::promise<std::list<T> > promise;
    };
    thread_safe_stack<chunk_to_sort> chunks; // 2
    std::vector<std::thread> threads; // 3
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;
    
    sorter():
    max_thread_count(std::thread::hardware_concurrency()-1),
    end_of_data(false)
    {
    }
    
    ~sorter() // 4
    {
        end_of_data=true; // 5
        for(unsigned i=0;i<threads.size();++i)
        {
            threads[i].join(); // 6
        }
    }
    
    void try_sort_chunk()
    {
        std::shared_ptr<chunk_to_sort > chunk=chunks.pop(); // 7
        if(chunk)
        {
            sort_chunk(chunk); // 8
        }
    }
    
    std::list<T> do_sort(std::list<T>& chunk_data) // 9
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }
        std::list<T> result;
        result.splice(result.begin(),chunk_data,chunk_data.begin());
        T const& partition_val=*result.begin();
        typename std::list<T>::iterator divide_point= // 10
        std::partition(chunk_data.begin(),chunk_data.end(),
                       [&](T const& val){return val<partition_val;});
        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(),
                                    chunk_data,chunk_data.begin(),
                                    divide_point);
        std::future<std::list<T> > new_lower=new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk)); // 11
        if(threads.size()<max_thread_count) // 12
        {
            threads.push_back(std::thread(&sorter<T>::sort_thread,this));
        }
        std::list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(),new_higher);
        while(new_lower.wait_for(std::chrono::seconds(0)) !=
              std::future_status::ready) // 13
        {
            try_sort_chunk(); // 14
        }
        result.splice(result.begin(),new_lower.get());
        return result;
    }
    void sort_chunk(std::shared_ptr<chunk_to_sort> const& chunk)
    {
        chunk->promise.set_value(do_sort(chunk->data)); // 15
    }
    
    void sort_thread()
    {
        while(!end_of_data) // 16
        {
            try_sort_chunk(); // 17
            std::this_thread::yield(); // 18
        }
    }
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) // 19
{
    if(input.empty())
    {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input); // 20
}

#endif /* SortChunk_h */
